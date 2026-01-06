#include "ui/StatisticsViewModel.h"
#include <QDebug>

namespace ZenRunner::UI {

StatisticsViewModel::StatisticsViewModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int StatisticsViewModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return orderedProcessIds_.size();
}

QVariant StatisticsViewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= orderedProcessIds_.size()) {
        return QVariant();
    }
    
    const QString& processId = orderedProcessIds_[index.row()];
    auto it = processStats_.find(processId);
    if (it == processStats_.end()) {
        return QVariant();
    }
    
    const auto& stats = it.value();
    
    switch (role) {
    case ProcessIdRole:
        return stats.processId;
    case ProjectNameRole:
        return stats.projectName;
    case CpuPercentRole:
        return stats.currentStats.cpuPercent;
    case MemoryMBRole:
        return stats.currentStats.memoryMB();
    case AvgCpuRole:
        return stats.summary ? stats.summary->avgCpu : 0.0;
    case MaxCpuRole:
        return stats.summary ? stats.summary->maxCpu : 0.0;
    case AvgMemoryRole:
        return stats.summary ? stats.summary->avgMemoryMB : 0.0;
    case MaxMemoryRole:
        return stats.summary ? stats.summary->maxMemoryMB : 0.0;
    case HasSpikeRole:
        return stats.summary ? stats.summary->hasRecentSpike : false;
    case HistoryRole: {
        if (!stats.summary) {
            return QVariantList();
        }
        QVariantList history;
        for (const auto& point : stats.summary->recentHistory) {
            QVariantMap dataPoint;
            dataPoint["timestamp"] = QDateTime::fromStdTimePoint(point.timestamp).toMSecsSinceEpoch();
            dataPoint["cpu"] = point.cpuPercent;
            dataPoint["memory"] = point.memoryMB;
            history.append(dataPoint);
        }
        return history;
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> StatisticsViewModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[ProcessIdRole] = "processId";
    roles[ProjectNameRole] = "projectName";
    roles[CpuPercentRole] = "cpuPercent";
    roles[MemoryMBRole] = "memoryMB";
    roles[AvgCpuRole] = "avgCpu";
    roles[MaxCpuRole] = "maxCpu";
    roles[AvgMemoryRole] = "avgMemory";
    roles[MaxMemoryRole] = "maxMemory";
    roles[HasSpikeRole] = "hasSpike";
    roles[HistoryRole] = "history";
    return roles;
}

double StatisticsViewModel::totalCpuPercent() const {
    double total = 0.0;
    for (const auto& stats : processStats_) {
        total += stats.currentStats.cpuPercent;
    }
    return total;
}

double StatisticsViewModel::totalMemoryMB() const {
    double total = 0.0;
    for (const auto& stats : processStats_) {
        total += stats.currentStats.memoryMB();
    }
    return total;
}

void StatisticsViewModel::setResourceMonitor(std::shared_ptr<ZenRunner::Core::IProcessResourceMonitor> monitor) {
    if (resourceMonitor_) {
        disconnect(resourceMonitor_.get(), nullptr, this, nullptr);
    }
    
    resourceMonitor_ = monitor;
    
    if (resourceMonitor_) {
        connect(resourceMonitor_.get(), &ZenRunner::Core::IProcessResourceMonitor::statsUpdated,
                this, &StatisticsViewModel::onStatsUpdated);
        connect(resourceMonitor_.get(), &ZenRunner::Core::IProcessResourceMonitor::resourceSpikeDetected,
                this, &StatisticsViewModel::onSpikeDetected);
    }
}

QVariantMap StatisticsViewModel::getProcessStats(const QString& processId) const {
    QVariantMap result;
    
    auto it = processStats_.find(processId);
    if (it == processStats_.end()) {
        return result;
    }
    
    const auto& stats = it.value();
    result["processId"] = stats.processId;
    result["projectName"] = stats.projectName;
    result["cpuPercent"] = stats.currentStats.cpuPercent;
    result["memoryMB"] = stats.currentStats.memoryMB();
    
    if (stats.summary) {
        result["avgCpu"] = stats.summary->avgCpu;
        result["maxCpu"] = stats.summary->maxCpu;
        result["avgMemory"] = stats.summary->avgMemoryMB;
        result["maxMemory"] = stats.summary->maxMemoryMB;
        result["hasSpike"] = stats.summary->hasRecentSpike;
    }
    
    return result;
}

void StatisticsViewModel::onStatsUpdated(const QString& processId, ZenRunner::Core::ProcessResourceStats stats) {
    updateProcessStats(processId);
    emit statisticsChanged();
}

void StatisticsViewModel::onSpikeDetected(const QString& processId, double cpuPercent, double memoryMB) {
    qDebug() << "Spike detected for" << processId << "- CPU:" << cpuPercent << "% Memory:" << memoryMB << "MB";
    emit spikeDetected(processId, cpuPercent, memoryMB);
}

void StatisticsViewModel::updateProcessStats(const QString& processId) {
    if (!resourceMonitor_) {
        return;
    }
    
    auto currentStats = resourceMonitor_->getCurrentStats(processId);
    auto summary = resourceMonitor_->getSummary(processId);
    
    if (!currentStats) {
        return;
    }
    
    // Check if this is a new process
    const bool isNew = !processStats_.contains(processId);
    
    ProcessDisplayStats displayStats;
    displayStats.processId = processId;
    displayStats.currentStats = *currentStats;
    displayStats.summary = summary;
    
    // Extract project name from processId (format: "projectName:scriptName")
    const int colonPos = processId.indexOf(':');
    if (colonPos > 0) {
        displayStats.projectName = processId.left(colonPos);
    } else {
        displayStats.projectName = processId;
    }
    
    if (isNew) {
        // Add new process
        const int newRow = orderedProcessIds_.size();
        beginInsertRows(QModelIndex(), newRow, newRow);
        orderedProcessIds_.append(processId);
        processStats_[processId] = displayStats;
        endInsertRows();
        emit processCountChanged();
    } else {
        // Update existing process
        processStats_[processId] = displayStats;
        const int row = orderedProcessIds_.indexOf(processId);
        if (row >= 0) {
            const QModelIndex idx = index(row);
            emit dataChanged(idx, idx);
        }
    }
}

} // namespace ZenRunner::UI
