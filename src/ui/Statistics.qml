import QtQuick
import QtQuick.Layouts
import "./components"

// Statistics dashboard panel showing CPU/RAM usage per project
Item {
    id: root
    
    property var statisticsViewModel: null
    
    // Spike notification - must be outside GlassCard
    Connections {
        target: statisticsViewModel
        
        function onSpikeDetected(processId, cpuPercent, memoryMB) {
            console.log("⚠️ Resource spike detected:", processId, 
                       "CPU:", cpuPercent.toFixed(1) + "%", 
                       "RAM:", memoryMB.toFixed(1) + "MB")
        }
    }

GlassCard {
    id: statisticsPanel
    anchors.fill: parent
    
    // Constants for display
    readonly property string labelText: "Avg"
    readonly property real maxMemoryScaleMB: 200.0  // Scale memory bars to 200MB max
    
    glassOpacity: 0.1
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: "📊 Resource Statistics"
                font.pixelSize: 16
                font.weight: Font.DemiBold
                color: "#ffffff"
            }
            
            Rectangle {
                width: 32
                height: 20
                radius: 10
                color: Qt.rgba(0.3, 0.6, 1.0, 0.2)
                visible: statisticsViewModel !== null
                
                Text {
                    anchors.centerIn: parent
                    text: statisticsViewModel ? statisticsViewModel.processCount.toString() : "0"
                    font.pixelSize: 11
                    font.weight: Font.Medium
                    color: "#4a90e2"
                }
            }
            
            Item { Layout.fillWidth: true }
        }
        
        // Total resource usage summary
        Row {
            Layout.fillWidth: true
            spacing: 16
            visible: statisticsViewModel !== null && statisticsViewModel.processCount > 0
            
            // Total CPU
            Column {
                spacing: 4
                
                Text {
                    text: "Total CPU"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                Text {
                    text: statisticsViewModel ? (statisticsViewModel.totalCpuPercent.toFixed(1) + "%") : "0%"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: {
                        if (!statisticsViewModel) return "#ffffff"
                        const total = statisticsViewModel.totalCpuPercent
                        if (total > 80) return "#ef4444"
                        if (total > 50) return "#fbbf24"
                        return "#4ade80"
                    }
                }
            }
            
            // Total Memory
            Column {
                spacing: 4
                
                Text {
                    text: "Total RAM"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                Text {
                    text: statisticsViewModel ? (statisticsViewModel.totalMemoryMB.toFixed(1) + " MB") : "0 MB"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: {
                        if (!statisticsViewModel) return "#ffffff"
                        const total = statisticsViewModel.totalMemoryMB
                        if (total > 100) return "#ef4444"
                        if (total > 50) return "#fbbf24"
                        return "#4ade80"
                    }
                }
            }
        }
        
        // Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(1, 1, 1, 0.1)
            visible: statisticsViewModel !== null && statisticsViewModel.processCount > 0
        }
        
        // Process list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            
            model: statisticsViewModel
            
            delegate: GlassCard {
                width: ListView.view.width
                glassOpacity: 0.06
                
                required property string processId
                required property string projectName
                required property real cpuPercent
                required property real memoryMB
                required property real avgCpu
                required property real maxCpu
                required property real avgMemory
                required property real maxMemory
                required property bool hasSpike
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    
                    // Process header
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        
                        Text {
                            text: projectName
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        
                        // Spike indicator
                        Rectangle {
                            width: 16
                            height: 16
                            radius: 8
                            color: "#ef4444"
                            visible: hasSpike
                            
                            SequentialAnimation on opacity {
                                running: hasSpike
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                            
                            Text {
                                anchors.centerIn: parent
                                text: "!"
                                font.pixelSize: 10
                                font.weight: Font.Bold
                                color: "#ffffff"
                            }
                        }
                    }
                    
                    // Current usage
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        
                        // CPU usage
                        Row {
                            spacing: 4
                            
                            Text {
                                text: "CPU:"
                                font.pixelSize: 11
                                color: "#888888"
                            }
                            
                            Text {
                                text: cpuPercent.toFixed(1) + "%"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: cpuPercent > 80 ? "#ef4444" : cpuPercent > 50 ? "#fbbf24" : "#4ade80"
                            }
                        }
                        
                        // Memory usage
                        Row {
                            spacing: 4
                            
                            Text {
                                text: "RAM:"
                                font.pixelSize: 11
                                color: "#888888"
                            }
                            
                            Text {
                                text: memoryMB.toFixed(1) + " MB"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: memoryMB > 100 ? "#ef4444" : memoryMB > 50 ? "#fbbf24" : "#4ade80"
                            }
                        }
                    }
                    
                    // Statistics bars
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        
                        // CPU stats
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            
                            Text {
                                text: statisticsPanel.labelText
                                font.pixelSize: 9
                                color: "#666666"
                                Layout.preferredWidth: 25
                            }
                            
                            // CPU average bar
                            Rectangle {
                                Layout.fillWidth: true
                                height: 4
                                radius: 2
                                color: Qt.rgba(0.3, 0.3, 0.3, 0.3)
                                
                                Rectangle {
                                    width: Math.min(parent.width * (avgCpu / 100), parent.width)
                                    height: parent.height
                                    radius: parent.radius
                                    color: "#4a90e2"
                                }
                            }
                            
                            Text {
                                text: avgCpu.toFixed(0) + "%"
                                font.pixelSize: 9
                                color: "#888888"
                                Layout.preferredWidth: 35
                            }
                        }
                        
                        // Memory stats
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            
                            Text {
                                text: statisticsPanel.labelText
                                font.pixelSize: 9
                                color: "#666666"
                                Layout.preferredWidth: 25
                            }
                            
                            // Memory average bar
                            Rectangle {
                                Layout.fillWidth: true
                                height: 4
                                radius: 2
                                color: Qt.rgba(0.3, 0.3, 0.3, 0.3)
                                
                                Rectangle {
                                    width: Math.min(parent.width * (avgMemory / statisticsPanel.maxMemoryScaleMB), parent.width)
                                    height: parent.height
                                    radius: parent.radius
                                    color: "#7c4dff"
                                }
                            }
                            
                            Text {
                                text: avgMemory.toFixed(0) + " MB"
                                font.pixelSize: 9
                                color: "#888888"
                                Layout.preferredWidth: 35
                            }
                        }
                    }
                    
                    // Peak indicator
                    Row {
                        spacing: 8
                        
                        Text {
                            text: "Peak: " + maxCpu.toFixed(0) + "% CPU, " + maxMemory.toFixed(0) + " MB"
                            font.pixelSize: 9
                            color: "#666666"
                        }
                    }
                }
            }
            
            // Empty state
            Item {
                visible: !statisticsViewModel || statisticsViewModel.processCount === 0
                anchors.fill: parent
                
                Column {
                    anchors.centerIn: parent
                    spacing: 8
                    
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "📊"
                        font.pixelSize: 32
                        opacity: 0.3
                    }
                    
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "No active processes"
                        font.pixelSize: 12
                        color: "#666666"
                    }
                    
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Start a process to see statistics"
                        font.pixelSize: 10
                        color: "#555555"
                    }
                }
            }
        }
        
        // Footer with info
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(1, 1, 1, 0.1)
            visible: statisticsViewModel !== null && statisticsViewModel.processCount > 0
        }
        
        Text {
            Layout.fillWidth: true
            text: "💡 Stats update every 2 seconds"
            font.pixelSize: 9
            color: "#555555"
            horizontalAlignment: Text.AlignHCenter
            visible: statisticsViewModel !== null && statisticsViewModel.processCount > 0
        }
    }
} // end GlassCard
} // end Item root
