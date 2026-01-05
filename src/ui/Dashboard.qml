import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import "./components"

// Main dashboard view - project overview and quick actions
Item {
    id: dashboard
    
    // File dialog for folder selection
    FolderDialog {
        id: folderDialog
        title: "Select Project Folder to Scan"
        onAccepted: {
            console.log("Selected folder:", selectedFolder)
            projectManager.scanFolder(selectedFolder, 3)
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 20
        
        // Header with title and controls
        GlassCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            glassOpacity: 0.12
            
            RowLayout {
                anchors.fill: parent
                spacing: 16
                
                // Application title
                Column {
                    Layout.fillWidth: true
                    spacing: 4
                    
                    Text {
                        text: "ZenRunner"
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        color: "#ffffff"
                    }
                    
                    Text {
                        text: "High-Performance Process Manager"
                        font.pixelSize: 13
                        color: "#888888"
                    }
                }
                
                // Quick action buttons
                Row {
                    spacing: 12
                    
                    GlassButton {
                        text: "+ Import Project"
                        accentColor: "#4a90e2"
                        width: 140
                    }
                    
                    GlassButton {
                        text: "+ New Workspace"
                        accentColor: "#7c4dff"
                        width: 150
                    }
                }
            }
        }
        
        // Content area with projects and workspaces
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20
            
            // Projects list panel
            GlassCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.6
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12
                    
                    // Projects header
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        
                        Text {
                            text: "Projects"
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                        }
                        
                        Rectangle {
                            width: 32
                            height: 20
                            radius: 10
                            color: Qt.rgba(0.3, 0.6, 1.0, 0.2)
                            
                            Text {
                                anchors.centerIn: parent
                                text: projectManager.projectCount.toString()
                                font.pixelSize: 11
                                font.weight: Font.Medium
                                color: "#4a90e2"
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        GlassButton {
                            text: "Scan Folder"
                            width: 110
                            implicitHeight: 32
                            onClicked: {
                                folderDialog.open()
                            }
                        }
                    }
                    
                    // Projects list
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 12
                        clip: true
                        
                        model: projectManager
                        
                        delegate: GlassCard {
                            width: ListView.view.width
                            glassOpacity: 0.08
                            
                            required property string name
                            required property string path
                            required property int scriptCount
                            required property var scripts
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 8
                                
                                // Project header
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    
                                    Text {
                                        text: name
                                        font.pixelSize: 16
                                        font.weight: Font.DemiBold
                                        color: "#ffffff"
                                        Layout.fillWidth: true
                                    }
                                    
                                    Rectangle {
                                        width: 50
                                        height: 20
                                        radius: 10
                                        color: Qt.rgba(0.3, 0.6, 1.0, 0.15)
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: scriptCount + " scripts"
                                            font.pixelSize: 10
                                            color: "#4a90e2"
                                        }
                                    }
                                }
                                
                                // Project path
                                Text {
                                    text: path
                                    font.pixelSize: 11
                                    color: "#888888"
                                    elide: Text.ElideMiddle
                                    Layout.fillWidth: true
                                }
                                
                                // Script action buttons
                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    
                                    Repeater {
                                        model: scripts
                                        
                                        GlassButton {
                                            required property var modelData
                                            
                                            text: modelData.name
                                            implicitHeight: 32
                                            width: Math.max(80, implicitWidth)
                                            accentColor: {
                                                const scriptName = modelData.name.toLowerCase()
                                                if (scriptName === "start" || scriptName === "dev") return "#4ade80"
                                                if (scriptName === "test") return "#fbbf24"
                                                if (scriptName === "build") return "#60a5fa"
                                                if (scriptName === "lint") return "#a78bfa"
                                                return "#4a90e2"
                                            }
                                            
                                            onClicked: {
                                                console.log("Running script:", modelData.name, "with command:", modelData.command)
                                                // TODO: Execute script via ProcessManager
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Empty state
                        Item {
                            visible: projectManager.projectCount === 0
                            anchors.fill: parent
                            
                            Column {
                                anchors.centerIn: parent
                                spacing: 12
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "📦"
                                    font.pixelSize: 48
                                    opacity: 0.3
                                }
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "No projects yet"
                                    font.pixelSize: 14
                                    color: "#666666"
                                }
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "Click 'Scan Folder' to detect projects"
                                    font.pixelSize: 12
                                    color: "#555555"
                                }
                            }
                        }
                    }
                }
            }
            
            // Sidebar with workspaces and status
            ColumnLayout {
                Layout.preferredWidth: parent.width * 0.35
                Layout.fillHeight: true
                spacing: 20
                
                // Workspaces panel
                GlassCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12
                        
                        Text {
                            text: "Workspaces"
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                        }
                        
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 8
                            color: Qt.rgba(0, 0, 0, 0.2)
                            
                            Column {
                                anchors.centerIn: parent
                                spacing: 8
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "🗂️"
                                    font.pixelSize: 32
                                    opacity: 0.3
                                }
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "No workspaces"
                                    font.pixelSize: 12
                                    color: "#666666"
                                }
                            }
                        }
                    }
                }
                
                // System status panel
                GlassCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    glassOpacity: 0.1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 16
                        
                        Text {
                            text: "System Status"
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                        }
                        
                        // Performance metrics
                        Column {
                            Layout.fillWidth: true
                            spacing: 12
                            
                            // Memory usage
                            StatusIndicator {
                                width: parent.width
                                label: "Memory"
                                value: "< 15 MB"
                                status: "optimal"
                            }
                            
                            // Active processes
                            StatusIndicator {
                                width: parent.width
                                label: "Processes"
                                value: "0 active"
                                status: "idle"
                            }
                            
                            // UI Performance
                            StatusIndicator {
                                width: parent.width
                                label: "UI Framerate"
                                value: "60 FPS"
                                status: "optimal"
                            }
                        }
                        
                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }
    }
    
    // Status indicator component
    component StatusIndicator: Row {
        property string label: ""
        property string value: ""
        property string status: "idle" // idle, optimal, warning, error
        
        spacing: 8
        
        Rectangle {
            width: 8
            height: 8
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: {
                switch(status) {
                    case "optimal": return "#4ade80"
                    case "warning": return "#fbbf24"
                    case "error": return "#ef4444"
                    default: return "#6b7280"
                }
            }
            
            SequentialAnimation on opacity {
                running: status !== "idle"
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 1000; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
            }
        }
        
        Column {
            spacing: 2
            
            Text {
                text: label
                font.pixelSize: 11
                color: "#888888"
            }
            
            Text {
                text: value
                font.pixelSize: 13
                font.weight: Font.Medium
                color: "#ffffff"
            }
        }
    }
}
