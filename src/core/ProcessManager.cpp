#include "core/ProcessManager.h"
#include <QDateTime>
#include <algorithm>

#ifdef Q_OS_UNIX
#include <signal.h>
#include <sys/types.h>
#endif

namespace ZenRunner {

// ============================================================================
// AsyncProcess Implementation
// ============================================================================

AsyncProcess::AsyncProcess(ProcessConfig config, QObject* parent)
    : QObject(parent)
    , config_(std::move(config))
    , process_(std::make_unique<QProcess>(this))
    , terminationTimer_(std::make_unique<QTimer>(this))
{
    // Configure the QProcess
    process_->setProgram(config_.command);
    process_->setArguments(config_.arguments);
    
    if (!config_.workingDirectory.isEmpty()) [[likely]] {
        process_->setWorkingDirectory(config_.workingDirectory);
    }
    
    if (!config_.environment.isEmpty()) {
        process_->setProcessEnvironment(config_.environment);
    }
    
    // Set channel mode
    if (config_.captureOutput) [[likely]] {
        if (config_.mergeChannels) {
            process_->setProcessChannelMode(QProcess::MergedChannels);
        } else {
            process_->setProcessChannelMode(QProcess::SeparateChannels);
        }
    } else [[unlikely]] {
        process_->setProcessChannelMode(QProcess::ForwardedChannels);
    }
    
    // Connect signals for asynchronous operation
    // Use Qt::DirectConnection for same-thread critical paths to minimize latency
    connect(process_.get(), &QProcess::readyReadStandardOutput,
            this, &AsyncProcess::onReadyReadStandardOutput, Qt::DirectConnection);
    connect(process_.get(), &QProcess::readyReadStandardError,
            this, &AsyncProcess::onReadyReadStandardError, Qt::DirectConnection);
    connect(process_.get(), &QProcess::started,
            this, &AsyncProcess::onStarted, Qt::DirectConnection);
    connect(process_.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AsyncProcess::onFinished, Qt::DirectConnection);
    connect(process_.get(), &QProcess::errorOccurred,
            this, &AsyncProcess::onErrorOccurred, Qt::DirectConnection);
    connect(process_.get(), &QProcess::stateChanged,
            this, &AsyncProcess::onStateChanged, Qt::DirectConnection);
    
    // Configure termination timer (single shot)
    terminationTimer_->setSingleShot(true);
    connect(terminationTimer_.get(), &QTimer::timeout,
            this, &AsyncProcess::onTerminationTimeout, Qt::DirectConnection);
}

AsyncProcess::~AsyncProcess() {
    // Non-blocking termination - send termination signal but don't wait
    if (process_ && process_->state() != QProcess::NotRunning) [[unlikely]] {
        // Disconnect signals to prevent callbacks during destruction
        process_->disconnect();
        
        // Send termination signal - the process will be killed by OS cleanup
        process_->terminate();
        
        // Note: We intentionally don't wait here to avoid blocking.
        // The OS will clean up the process. If immediate cleanup is needed,
        // call stop() explicitly before destroying the object.
    }
}

Result<bool> AsyncProcess::start() {
    if (state_ != ProcessState::NotStarted && state_ != ProcessState::Stopped 
        && state_ != ProcessState::Finished && state_ != ProcessState::Crashed) [[unlikely]] {
        return Result<bool>::Err(QString("Process is already running or starting"));
    }
    
    setState(ProcessState::Starting);
    
    // Start the process asynchronously
    process_->start();
    
    // The onStarted() slot will be called when the process actually starts
    return Result<bool>::Ok(true);
}

void AsyncProcess::stop(int timeoutMs) {
    if (state_ != ProcessState::Running && state_ != ProcessState::Paused) [[unlikely]] {
        return;
    }
    
    setState(ProcessState::Stopping);
    terminationRequested_ = true;
    
    // Try graceful termination first
    process_->terminate();
    
    // Start timer for forceful kill if needed
    terminationTimer_->start(timeoutMs);
}

void AsyncProcess::pause() {
    #ifdef Q_OS_UNIX
    if (state_ == ProcessState::Running) [[likely]] {
        const qint64 pid = process_->processId();
        if (pid > 0) {
            ::kill(static_cast<pid_t>(pid), SIGSTOP);
            setState(ProcessState::Paused);
        }
    }
    #endif
}

void AsyncProcess::resume() {
    #ifdef Q_OS_UNIX
    if (state_ == ProcessState::Paused) [[likely]] {
        const qint64 pid = process_->processId();
        if (pid > 0) {
            ::kill(static_cast<pid_t>(pid), SIGCONT);
            setState(ProcessState::Running);
        }
    }
    #endif
}

void AsyncProcess::writeInput(const QString& data) {
    if (state_ == ProcessState::Running) [[likely]] {
        process_->write(data.toUtf8());
    }
}

void AsyncProcess::forceKillImmediate() {
    // Non-blocking immediate kill - no waiting
    if (process_ && process_->state() != QProcess::NotRunning) {
        process_->kill();
    }
}

qint64 AsyncProcess::processId() const {
    return process_ ? process_->processId() : -1;
}

std::vector<LogEntry> AsyncProcess::getRecentLogs(std::size_t count) const {
    return logBuffer_.lastN(count);
}

std::vector<LogEntry> AsyncProcess::getAllLogs() const {
    return logBuffer_.toVector();
}

void AsyncProcess::clearLogs() {
    logBuffer_.clear();
}

void AsyncProcess::onReadyReadStandardOutput() {
    if (!config_.captureOutput) [[unlikely]] {
        return;
    }
    
    // Use move semantics to avoid copies
    QByteArray data = process_->readAllStandardOutput();
    if (data.isEmpty()) [[unlikely]] {
        return;
    }
    
    QString output = QString::fromUtf8(data);
    
    // Emit signal before processing to minimize latency for UI updates
    emit outputReceived(output, false);
    
    // Split by newlines and add each line as a log entry
    // Reserve approximate capacity to avoid reallocations
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (!line.trimmed().isEmpty()) [[likely]] {
            addLogEntry(line, LogLevel::Info, false);
        }
    }
}

void AsyncProcess::onReadyReadStandardError() {
    if (!config_.captureOutput) [[unlikely]] {
        return;
    }
    
    // Use move semantics to avoid copies
    QByteArray data = process_->readAllStandardError();
    if (data.isEmpty()) [[unlikely]] {
        return;
    }
    
    QString output = QString::fromUtf8(data);
    
    // Emit signal before processing to minimize latency for UI updates
    emit outputReceived(output, true);
    
    // Split by newlines and add each line as a log entry
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (!line.trimmed().isEmpty()) [[likely]] {
            addLogEntry(line, LogLevel::Error, true);
        }
    }
}

void AsyncProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    terminationTimer_->stop();
    exitCode_ = exitCode;
    
    if (exitStatus == QProcess::CrashExit) [[unlikely]] {
        setState(ProcessState::Crashed);
        addLogEntry(QString("Process crashed with exit code %1").arg(exitCode),
                   LogLevel::Critical, true);
    } else [[likely]] {
        setState(ProcessState::Finished);
        if (exitCode != 0) [[unlikely]] {
            addLogEntry(QString("Process finished with exit code %1").arg(exitCode),
                       LogLevel::Warning, false);
        }
    }
    
    emit finished(exitCode, exitStatus);
    terminationRequested_ = false;
}

void AsyncProcess::onErrorOccurred(QProcess::ProcessError error) {
    const QString errorMsg = processErrorToString(error);
    
    if (error == QProcess::FailedToStart) [[unlikely]] {
        setState(ProcessState::Crashed);
    }
    
    addLogEntry(QString("Process error: %1").arg(errorMsg), 
               LogLevel::Critical, true);
    emit errorOccurred(errorMsg);
}

void AsyncProcess::onStarted() {
    setState(ProcessState::Running);
    addLogEntry(QString("Process started (PID: %1)").arg(process_->processId()),
               LogLevel::Info, false);
}

void AsyncProcess::onStateChanged(QProcess::ProcessState /*newState*/) {
    // QProcess state changes are already handled by our signals
    // This is here for additional monitoring if needed
}

void AsyncProcess::onTerminationTimeout() {
    // Graceful termination timeout - force kill
    if (process_ && process_->state() != QProcess::NotRunning) [[unlikely]] {
        addLogEntry("Process did not terminate gracefully, forcing kill",
                   LogLevel::Warning, true);
        process_->kill();
    }
}

void AsyncProcess::setState(ProcessState newState) {
    if (state_ != newState) [[likely]] {
        state_ = newState;
        emit stateChanged(newState);
    }
}

void AsyncProcess::addLogEntry(const QString& text, LogLevel level, bool isStderr) {
    LogEntry entry;
    entry.text = text;
    entry.level = level;
    entry.isStderr = isStderr;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    logBuffer_.emplace(std::move(entry));
    emit logAdded(entry);
}

QString AsyncProcess::processErrorToString(QProcess::ProcessError error) const {
    switch (error) {
        case QProcess::FailedToStart:
            return "Failed to start: The process failed to start. Either the invoked program is missing, or you may have insufficient permissions.";
        case QProcess::Crashed:
            return "Crashed: The process crashed some time after starting successfully.";
        case QProcess::Timedout:
            return "Timeout: The operation timed out.";
        case QProcess::WriteError:
            return "Write Error: An error occurred when attempting to write to the process.";
        case QProcess::ReadError:
            return "Read Error: An error occurred when attempting to read from the process.";
        case QProcess::UnknownError:
        default:
            return "Unknown Error: An unknown error occurred.";
    }
}

// ============================================================================
// ProcessManager Implementation
// ============================================================================

ProcessManager::ProcessManager(QObject* parent)
    : QObject(parent)
{
}

ProcessManager::~ProcessManager() {
    // Non-blocking cleanup: force kill all running processes immediately
    std::lock_guard lock(processesMutex_);
    
    for (auto& [id, process] : processes_) {
        if (process->state() == ProcessState::Running ||
            process->state() == ProcessState::Paused) {
            // Direct kill without waiting - non-blocking
            process->forceKillImmediate();
        }
    }
    // processes_ map destruction will clean up AsyncProcess objects
}

Result<AsyncProcess*> ProcessManager::createProcess(
    const QString& id,
    const ProcessConfig& config)
{
    std::lock_guard lock(processesMutex_);
    
    if (processes_.contains(id)) [[unlikely]] {
        return Result<AsyncProcess*>::Err(
            QString("Process with ID '%1' already exists").arg(id)
        );
    }
    
    auto process = std::make_unique<AsyncProcess>(config, this);
    auto* processPtr = process.get();
    
    // Connect signals before storing
    connectProcessSignals(id, processPtr);
    
    processes_[id] = std::move(process);
    
    return Result<AsyncProcess*>::Ok(processPtr);
}

AsyncProcess* ProcessManager::getProcess(const QString& id) const {
    std::lock_guard lock(processesMutex_);
    
    auto it = processes_.find(id);
    if (it != processes_.end()) [[likely]] {
        return it->second.get();
    }
    
    return nullptr;
}

void ProcessManager::removeProcess(const QString& id) {
    std::lock_guard lock(processesMutex_);
    
    auto it = processes_.find(id);
    if (it != processes_.end()) [[likely]] {
        // Stop the process gracefully before removing
        it->second->stop(3000);
        processes_.erase(it);
    }
}

Result<bool> ProcessManager::startProcess(const QString& id) {
    auto* process = getProcess(id);
    
    if (!process) [[unlikely]] {
        return Result<bool>::Err(
            QString("Process with ID '%1' not found").arg(id)
        );
    }
    
    return process->start();
}

void ProcessManager::stopProcess(const QString& id, int timeoutMs) {
    auto* process = getProcess(id);
    
    if (process) [[likely]] {
        process->stop(timeoutMs);
    }
}

bool ProcessManager::runScript(const QString& id, const QString& command, const QString& workingDir) {
    // Remove existing process with same ID if it exists
    if (getProcess(id)) {
        removeProcess(id);
    }
    
    // Create process config
    ProcessConfig config;
    config.command = command;
    config.workingDirectory = workingDir;
    config.captureOutput = true;
    
    // Create the process
    auto result = createProcess(id, config);
    if (result.isErr()) {
        qWarning() << "Failed to create process:" << result.error();
        return false;
    }
    
    // Start the process
    auto startResult = startProcess(id);
    if (startResult.isErr()) {
        qWarning() << "Failed to start process:" << startResult.error();
        removeProcess(id);
        return false;
    }
    
    return true;
}

void ProcessManager::startAll(bool sequential) {
    std::lock_guard lock(processesMutex_);
    
    if (sequential) {
        // Start processes one after another
        // Note: For true sequential startup, we'd need to wait for each
        // process to start before starting the next. This is a simplified version.
        for (auto& [id, process] : processes_) {
            (void)process->start();  // Explicitly ignore return value
        }
    } else {
        // Start all processes simultaneously
        for (auto& [id, process] : processes_) {
            (void)process->start();  // Explicitly ignore return value
        }
    }
}

void ProcessManager::stopAll(int timeoutMs) {
    std::lock_guard lock(processesMutex_);
    
    for (auto& [id, process] : processes_) {
        if (process->state() == ProcessState::Running ||
            process->state() == ProcessState::Paused) [[likely]] {
            process->stop(timeoutMs);
        }
    }
}

QStringList ProcessManager::processIds() const {
    std::lock_guard lock(processesMutex_);
    
    QStringList ids;
    ids.reserve(processes_.size());
    for (const auto& [id, _] : processes_) {
        ids.append(id);
    }
    return ids;
}

int ProcessManager::runningCount() const {
    std::lock_guard lock(processesMutex_);
    
    return std::count_if(processes_.begin(), processes_.end(),
        [](const auto& pair) {
            return pair.second->state() == ProcessState::Running;
        }
    );
}

bool ProcessManager::hasRunningProcesses() const {
    return runningCount() > 0;
}

void ProcessManager::connectProcessSignals(const QString& id, AsyncProcess* process) {
    // Use Qt::DirectConnection for same-thread signal forwarding to minimize latency
    // All AsyncProcess instances are created in ProcessManager's thread
    connect(process, &AsyncProcess::stateChanged,
            this, [this, id](ProcessState newState) {
                emit processStateChanged(id, newState);
            }, Qt::DirectConnection);
    
    connect(process, &AsyncProcess::outputReceived,
            this, [this, id](const QString& output, bool isStderr) {
                emit processOutput(id, output, isStderr);
            }, Qt::DirectConnection);
    
    connect(process, &AsyncProcess::finished,
            this, [this, id](int exitCode, QProcess::ExitStatus exitStatus) {
                emit processFinished(id, exitCode);
                // Emit crash-specific signal for immediate notification
                if (exitStatus == QProcess::CrashExit) {
                    emit processCrashed(id, exitCode);
                }
            }, Qt::DirectConnection);
    
    connect(process, &AsyncProcess::errorOccurred,
            this, [this, id](const QString& error) {
                emit processError(id, error);
            }, Qt::DirectConnection);
}

} // namespace ZenRunner
