#include "core/ProcessManager.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <algorithm>

#ifdef Q_OS_UNIX
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
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
    
#ifdef Q_OS_UNIX
    // Create a new process group on Unix systems using setChildProcessModifier
    // This allows us to kill all child processes when stopping
    // setpgid(0, 0) creates a new process group where the calling process becomes the group leader
    process_->setChildProcessModifier([](){ 
        ::setpgid(0, 0);
    });
#endif
    
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
        
#ifdef Q_OS_UNIX
        // Kill the entire process group (don't log in destructor to avoid issues)
        const qint64 pid = process_->processId();
        if (pid > 0) {
            ::killpg(static_cast<pid_t>(pid), SIGTERM);
        }
#else
        // Send termination signal - the process will be killed by OS cleanup
        process_->terminate();
#endif
        
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
    
    // Wait briefly to detect immediate startup failures (e.g., command not found)
    // This allows us to return an error synchronously for obvious failures
    if (!process_->waitForStarted(100)) {
        const QProcess::ProcessError error = process_->error();
        if (error != QProcess::UnknownError && error != QProcess::Timedout) {
            // Process failed to start - get error message
            const QString errorMsg = processErrorToString(error);
            setState(ProcessState::Crashed);
            addLogEntry(QString("Failed to start: %1").arg(errorMsg), 
                       LogLevel::Critical, true);
            return Result<bool>::Err(errorMsg);
        }
    }
    
    // The onStarted() slot will be called when the process actually starts
    return Result<bool>::Ok(true);
}

void AsyncProcess::stop(int timeoutMs) {
    if (state_ != ProcessState::Running && state_ != ProcessState::Paused) [[unlikely]] {
        return;
    }
    
    setState(ProcessState::Stopping);
    terminationRequested_ = true;
    
#ifdef Q_OS_UNIX
    // Kill the entire process group to ensure all child processes are terminated
    killProcessGroup(SIGTERM);
#else
    // On Windows, use the default Qt termination
    process_->terminate();
#endif
    
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
#ifdef Q_OS_UNIX
        // Kill the entire process group immediately with SIGKILL
        killProcessGroup(SIGKILL);
#else
        process_->kill();
#endif
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

void AsyncProcess::processChunkedOutput(bool isStderr) {
    // Configuration constants for chunk processing
    constexpr qint64 MAX_CHUNK_SIZE = 65536; // 64KB chunks
    constexpr int CHUNKS_BEFORE_YIELD = 4; // Process 4 chunks (256KB) before yielding
    constexpr int PROCESS_EVENTS_TIMEOUT_MS = 5; // Short timeout to reduce context switching
    
    int chunksProcessed = 0;
    
    while (process_->bytesAvailable() > 0) {
        // Use move semantics to avoid copies
        QByteArray data = process_->read(MAX_CHUNK_SIZE);
        if (data.isEmpty()) [[unlikely]] {
            break;
        }
        
        QString output = QString::fromUtf8(data);
        
        // Emit signal before processing to minimize latency for UI updates
        emit outputReceived(output, isStderr);
        
        // Split by newlines and add each line as a log entry
        // Reserve approximate capacity to avoid reallocations
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        const LogLevel level = isStderr ? LogLevel::Error : LogLevel::Info;
        
        for (const QString& line : lines) {
            if (!line.trimmed().isEmpty()) [[likely]] {
                addLogEntry(line, level, isStderr);
            }
        }
        
        chunksProcessed++;
        
        // Allow event loop to process other events to keep UI responsive
        // Only yield after processing multiple chunks to reduce context switching
        // Note: ExcludeUserInputEvents is intentional - during heavy output processing,
        // we prioritize log rendering over user input to prevent event queue buildup.
        // User input is still processed between readyRead signals (typically < 100ms gaps).
        if (process_->bytesAvailable() > 0 && chunksProcessed >= CHUNKS_BEFORE_YIELD) {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, PROCESS_EVENTS_TIMEOUT_MS);
            chunksProcessed = 0;
        }
    }
}

void AsyncProcess::onReadyReadStandardOutput() {
    if (!config_.captureOutput) [[unlikely]] {
        return;
    }
    
    processChunkedOutput(false);
}

void AsyncProcess::onReadyReadStandardError() {
    if (!config_.captureOutput) [[unlikely]] {
        return;
    }
    
    processChunkedOutput(true);
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
#ifdef Q_OS_UNIX
        // Kill the entire process group with SIGKILL
        killProcessGroup(SIGKILL);
#else
        process_->kill();
#endif
    }
}

void AsyncProcess::setState(ProcessState newState) {
    if (state_ != newState) [[likely]] {
        state_ = newState;
        emit stateChanged(newState);
    }
}

#ifdef Q_OS_UNIX
bool AsyncProcess::killProcessGroup(int signal) {
    const qint64 pid = process_->processId();
    if (pid <= 0) {
        return false;
    }
    
    // Attempt to kill the process group
    int result = ::killpg(static_cast<pid_t>(pid), signal);
    if (result == 0) {
        return true;  // Success
    }
    
    // If killpg failed, log the error and fall back to Qt's methods
    if (signal == SIGTERM) {
        addLogEntry(QString("Failed to send SIGTERM to process group (errno: %1), falling back to terminate()").arg(errno),
                   LogLevel::Warning, true);
        process_->terminate();
    } else if (signal == SIGKILL) {
        addLogEntry(QString("Failed to send SIGKILL to process group (errno: %1), falling back to kill()").arg(errno),
                   LogLevel::Warning, true);
        process_->kill();
    }
    
    return false;
}
#endif

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

bool ProcessManager::runScript(const QString& id, const QString& scriptName, const QString& workingDir) {
    // Remove existing process with same ID if it exists
    if (getProcess(id)) {
        removeProcess(id);
    }
    
    // Detect package manager from lock files
    QString packageManager = "npm"; // default
    QDir projectDir(workingDir);
    if (projectDir.exists("pnpm-lock.yaml")) {
        packageManager = "pnpm";
    } else if (projectDir.exists("yarn.lock")) {
        packageManager = "yarn";
    }
    
    qDebug() << "Running script:" << scriptName << "in" << workingDir << "with" << packageManager;
    
    // Build the command to run the npm script
    QString command;
    QStringList args;
    
    if (packageManager == "yarn") {
        command = "yarn";
        args = QStringList{scriptName};
    } else if (packageManager == "pnpm") {
        command = "pnpm";
        args = QStringList{"run", scriptName};
    } else {  // npm
        command = "npm";
        args = QStringList{"run", scriptName};
    }
    
    // Create process config with proper environment
    ProcessConfig config;
    config.command = command;
    config.arguments = args;
    config.workingDirectory = workingDir;
    config.captureOutput = true;
    
    // Set up environment with PATH
    // This ensures npm/yarn/pnpm can find node executables
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    // Add local node_modules/.bin to PATH
    QString binPath = projectDir.absoluteFilePath("node_modules/.bin");
    QString currentPath = env.value("PATH");
    env.insert("PATH", binPath + ":" + currentPath);
    
    config.environment = env;
    
    qDebug() << "Executing:" << command << args.join(" ") << "in" << workingDir;
    
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
