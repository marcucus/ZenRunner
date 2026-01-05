import QtQuick
import QtQuick.Layouts
import "./components"

// Individual project detail view with scripts and controls
GlassCard {
    id: projectView
    
    // Properties
    property string projectName: "Sample Project"
    property string projectPath: "/path/to/project"
    property var scripts: []
    property bool isRunning: false
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Project header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            // Project icon
            Rectangle {
                width: 48
                height: 48
                radius: 12
                color: Qt.rgba(0.3, 0.6, 1.0, 0.2)
                border.width: 1
                border.color: Qt.rgba(0.3, 0.6, 1.0, 0.3)
                
                Text {
                    anchors.centerIn: parent
                    text: "📦"
                    font.pixelSize: 24
                }
            }
            
            // Project info
            Column {
                Layout.fillWidth: true
                spacing: 4
                
                Text {
                    text: projectView.projectName
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    color: "#ffffff"
                }
                
                Text {
                    text: projectView.projectPath
                    font.pixelSize: 11
                    color: "#888888"
                    elide: Text.ElideMiddle
                }
            }
            
            // Status indicator
            Rectangle {
                width: 80
                height: 28
                radius: 14
                color: projectView.isRunning ? 
                    Qt.rgba(0.3, 0.8, 0.4, 0.2) : 
                    Qt.rgba(0.4, 0.4, 0.4, 0.2)
                
                Row {
                    anchors.centerIn: parent
                    spacing: 6
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: projectView.isRunning ? "#4ade80" : "#6b7280"
                    }
                    
                    Text {
                        text: projectView.isRunning ? "Running" : "Idle"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                }
            }
        }
        
        // Scripts section
        Column {
            Layout.fillWidth: true
            spacing: 12
            
            Text {
                text: "Available Scripts"
                font.pixelSize: 14
                font.weight: Font.DemiBold
                color: "#ffffff"
            }
            
            // Scripts grid
            GridLayout {
                Layout.fillWidth: true
                columns: 3
                rowSpacing: 8
                columnSpacing: 8
                
                Repeater {
                    model: ["dev", "build", "test", "lint", "start", "deploy"]
                    
                    GlassButton {
                        text: modelData
                        accentColor: {
                            switch(modelData) {
                                case "dev":
                                case "start": return "#4a90e2"
                                case "build": return "#9b59b6"
                                case "test": return "#16a085"
                                case "lint": return "#f39c12"
                                case "deploy": return "#e74c3c"
                                default: return "#95a5a6"
                            }
                        }
                        Layout.fillWidth: true
                        
                        // Icon prefix
                        contentItem: Row {
                            spacing: 6
                            anchors.centerIn: parent
                            
                            Text {
                                text: {
                                    switch(modelData) {
                                        case "dev": return "⚡"
                                        case "build": return "🔨"
                                        case "test": return "✓"
                                        case "lint": return "🔍"
                                        case "start": return "▶"
                                        case "deploy": return "🚀"
                                        default: return "●"
                                    }
                                }
                                font.pixelSize: 14
                            }
                            
                            Text {
                                text: modelData
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                        }
                    }
                }
            }
        }
        
        // Control buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            GlassButton {
                text: "View Logs"
                accentColor: "#3498db"
                Layout.fillWidth: true
            }
            
            GlassButton {
                text: "Terminal"
                accentColor: "#2ecc71"
                Layout.fillWidth: true
            }
            
            GlassButton {
                text: "Settings"
                accentColor: "#95a5a6"
                Layout.fillWidth: true
            }
        }
    }
}
