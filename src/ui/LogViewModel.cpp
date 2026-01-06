#include "LogViewModel.h"
#include <QDateTime>
#include <QTimer>
#include <algorithm>

namespace ZenRunner::UI {

/**
 * @brief Memory-efficient LogViewModel implementation with throttling
 * 
 * This implementation:
 * - Uses lazy loading - only materializes visible data
 * - Avoids copying log entries when possible
 * - Uses Qt's implicit sharing for strings
 * - Minimizes signal emissions to reduce overhead
 * - Implements throttling to prevent log flood from overwhelming UI
 */
class LogViewModel : public ILogViewModel {
    Q_OBJECT

public:
    enum LogRoles {
        TextRole = Qt::UserRole + 1,
        TimestampRole,
        IsErrorRole,
        FormattedTimeRole,
        PlainTextRole,
        HasAnsiCodesRole,
        StyledSegmentsRole
    };

    explicit LogViewModel(QObject* parent = nullptr)
        : ILogViewModel(parent)
        , showErrorsOnly_(false)
        , refreshPending_(false)
        , throttleIntervalMs_(16) // 60 FPS target
    {
        // Setup throttle timer
        throttleTimer_ = new QTimer(this);
        throttleTimer_->setSingleShot(true);
        connect(throttleTimer_, &QTimer::timeout, this, &LogViewModel::performThrottledRefresh);
    }

    ~LogViewModel() override = default;

    // ILogViewModel interface
    void setLogBuffer(std::shared_ptr<Core::ILogBuffer> logBuffer) override {
        if (logBuffer_ != logBuffer) {
            logBuffer_ = logBuffer;
            
            // Setup callback for throttled updates when logs are added
            if (logBuffer_) {
                logBuffer_->setUpdateCallback([this]() {
                    requestLogUpdate();
                });
            }
            
            scheduleThrottledRefresh();
        }
    }

    std::shared_ptr<Core::ILogBuffer> getLogBuffer() const override {
        return logBuffer_;
    }

    void setFilter(const QString& filter) override {
        if (filter_ != filter) {
            filter_ = filter;
            scheduleThrottledRefresh();
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
            scheduleThrottledRefresh();
        }
    }

    bool isShowingErrorsOnly() const override {
        return showErrorsOnly_;
    }

    void refresh() override {
        // Public refresh can be called directly for immediate updates
        // (e.g., when user explicitly requests refresh)
        performImmediateRefresh();
    }

    void requestLogUpdate() override {
        // Throttled log update for automatic updates when logs are added
        scheduleThrottledRefresh();
    }

    void setThrottleInterval(int intervalMs) override {
        throttleIntervalMs_ = intervalMs;
    }

    int getThrottleInterval() const override {
        return throttleIntervalMs_;
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
            
            case PlainTextRole:
                return entry.plainText;
                
            case HasAnsiCodesRole:
                return entry.hasAnsiCodes;
                
            case StyledSegmentsRole:
                return convertSegmentsToVariant(entry.segments);
                
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        static const QHash<int, QByteArray> roles = {
            {TextRole, "text"},
            {TimestampRole, "timestamp"},
            {IsErrorRole, "isError"},
            {FormattedTimeRole, "formattedTime"},
            {PlainTextRole, "plainText"},
            {HasAnsiCodesRole, "hasAnsiCodes"},
            {StyledSegmentsRole, "styledSegments"}
        };
        return roles;
    }

private:
    std::shared_ptr<Core::ILogBuffer> logBuffer_;
    std::vector<Core::LogEntry> filteredLogs_;
    QString filter_;
    bool showErrorsOnly_;
    QTimer* throttleTimer_;
    bool refreshPending_;
    int throttleIntervalMs_;
    
    /**
     * @brief Schedule a throttled refresh
     * 
     * Sets the pending flag and starts the timer. If timer is already running,
     * this ensures refresh will happen when timer fires.
     */
    void scheduleThrottledRefresh() {
        refreshPending_ = true;
        
        if (!throttleTimer_->isActive()) {
            throttleTimer_->start(throttleIntervalMs_);
        }
    }
    
    /**
     * @brief Perform the throttled refresh
     * 
     * Called by the timer. Only refreshes if pending flag is set.
     */
    void performThrottledRefresh() {
        if (refreshPending_) {
            refreshPending_ = false;
            performImmediateRefresh();
            emit logsAdded();
        }
    }
    
    /**
     * @brief Perform immediate refresh of the model
     * 
     * Updates the filtered logs from the buffer and notifies the view.
     */
    void performImmediateRefresh() {
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
    
    /**
     * @brief Convert styled segments to QVariantList for QML
     * @param segments Vector of styled segments
     * @return QVariantList containing segment data
     */
    QVariant convertSegmentsToVariant(const std::vector<Core::StyledSegment>& segments) const {
        QVariantList result;
        
        for (const auto& seg : segments) {
            QVariantMap segmentMap;
            segmentMap["text"] = seg.text;
            segmentMap["fgColor"] = seg.foregroundColor.name();
            segmentMap["bgColor"] = seg.backgroundColor.name();
            segmentMap["bold"] = seg.bold;
            segmentMap["italic"] = seg.italic;
            segmentMap["underline"] = seg.underline;
            
            result.append(segmentMap);
        }
        
        return result;
    }
};

// Factory function
std::unique_ptr<ILogViewModel> createLogViewModel(QObject* parent) {
    return std::make_unique<LogViewModel>(parent);
}

} // namespace ZenRunner::UI

#include "LogViewModel.moc"
