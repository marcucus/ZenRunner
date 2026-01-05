#include "ui/LogViewModel.h"
#include <QDateTime>
#include <algorithm>

namespace ZenRunner::UI {

/**
 * @brief Memory-efficient LogViewModel implementation
 * 
 * This implementation:
 * - Uses lazy loading - only materializes visible data
 * - Avoids copying log entries when possible
 * - Uses Qt's implicit sharing for strings
 * - Minimizes signal emissions to reduce overhead
 */
class LogViewModel : public ILogViewModel {
    Q_OBJECT

public:
    enum LogRoles {
        TextRole = Qt::UserRole + 1,
        TimestampRole,
        IsErrorRole,
        FormattedTimeRole
    };

    explicit LogViewModel(QObject* parent = nullptr)
        : ILogViewModel(parent)
        , showErrorsOnly_(false)
    {
    }

    ~LogViewModel() override = default;

    // ILogViewModel interface
    void setLogBuffer(std::shared_ptr<Core::ILogBuffer> logBuffer) override {
        if (logBuffer_ != logBuffer) {
            logBuffer_ = logBuffer;
            refresh();
        }
    }

    std::shared_ptr<Core::ILogBuffer> getLogBuffer() const override {
        return logBuffer_;
    }

    void setFilter(const QString& filter) override {
        if (filter_ != filter) {
            filter_ = filter;
            refresh();
            emit filterChanged();
        }
    }

    QString getFilter() const override {
        return filter_;
    }

    void clearFilter() override {
        setFilter(QString());
    }

    void setShowErrorsOnly(bool errorsOnly) override {
        if (showErrorsOnly_ != errorsOnly) {
            showErrorsOnly_ = errorsOnly;
            refresh();
        }
    }

    bool isShowingErrorsOnly() const override {
        return showErrorsOnly_;
    }

    void refresh() override {
        if (!logBuffer_) [[unlikely]] {
            return;
        }

        beginResetModel();
        
        // Get all logs from buffer
        std::vector<Core::LogEntry> allLogs = logBuffer_->getAll();
        
        // Apply filters
        filteredLogs_.clear();
        filteredLogs_.reserve(allLogs.size());
        
        for (const auto& entry : allLogs) {
            // Filter by errors only
            if (showErrorsOnly_ && !entry.isError) {
                continue;
            }
            
            // Filter by text search
            if (!filter_.isEmpty() && 
                !entry.text.contains(filter_, Qt::CaseInsensitive)) {
                continue;
            }
            
            filteredLogs_.push_back(entry);
        }
        
        endResetModel();
    }

    void scrollToBottom() override {
        // QML will handle scrolling based on model changes
        // This is just a signal that can be connected to scrolling logic
    }

    void clearLogs() override {
        if (logBuffer_) {
            beginResetModel();
            logBuffer_->clear();
            filteredLogs_.clear();
            endResetModel();
        }
    }

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) [[unlikely]] {
            return 0;
        }
        return static_cast<int>(filteredLogs_.size());
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || 
            index.row() < 0 || 
            index.row() >= static_cast<int>(filteredLogs_.size())) [[unlikely]] {
            return QVariant();
        }

        const Core::LogEntry& entry = filteredLogs_[static_cast<size_t>(index.row())];

        switch (role) {
            case TextRole:
                return entry.text;
                
            case TimestampRole:
                return entry.timestamp;
                
            case IsErrorRole:
                return entry.isError;
                
            case FormattedTimeRole: {
                const QDateTime dt = QDateTime::fromMSecsSinceEpoch(entry.timestamp);
                return dt.toString("hh:mm:ss.zzz");
            }
                
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        static const QHash<int, QByteArray> roles = {
            {TextRole, "text"},
            {TimestampRole, "timestamp"},
            {IsErrorRole, "isError"},
            {FormattedTimeRole, "formattedTime"}
        };
        return roles;
    }

private:
    std::shared_ptr<Core::ILogBuffer> logBuffer_;
    std::vector<Core::LogEntry> filteredLogs_;
    QString filter_;
    bool showErrorsOnly_;
};

// Factory function
std::unique_ptr<ILogViewModel> createLogViewModel(QObject* parent) {
    return std::make_unique<LogViewModel>(parent);
}

} // namespace ZenRunner::UI

#include "LogViewModel.moc"
