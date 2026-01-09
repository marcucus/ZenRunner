import QtQuick
import QtQuick.Layouts
import "./components"

// Panneau de statistiques compact
GlassCard {
    id: root
    
    // Utiliser une propriété locale pour éviter les binding loops
    readonly property var stats: (typeof statisticsViewModel !== 'undefined' ? statisticsViewModel : null)
    
    glassOpacity: 0.1
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: "📊 Application Statistics"
                font.pixelSize: 16
                font.weight: Font.Bold
                color: "#ffffff"
            }
            
            Item { Layout.fillWidth: true }
            
            // Badge nombre de processus actifs
            Rectangle {
                width: 60
                height: 24
                radius: 12
                color: Qt.rgba(0.3, 0.8, 0.4, 0.2)
                visible: typeof stats !== 'undefined' && stats !== null
                
                Text {
                    anchors.centerIn: parent
                    text: (typeof stats !== 'undefined' && stats ? stats.processCount : 0) + " running"
                    font.pixelSize: 10
                    font.weight: Font.Medium
                    color: "#4ade80"
                }
            }
        }
        
        // Grille de métriques
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 4
            rowSpacing: 12
            columnSpacing: 16
            
            // Total CPU
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                
                Text {
                    text: "Total CPU"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                RowLayout {
                    spacing: 8
                    
                    Text {
                        text: stats ? (stats.totalCpuPercent.toFixed(1) + "%") : "0%"
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: {
                            if (!stats) return "#ffffff"
                            const total = stats.totalCpuPercent
                            if (total > 80) return "#ef4444"
                            if (total > 50) return "#fbbf24"
                            return "#4ade80"
                        }
                    }
                    
                    Text {
                        text: "💻"
                        font.pixelSize: 16
                        opacity: 0.6
                    }
                }
                
                // Barre de progression CPU
                Rectangle {
                    Layout.fillWidth: true
                    height: 6
                    radius: 3
                    color: Qt.rgba(1, 1, 1, 0.1)
                    
                    Rectangle {
                        width: parent.width * (stats ? Math.min(stats.totalCpuPercent / 100, 1) : 0)
                        height: parent.height
                        radius: parent.radius
                        color: {
                            if (!stats) return "#4ade80"
                            const total = stats.totalCpuPercent
                            if (total > 80) return "#ef4444"
                            if (total > 50) return "#fbbf24"
                            return "#4ade80"
                        }
                        
                        Behavior on width {
                            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                        }
                    }
                }
            }
            
            // Total Memory
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                
                Text {
                    text: "Total Memory"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                RowLayout {
                    spacing: 8
                    
                    Text {
                        text: stats ? (stats.totalMemoryMB.toFixed(0) + " MB") : "0 MB"
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: {
                            if (!stats) return "#ffffff"
                            const total = stats.totalMemoryMB
                            if (total > 500) return "#ef4444"
                            if (total > 200) return "#fbbf24"
                            return "#4ade80"
                        }
                    }
                    
                    Text {
                        text: "🧠"
                        font.pixelSize: 16
                        opacity: 0.6
                    }
                }
                
                // Barre de progression Memory
                Rectangle {
                    Layout.fillWidth: true
                    height: 6
                    radius: 3
                    color: Qt.rgba(1, 1, 1, 0.1)
                    
                    Rectangle {
                        width: parent.width * (stats ? Math.min(stats.totalMemoryMB / 1000, 1) : 0)
                        height: parent.height
                        radius: parent.radius
                        color: {
                            if (!stats) return "#4ade80"
                            const total = stats.totalMemoryMB
                            if (total > 500) return "#ef4444"
                            if (total > 200) return "#fbbf24"
                            return "#4ade80"
                        }
                        
                        Behavior on width {
                            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                        }
                    }
                }
            }
            
            // Active Processes
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                
                Text {
                    text: "Active Processes"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                RowLayout {
                    spacing: 8
                    
                    Text {
                        text: stats ? stats.processCount.toString() : "0"
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: "#4a90e2"
                    }
                    
                    Text {
                        text: "⚡"
                        font.pixelSize: 16
                        opacity: 0.6
                    }
                }
                
                Text {
                    text: "processes running"
                    font.pixelSize: 9
                    color: "#666666"
                }
            }
            
            // Uptime / Status
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                
                Text {
                    text: "Application Status"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                RowLayout {
                    spacing: 8
                    
                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: "#4ade80"
                        
                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 1000 }
                            NumberAnimation { to: 1.0; duration: 1000 }
                        }
                    }
                    
                    Text {
                        text: "Running"
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: "#4ade80"
                    }
                }
                
                Text {
                    text: "All systems operational"
                    font.pixelSize: 9
                    color: "#666666"
                }
            }
        }
        
        // Message si aucun processus
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: stats === null || stats.processCount === 0
            
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 8
                
                Text {
                    text: "💤"
                    font.pixelSize: 32
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    opacity: 0.3
                }
                
                Text {
                    text: "No active processes"
                    font.pixelSize: 13
                    color: "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
