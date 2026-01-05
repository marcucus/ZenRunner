#pragma once

#include "core/ILogBuffer.hpp"
#include <QAbstractListModel>
#include <QString>
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief Interface for log view model
 * 
 * This interface defines the contract for exposing log data to the UI layer.
 * It bridges the Core layer's log buffer to QML via QAbstractListModel.
 */
class ILogViewModel : public QAbstractListModel {
    Q_OBJECT

public:
    virtual ~ILogViewModel() = default;

    /**
     * @brief Set the log buffer data source
     * @param logBuffer Shared pointer to log buffer
     */
    virtual void setLogBuffer(std::shared_ptr<Core::ILogBuffer> logBuffer) = 0;

    /**
     * @brief Get the current log buffer
     * @return Shared pointer to log buffer
     */
    virtual std::shared_ptr<Core::ILogBuffer> getLogBuffer() const = 0;

    /**
     * @brief Set the filter text for searching logs
     * @param filter Filter string (case-insensitive)
     */
    virtual void setFilter(const QString& filter) = 0;

    /**
     * @brief Get the current filter text
     * @return Filter string
     */
    virtual QString getFilter() const = 0;

    /**
     * @brief Clear the filter
     */
    virtual void clearFilter() = 0;

    /**
     * @brief Set whether to show only errors
     * @param errorsOnly true to show only errors
     */
    virtual void setShowErrorsOnly(bool errorsOnly) = 0;

    /**
     * @brief Check if showing only errors
     * @return true if filtering to errors only
     */
    virtual bool isShowingErrorsOnly() const = 0;

    /**
     * @brief Refresh the model from the log buffer
     */
    virtual void refresh() = 0;

    /**
     * @brief Scroll to the bottom (most recent log)
     */
    Q_INVOKABLE virtual void scrollToBottom() = 0;

    /**
     * @brief Clear all logs
     */
    Q_INVOKABLE virtual void clearLogs() = 0;

signals:
    /**
     * @brief Emitted when new log entries are added
     */
    void logsAdded();

    /**
     * @brief Emitted when filter changes
     */
    void filterChanged();
};

} // namespace ZenRunner::UI
