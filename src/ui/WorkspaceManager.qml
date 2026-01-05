import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "./components"

// Workspace management interface for grouping and controlling projects
GlassCard {
    id: workspaceManager
    
    // Property to hold the workspace view model (set from C++)
    property var workspaceViewModel: null
    
    // Dialog for creating/editing workspaces
    WorkspaceDialog {
        id: workspaceDialog
        
        onAccepted: {
            if (workspaceDialog.isEditMode) {
                // Update existing workspace
                if (workspaceViewModel) {
                    workspaceViewModel.updateWorkspace(
                        workspaceDialog.workspaceId,
                        workspaceDialog.workspaceName,
                        workspaceDialog.workspaceDescription
                    )
                }
            } else {
                // Create new workspace
                if (workspaceViewModel) {
                    workspaceViewModel.createWorkspace(
                        workspaceDialog.workspaceName,
                        workspaceDialog.workspaceDescription
                    )
                }
            }
        }
    }
    
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
                onClicked: {
                    workspaceDialog.resetForNew()
                    workspaceDialog.open()
                }
            }
        }
        
        // Workspaces list
        ListView {
            id: workspaceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            clip: true
            
            // Use the workspace view model
            model: workspaceViewModel
            
            delegate: GlassCard {
                width: workspaceList.width
                height: 140
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
                                text: model.name ? model.name.charAt(0).toUpperCase() : "?"
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
                                text: model.description || "No description"
                                font.pixelSize: 11
                                color: "#888888"
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }
                            
                            Text {
                                text: model.projectCount + " project" + 
                                      (model.projectCount !== 1 ? "s" : "")
                                font.pixelSize: 12
                                color: "#666666"
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
                            onClicked: {
                                if (model.isRunning) {
                                    // Stop all processes in workspace
                                    if (workspaceViewModel) {
                                        workspaceViewModel.stopAllProjects(model.workspaceId, false)
                                    }
                                } else {
                                    // Start all processes in workspace (parallel by default)
                                    if (workspaceViewModel) {
                                        workspaceViewModel.startAllProjects(model.workspaceId, "dev", true)
                                    }
                                }
                            }
                        }
                        
                        GlassButton {
                            text: "⚙️"
                            width: 40
                            implicitHeight: 32
                            accentColor: "#95a5a6"
                            onClicked: {
                                // Open edit dialog
                                workspaceDialog.loadWorkspace(
                                    model.workspaceId,
                                    model.name,
                                    model.description
                                )
                                workspaceDialog.open()
                            }
                        }
                        
                        GlassButton {
                            text: "🗑️"
                            width: 40
                            implicitHeight: 32
                            accentColor: "#e74c3c"
                            onClicked: {
                                // Delete workspace (with confirmation)
                                deleteConfirmDialog.workspaceId = model.workspaceId
                                deleteConfirmDialog.workspaceName = model.name
                                deleteConfirmDialog.open()
                            }
                        }
                    }
                    
                    // Execution mode selector (shown when hovering)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: mouseArea.containsMouse && !model.isRunning
                        opacity: visible ? 1.0 : 0.0
                        
                        Behavior on opacity {
                            NumberAnimation { duration: 200 }
                        }
                        
                        Text {
                            text: "Execution:"
                            font.pixelSize: 10
                            color: "#888888"
                        }
                        
                        GlassButton {
                            text: "⚡ Parallel"
                            Layout.fillWidth: true
                            implicitHeight: 24
                            accentColor: "#3498db"
                            onClicked: {
                                if (workspaceViewModel) {
                                    workspaceViewModel.startAllProjects(model.workspaceId, "dev", true)
                                }
                            }
                        }
                        
                        GlassButton {
                            text: "➡️ Sequential"
                            Layout.fillWidth: true
                            implicitHeight: 24
                            accentColor: "#9b59b6"
                            onClicked: {
                                if (workspaceViewModel) {
                                    workspaceViewModel.startAllProjects(model.workspaceId, "dev", false)
                                }
                            }
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
                        onClicked: {
                            workspaceDialog.resetForNew()
                            workspaceDialog.open()
                        }
                    }
                }
            }
        }
    }
    
    // Delete confirmation dialog
    Dialog {
        id: deleteConfirmDialog
        
        property string workspaceId: ""
        property string workspaceName: ""
        
        title: "Delete Workspace"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        
        width: 400
        anchors.centerIn: parent
        
        background: Rectangle {
            color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
            border.color: Qt.rgba(1, 1, 1, 0.1)
            border.width: 1
            radius: 12
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 16
            
            Text {
                Layout.fillWidth: true
                text: "Are you sure you want to delete workspace \"" + deleteConfirmDialog.workspaceName + "\"?"
                font.pixelSize: 14
                color: "#ffffff"
                wrapMode: Text.WordWrap
            }
            
            Text {
                Layout.fillWidth: true
                text: "⚠️ This action cannot be undone. All workspace settings will be lost, but projects will remain intact."
                font.pixelSize: 12
                color: "#f39c12"
                wrapMode: Text.WordWrap
            }
        }
        
        onAccepted: {
            if (workspaceViewModel) {
                workspaceViewModel.deleteWorkspace(deleteConfirmDialog.workspaceId)
            }
        }
    }
}
