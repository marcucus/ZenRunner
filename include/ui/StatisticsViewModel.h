#pragma once

#include "core/IProcessResourceMonitor.hpp"
#include <QObject>
#include <QAbstractListModel>
#include <QVariantMap>
#include <memory>

namespace ZenRunner::UI {

/**
 * @brief View model for statistics display in QML
 * 
 * Aggregates resource statistics per project and exposes them
 * to the QML interface in a format suitable for visualization.
 */
class StatisticsViewModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int processCount READ processCount NOTIFY processCountChanged)
    Q_PROPERTY(double totalCpuPercent READ totalCpuPercent NOTIFY statisticsChanged)
    Q_PROPERTY(double totalMemoryMB READ totalMemoryMB NOTIFY statisticsChanged)
    
public:
    enum StatisticsRoles {
        ProcessIdRole = Qt::UserRole + 1,
        ProjectNameRole,
        CpuPercentRole,
        MemoryMBRole,
        AvgCpuRole,
        MaxCpuRole,
        AvgMemoryRole,
        MaxMemoryRole,
        HasSpikeRole,
        HistoryRole
    };
    
    explicit StatisticsViewModel(QObject* parent = nullptr);
    ~StatisticsViewModel() override = default;
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // Properties
    int processCount() const { return processStats_.size(); }
    double totalCpuPercent() const;
    double totalMemoryMB() const;
    
    /**
     * @brief Set the resource monitor to track
     * @param monitor Resource monitor instance
     */
    void setResourceMonitor(std::shared_ptr<ZenRunner::Core::IProcessResourceMonitor> monitor);
    
    /**
     * @brief Get detailed statistics for a process
     * @param processId Process identifier
     * @return Variant map with detailed stats
     */
    Q_INVOKABLE QVariantMap getProcessStats(const QString& processId) const;
    
signals:
    void processCountChanged();
    void statisticsChanged();
    void spikeDetected(const QString& processId, double cpuPercent, double memoryMB);
    
private slots:
    void onStatsUpdated(const QString& processId, ZenRunner::Core::ProcessResourceStats stats);
    void onSpikeDetected(const QString& processId, double cpuPercent, double memoryMB);
    
private:
    struct ProcessDisplayStats {
        QString processId;
        QString projectName;  // Derived from processId
        ZenRunner::Core::ProcessResourceStats currentStats;
        std::shared_ptr<ZenRunner::Core::ResourceSummary> summary;
    };
    
    void updateProcessStats(const QString& processId);
    
    std::shared_ptr<ZenRunner::Core::IProcessResourceMonitor> resourceMonitor_;
    QMap<QString, ProcessDisplayStats> processStats_;
    QStringList orderedProcessIds_;  // For consistent model ordering
};

} // namespace ZenRunner::UI
