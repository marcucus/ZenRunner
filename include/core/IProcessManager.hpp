#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <functional>

namespace ZenRunner::Core {

/**
 * @brief Process state enumeration
 */
enum class ProcessState {
    NotStarted,
    Starting,
    Running,
    Stopping,
    Stopped,
    Crashed,
    Error
};

/**
 * @brief Interface for process management
 * 
 * This interface defines the contract for managing external processes.
 * All process lifecycle operations (start, stop, monitor) must go through this interface.
 */
class IProcessManager {
public:
    virtual ~IProcessManager() = default;

    /**
     * @brief Start a process with the given command and arguments
     * @param processId Unique identifier for the process
     * @param command Command to execute
     * @param arguments Command arguments
     * @param workingDirectory Working directory for the process
     * @return true if process started successfully
     */
    virtual bool startProcess(
        const QString& processId,
        const QString& command,
        const QStringList& arguments,
        const QString& workingDirectory
    ) = 0;

    /**
     * @brief Stop a running process
     * @param processId Process identifier
     * @param forceKill If true, force kill the process; otherwise attempt graceful termination
     * @return true if process stopped successfully
     */
    virtual bool stopProcess(const QString& processId, bool forceKill = false) = 0;

    /**
     * @brief Get the current state of a process
     * @param processId Process identifier
     * @return Current process state
     */
    virtual ProcessState getProcessState(const QString& processId) const = 0;

    /**
     * @brief Check if a process is running
     * @param processId Process identifier
     * @return true if process is running
     */
    virtual bool isProcessRunning(const QString& processId) const = 0;

    /**
     * @brief Get the exit code of a finished process
     * @param processId Process identifier
     * @return Exit code, or -1 if process is still running or doesn't exist
     */
    virtual int getExitCode(const QString& processId) const = 0;

    /**
     * @brief Register a callback for standard output
     * @param processId Process identifier
     * @param callback Function to call when output is available
     */
    virtual void onStandardOutput(
        const QString& processId,
        std::function<void(const QString&)> callback
    ) = 0;

    /**
     * @brief Register a callback for standard error
     * @param processId Process identifier
     * @param callback Function to call when error output is available
     */
    virtual void onStandardError(
        const QString& processId,
        std::function<void(const QString&)> callback
    ) = 0;

    /**
     * @brief Register a callback for process state changes
     * @param processId Process identifier
     * @param callback Function to call when state changes
     */
    virtual void onStateChanged(
        const QString& processId,
        std::function<void(ProcessState)> callback
    ) = 0;

    /**
     * @brief Stop all running processes
     * @param forceKill If true, force kill all processes
     */
    virtual void stopAllProcesses(bool forceKill = false) = 0;
};

} // namespace ZenRunner::Core
