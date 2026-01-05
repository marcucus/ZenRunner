import QtQuick
import QtQuick.Layouts
import "./components"

// Workspace management interface for grouping and controlling projects
GlassCard {
    id: workspaceManager
    
    property var workspaces: []
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            Text {
                text: "🗂️ Workspace Manager"
                font.pixelSize: 20
                font.weight: Font.Bold
                color: "#ffffff"
            }
            
            Item { Layout.fillWidth: true }
            
            GlassButton {
                text: "+ Create Workspace"
                accentColor: "#7c4dff"
                width: 160
            }
        }
        
        // Workspaces list
        ListView {
            id: workspaceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            clip: true
            
            // Placeholder model
            model: ListModel {
                ListElement {
                    name: "Full Stack Development"
                    projectCount: 3
                    isRunning: true
                    color: "#4a90e2"
                }
                ListElement {
                    name: "Microservices"
                    projectCount: 5
                    isRunning: false
                    color: "#9b59b6"
                }
                ListElement {
                    name: "Frontend Only"
                    projectCount: 1
                    isRunning: false
                    color: "#16a085"
                }
            }
            
            delegate: GlassCard {
                width: workspaceList.width
                height: 120
                glassOpacity: mouseArea.containsMouse ? 0.2 : 0.15
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12
                    
                    // Workspace header
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        
                        // Color indicator
                        Rectangle {
                            width: 40
                            height: 40
                            radius: 8
                            color: Qt.rgba(
                                parseInt(model.color.substring(1, 3), 16) / 255,
                                parseInt(model.color.substring(3, 5), 16) / 255,
                                parseInt(model.color.substring(5, 7), 16) / 255,
                                0.2
                            )
                            border.width: 1
                            border.color: model.color
                            
                            Text {
                                anchors.centerIn: parent
                                text: model.name.charAt(0)
                                font.pixelSize: 18
                                font.weight: Font.Bold
                                color: model.color
                            }
                        }
                        
                        // Workspace info
                        Column {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            Text {
                                text: model.name
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                color: "#ffffff"
                            }
                            
                            Text {
                                text: model.projectCount + " project" + 
                                      (model.projectCount !== 1 ? "s" : "")
                                font.pixelSize: 12
                                color: "#888888"
                            }
                        }
                        
                        // Status
                        Rectangle {
                            width: 70
                            height: 24
                            radius: 12
                            color: model.isRunning ? 
                                Qt.rgba(0.3, 0.8, 0.4, 0.2) : 
                                Qt.rgba(0.4, 0.4, 0.4, 0.2)
                            
                            Row {
                                anchors.centerIn: parent
                                spacing: 4
                                
                                Rectangle {
                                    width: 6
                                    height: 6
                                    radius: 3
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: model.isRunning ? "#4ade80" : "#6b7280"
                                }
                                
                                Text {
                                    text: model.isRunning ? "Active" : "Idle"
                                    font.pixelSize: 10
                                    color: "#ffffff"
                                }
                            }
                        }
                    }
                    
                    // Control buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        
                        GlassButton {
                            text: model.isRunning ? "⏸ Stop All" : "▶ Start All"
                            accentColor: model.isRunning ? "#e74c3c" : "#4ade80"
                            Layout.fillWidth: true
                            implicitHeight: 32
                        }
                        
                        GlassButton {
                            text: "⚙️"
                            width: 40
                            implicitHeight: 32
                            accentColor: "#95a5a6"
                        }
                        
                        GlassButton {
                            text: "📊"
                            width: 40
                            implicitHeight: 32
                            accentColor: "#3498db"
                        }
                    }
                }
                
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                }
            }
            
            // Empty state
            Rectangle {
                visible: workspaceList.count === 0
                anchors.centerIn: parent
                width: parent.width * 0.8
                height: 200
                color: "transparent"
                
                Column {
                    anchors.centerIn: parent
                    spacing: 16
                    
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "🗂️"
                        font.pixelSize: 64
                        opacity: 0.3
                    }
                    
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "No Workspaces Yet"
                        font.pixelSize: 18
                        font.weight: Font.DemiBold
                        color: "#666666"
                    }
                    
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Create a workspace to group related projects together"
                        font.pixelSize: 13
                        color: "#555555"
                    }
                    
                    GlassButton {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "+ Create Your First Workspace"
                        accentColor: "#7c4dff"
                        width: 240
                        height: 40
                    }
                }
            }
        }
    }
}
