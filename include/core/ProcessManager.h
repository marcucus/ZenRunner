#pragma once

#include "CircularBuffer.h"
#include "../types/CommonTypes.h"

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QProcessEnvironment>
#include <QTimer>
#include <memory>
#include <functional>
#include <concepts>
#include <unordered_map>

namespace ZenRunner {

/**
 * @brief Manages a single asynchronous process with complete lifecycle control
 * 
 * This class uses C++20 features and Qt's signal/slot mechanism to ensure
 * the main thread is never blocked during process execution. All I/O is
 * captured asynchronously via signals.
 */
class AsyncProcess : public QObject {
    Q_OBJECT

public:
    explicit AsyncProcess(ProcessConfig config, QObject* parent = nullptr);
    ~AsyncProcess() override;

    // Delete copy operations (QObject-derived)
    AsyncProcess(const AsyncProcess&) = delete;
    AsyncProcess& operator=(const AsyncProcess&) = delete;

    /**
     * @brief Start the process asynchronously
     * 
     * This method returns immediately. The process runs in the background
     * and emits signals for state changes and output.
     * 
     * @return Result indicating success or failure
     */
    [[nodiscard]] Result<bool> start();

    /**
     * @brief Stop the process gracefully
     * 
     * Sends SIGTERM and waits for termination. If the process doesn't
     * terminate within the timeout, sends SIGKILL.
     * 
     * @param timeoutMs Maximum time to wait for graceful termination
     */
    void stop(int timeoutMs = 5000);

    /**
     * @brief Pause the process (SIGSTOP)
     */
    void pause();

    /**
     * @brief Resume the process (SIGCONT)
     */
    void resume();

    /**
     * @brief Send input to the process stdin
     */
    void writeInput(const QString& data);

    /**
     * @brief Get the current process state
     */
    [[nodiscard]] ProcessState state() const noexcept { return state_; }

    /**
     * @brief Get the process ID (returns -1 if not running)
     */
    [[nodiscard]] qint64 processId() const;

    /**
     * @brief Get the exit code (valid only if state is Finished or Crashed)
     */
    [[nodiscard]] int exitCode() const noexcept { return exitCode_; }

    /**
     * @brief Get recent log entries
     */
    [[nodiscard]] std::vector<LogEntry> getRecentLogs(std::size_t count = 100) const;

    /**
     * @brief Get all log entries
     */
    [[nodiscard]] std::vector<LogEntry> getAllLogs() const;

    /**
     * @brief Clear all stored logs
     */
    void clearLogs();

    /**
     * @brief Get process configuration
     */
    [[nodiscard]] const ProcessConfig& config() const noexcept { return config_; }

    /**
     * @brief Force immediate kill (non-blocking, for emergency cleanup)
     * 
     * This method sends SIGKILL immediately without graceful termination.
     * Should only be used during emergency cleanup (e.g., destructor).
     */
    void forceKillImmediate();

signals:
    /**
     * @brief Emitted when process state changes
     */
    void stateChanged(ProcessState newState);

    /**
     * @brief Emitted when new output is available
     */
    void outputReceived(const QString& output, bool isStderr);

    /**
     * @brief Emitted when a new log entry is added
     */
    void logAdded(const LogEntry& entry);

    /**
     * @brief Emitted when the process finishes
     */
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief Emitted when the process encounters an error
     */
    void errorOccurred(const QString& error);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);
    void onStarted();
    void onStateChanged(QProcess::ProcessState newState);
    void onTerminationTimeout();

private:
    void setState(ProcessState newState);
    void addLogEntry(const QString& text, LogLevel level, bool isStderr);
    QString processErrorToString(QProcess::ProcessError error) const;
    
    /**
     * @brief Process output in chunks with smart batching
     * 
     * Reads process output in 64KB chunks and yields to the event loop
     * every 4 chunks to maintain UI responsiveness without excessive
     * context switching.
     * 
     * @param isStderr Whether to process stderr (true) or stdout (false)
     */
    void processChunkedOutput(bool isStderr);
    
#ifdef Q_OS_UNIX
    /**
     * @brief Kill the entire process group with the specified signal
     * 
     * Sends a signal to the process group. If the process group doesn't exist
     * or the call fails, falls back to Qt's process termination methods.
     * 
     * @param signal The signal to send (e.g., SIGTERM, SIGKILL)
     * @return true if the signal was sent successfully, false otherwise
     */
    bool killProcessGroup(int signal);
#endif

    ProcessConfig config_;
    std::unique_ptr<QProcess> process_;
    std::unique_ptr<QTimer> terminationTimer_;
    CircularBuffer<LogEntry, 5000> logBuffer_;
    ProcessState state_ = ProcessState::NotStarted;
    int exitCode_ = 0;
    bool terminationRequested_ = false;
};

/**
 * @brief Manages multiple asynchronous processes
 * 
 * This class coordinates multiple AsyncProcess instances and provides
 * batch operations for starting/stopping groups of processes.
 */
class ProcessManager : public QObject {
    Q_OBJECT

public:
    explicit ProcessManager(QObject* parent = nullptr);
    ~ProcessManager() override;

    /**
     * @brief Create and register a new process
     * 
     * @param id Unique identifier for the process
     * @param config Process configuration
     * @return Result with the process pointer if successful
     */
    [[nodiscard]] Result<AsyncProcess*> createProcess(
        const QString& id, 
        const ProcessConfig& config
    );

    /**
     * @brief Get a process by ID
     */
    [[nodiscard]] AsyncProcess* getProcess(const QString& id) const;

    /**
     * @brief Remove a process by ID
     * 
     * The process is stopped gracefully before removal.
     */
    void removeProcess(const QString& id);

    /**
     * @brief Start a process by ID
     */
    [[nodiscard]] Result<bool> startProcess(const QString& id);

    /**
     * @brief Stop a process by ID
     */
    Q_INVOKABLE void stopProcess(const QString& id, int timeoutMs = 5000);

    /**
     * @brief Start all registered processes
     * 
     * @param sequential If true, processes start one after another
     */
    void startAll(bool sequential = false);

    /**
     * @brief Stop all running processes
     */
    void stopAll(int timeoutMs = 5000);

    /**
     * @brief Get all process IDs
     */
    [[nodiscard]] QStringList processIds() const;

    /**
     * @brief Get count of running processes
     */
    [[nodiscard]] int runningCount() const;

    /**
     * @brief Check if any process is running
     */
    [[nodiscard]] bool hasRunningProcesses() const;
    
    /**
     * @brief Create and start a process from QML
     * @param id Process identifier
     * @param command Command to execute
     * @param workingDir Working directory
     * @return true if process was created and started
     */
    Q_INVOKABLE bool runScript(const QString& id, const QString& command, const QString& workingDir);
    
    /**
     * @brief Get information about all processes (for process monitor page)
     * @return List of QVariantMap containing process info (id, state, pid, command, workingDir)
     */
    Q_INVOKABLE QVariantList getAllProcessesInfo() const;
    
    /**
     * @brief Get information about a specific process
     * @param id Process identifier
     * @return QVariantMap with process info or empty map if not found
     */
    Q_INVOKABLE QVariantMap getProcessInfo(const QString& id) const;
    
    /**
     * @brief Force kill a process immediately (sends SIGKILL)
     * @param id Process identifier
     */
    Q_INVOKABLE void killProcess(const QString& id);
    
    /**
     * @brief Stop all running processes (invokable from QML)
     * @param timeoutMs Maximum time to wait for graceful termination
     */
    Q_INVOKABLE void stopAllProcesses(int timeoutMs = 5000);
    
    /**
     * @brief Get count of running processes (invokable from QML)
     */
    Q_INVOKABLE int getRunningCount() const;

signals:
    /**
     * @brief Emitted when a process state changes
     */
    void processStateChanged(const QString& id, ProcessState newState);

    /**
     * @brief Emitted when any process produces output
     */
    void processOutput(const QString& id, const QString& output, bool isStderr);

    /**
     * @brief Emitted when a process finishes
     */
    void processFinished(const QString& id, int exitCode);

    /**
     * @brief Emitted when a process encounters an error
     */
    void processError(const QString& id, const QString& error);

    /**
     * @brief Emitted when a process crashes
     * 
     * This signal is emitted specifically for crash events (CrashExit),
     * allowing immediate user notification through the notification system.
     * 
     * @param id Process identifier
     * @param exitCode Exit code of the crashed process
     */
    void processCrashed(const QString& id, int exitCode);

private:
    void connectProcessSignals(const QString& id, AsyncProcess* process);
    QString processStateToString(ProcessState state) const;

    std::unordered_map<QString, std::unique_ptr<AsyncProcess>> processes_;
    mutable std::mutex processesMutex_;
};

} // namespace ZenRunner
