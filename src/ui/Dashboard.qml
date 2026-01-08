import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./components"

// Main dashboard view - project overview and quick actions
Item {
    id: dashboard
    
    // Track active processes
    property var activeProcesses: ({})
    
    // ProcessState enum values
    readonly property int processStateNotStarted: 0
    readonly property int processStateStarting: 1
    readonly property int processStateRunning: 2
    readonly property int processStatePaused: 3
    readonly property int processStateStopping: 4
    readonly property int processStateStopped: 5
    readonly property int processStateCrashed: 6
    readonly property int processStateFinished: 7
    
    // Listen to processManager signals
    Connections {
        target: processManager
        
        function onProcessStateChanged(id, newState) {
            console.log("Process state changed:", id, newState)
            
            // Remove from active processes if stopped, crashed, or finished
            if (newState === processStateStopped || 
                newState === processStateCrashed || 
                newState === processStateFinished) {
                if (activeProcesses[id] !== undefined) {
                    delete activeProcesses[id]
                    dashboard.activeProcessesChanged()
                }
            }
        }
        
        function onProcessFinished(id, exitCode) {
            console.log("Process finished:", id, "Exit code:", exitCode)
            if (activeProcesses[id] !== undefined) {
                delete activeProcesses[id]
                dashboard.activeProcessesChanged()
            }
        }
        
        function onProcessError(id, error) {
            console.log("Process error:", id, error)
        }
    }
    
    // Listen to projectManager signals
    Connections {
        target: projectManager
        
        function onProjectsDetected(count) {
            console.log("Dashboard: Projects detected:", count)
        }
        
        function onScanComplete(success, message) {
            console.log("Dashboard: Scan complete:", success, message)
        }
        
        function onProjectCountChanged() {
            console.log("Dashboard: Project count changed to:", projectManager.projectCount)
        }
    }
    
    // File dialog for folder selection
    FolderDialog {
        id: folderDialog
        title: "Select Project Folder to Scan"
        onAccepted: {
            console.log("Selected folder:", selectedFolder)
            projectManager.scanFolder(selectedFolder, 3)
        }
    }
    
    // Workspace creation dialog
    Dialog {
        id: workspaceCreationDialog
        title: "Create New Workspace"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 500
        height: 600
        
        property var selectedProjectIndices: []
        
        onOpened: {
            selectedProjectIndices = []
            workspaceNameField.text = ""
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 16
            
            Text {
                text: "Workspace Name"
                font.pixelSize: 14
                color: "#ffffff"
            }
            
            TextField {
                id: workspaceNameField
                Layout.fillWidth: true
                placeholderText: "Enter workspace name..."
                font.pixelSize: 14
            }
            
            Text {
                text: "Select Projects"
                font.pixelSize: 14
                color: "#ffffff"
                Layout.topMargin: 8
            }
            
            // Project selection list
            ListView {
                id: projectSelectionList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 8
                
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
                
                model: projectManager
                
                delegate: Rectangle {
                    width: ListView.view.width
                    height: 40
                    color: checkBox.checked ? Qt.rgba(0.3, 0.6, 1.0, 0.2) : Qt.rgba(1, 1, 1, 0.05)
                    radius: 8
                    
                    required property int index
                    required property string name
                    required property string path
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8
                        
                        CheckBox {
                            id: checkBox
                            checked: false
                            
                            onCheckedChanged: {
                                if (checked) {
                                    var arr = workspaceCreationDialog.selectedProjectIndices
                                    arr.push(index)
                                    workspaceCreationDialog.selectedProjectIndices = arr
                                } else {
                                    var arr = workspaceCreationDialog.selectedProjectIndices
                                    const idx = arr.indexOf(index)
                                    if (idx > -1) {
                                        arr.splice(idx, 1)
                                        workspaceCreationDialog.selectedProjectIndices = arr
                                    }
                                }
                            }
                        }
                        
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            
                            Text {
                                text: name
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                                color: "#ffffff"
                            }
                            
                            Text {
                                text: path
                                font.pixelSize: 10
                                color: "#888888"
                                elide: Text.ElideMiddle
                                width: parent.width
                            }
                        }
                    }
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Cancel"
                    onClicked: workspaceCreationDialog.close()
                }
                
                Button {
                    text: "Create"
                    enabled: workspaceNameField.text.trim() !== "" && workspaceCreationDialog.selectedProjectIndices.length > 0
                    onClicked: {
                        console.log("Creating workspace:", workspaceNameField.text, "with", workspaceCreationDialog.selectedProjectIndices.length, "projects")
                        
                        // For now, just create the workspace with name and description
                        // TODO: Add projects to workspace after creation
                        workspaceViewModel.createWorkspace(workspaceNameField.text, workspaceCreationDialog.selectedProjectIndices.length + " projects selected")
                        
                        // Clear and close
                        workspaceNameField.text = ""
                        workspaceCreationDialog.selectedProjectIndices = []
                        workspaceCreationDialog.close()
                    }
                }
            }
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
                        onClicked: {
                            workspaceCreationDialog.open()
                        }
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
                        id: projectListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 12
                        clip: true
                        
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }
                        
                        model: projectManager
                        
                        // Empty state message
                        Text {
                            anchors.centerIn: parent
                            visible: projectListView.count === 0
                            text: "No projects found\nClick 'Scan Folder' to add projects"
                            font.pixelSize: 14
                            color: "#666666"
                            horizontalAlignment: Text.AlignHCenter
                        }
                        
                        delegate: GlassCard {
                            width: ListView.view.width
                            height: 150  // Fixed height for now
                            glassOpacity: 0.08
                            
                            required property int index
                            required property string name
                            required property string path
                            required property int scriptCount
                            required property var scripts
                            
                            Component.onCompleted: {
                                console.log("Project delegate created:", index, name, "with", scriptCount, "scripts")
                            }
                            
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
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
                                    id: scriptFlow
                                    Layout.fillWidth: true
                                    spacing: 8
                                    
                                    Repeater {
                                        model: scripts
                                        
                                        Rectangle {
                                            required property var modelData
                                            
                                            width: Math.max(100, scriptButton.implicitWidth + 50)
                                            height: 32
                                            radius: 8
                                            color: "transparent"
                                            
                                            property string processId: name + "_" + (modelData && modelData.name ? modelData.name : "unknown")
                                            property bool isRunning: activeProcesses[processId] !== undefined
                                            
                                            Row {
                                                anchors.fill: parent
                                                spacing: 4
                                                
                                                // Status indicator
                                                Rectangle {
                                                    width: 8
                                                    height: 8
                                                    radius: 4
                                                    anchors.verticalCenter: parent.verticalCenter
                                                    visible: parent.parent.isRunning
                                                    color: "#4ade80"
                                                    
                                                    SequentialAnimation on opacity {
                                                        running: parent.visible
                                                        loops: Animation.Infinite
                                                        NumberAnimation { to: 0.3; duration: 800 }
                                                        NumberAnimation { to: 1.0; duration: 800 }
                                                    }
                                                }
                                                
                                                // Start/Script button
                                                GlassButton {
                                                    id: scriptButton
                                                    text: modelData && modelData.name ? modelData.name : "Unknown"
                                                    implicitHeight: 32
                                                    width: parent.parent.isRunning ? parent.parent.width - 40 : parent.parent.width
                                                    visible: !parent.parent.isRunning
                                                    accentColor: {
                                                        if (!modelData || !modelData.name) return "#4a90e2"
                                                        const scriptName = modelData.name.toLowerCase()
                                                        if (scriptName === "start" || scriptName === "dev") return "#4ade80"
                                                        if (scriptName === "test") return "#fbbf24"
                                                        if (scriptName === "build") return "#60a5fa"
                                                        if (scriptName === "lint") return "#a78bfa"
                                                        return "#4a90e2"
                                                    }
                                                    
                                                    onClicked: {
                                                        if (modelData && modelData.name && modelData.command) {
                                                            console.log("Running script:", modelData.name, "in project:", name)
                                                            console.log("Path:", path, "Command:", modelData.command)
                                                            
                                                            var pid = parent.parent.processId
                                                            // Pass the script name, not the command
                                                            var success = processManager.runScript(pid, modelData.name, path)
                                                            if (success) {
                                                                console.log("Process started successfully:", pid)
                                                                activeProcesses[pid] = {
                                                                    name: modelData.name,
                                                                    projectName: name,
                                                                    path: path
                                                                }
                                                                dashboard.activeProcessesChanged()
                                                            } else {
                                                                console.log("Failed to start process:", pid)
                                                            }
                                                        }
                                                    }
                                                }
                                                
                                                // Running state button
                                                GlassButton {
                                                    text: modelData && modelData.name ? modelData.name : "Running"
                                                    implicitHeight: 32
                                                    width: parent.parent.width - 40
                                                    visible: parent.parent.isRunning
                                                    accentColor: "#4ade80"
                                                    enabled: false
                                                }
                                                
                                                // Stop button
                                                GlassButton {
                                                    text: "⏹"
                                                    implicitHeight: 32
                                                    width: 32
                                                    visible: parent.parent.isRunning
                                                    accentColor: "#ef4444"
                                                    
                                                    onClicked: {
                                                        var pid = parent.parent.processId
                                                        console.log("Stopping process:", pid)
                                                        processManager.stopProcess(pid)
                                                        delete activeProcesses[pid]
                                                        dashboard.activeProcessesChanged()
                                                    }
                                                }
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
                WorkspaceManager {
                    id: workspaceManagerPanel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    // This property will be set from C++ (main.cpp)
                    // workspaceViewModel: workspaceViewModelInstance
                }
                
                // Active Processes panel with tabs
                GlassCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    glassOpacity: 0.1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12
                        
                        // Tab bar
                        Row {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            GlassButton {
                                text: "📊 Statistics"
                                width: 120
                                implicitHeight: 32
                                accentColor: processTabBar.currentIndex === 0 ? "#4a90e2" : "#666666"
                                onClicked: processTabBar.currentIndex = 0
                            }
                            
                            GlassButton {
                                text: "📋 Logs"
                                width: 100
                                implicitHeight: 32
                                accentColor: processTabBar.currentIndex === 1 ? "#4a90e2" : "#666666"
                                onClicked: processTabBar.currentIndex = 1
                            }
                        }
                        
                        // Tab content
                        StackLayout {
                            id: processTabBar
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            currentIndex: 0
                            
                            // Statistics tab
                            Statistics {
                                id: statisticsPanel
                                // This property will be set from C++ (main.cpp)
                                // statisticsViewModel: statisticsViewModelInstance
                            }
                            
                            // Logs tab
                            Item {
                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 8
                                    
                                    Text {
                                        text: Object.keys(activeProcesses).length > 0 ? 
                                              "Active Processes (" + Object.keys(activeProcesses).length + ")" : 
                                              "No Active Processes"
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: "#ffffff"
                                    }
                                    
                                    ScrollView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        clip: true
                                        
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 8
                                            
                                            Repeater {
                                                model: Object.keys(activeProcesses)
                                                
                                                delegate: GlassCard {
                                                    Layout.fillWidth: true
                                                    Layout.preferredHeight: 120
                                                    glassOpacity: 0.08
                                                    
                                                    required property string modelData
                                                    
                                                    property var processInfo: activeProcesses[modelData] || {}
                                                    
                                                    ColumnLayout {
                                                        anchors.fill: parent
                                                        anchors.margins: 12
                                                        spacing: 8
                                                        
                                                        RowLayout {
                                                            Layout.fillWidth: true
                                                            
                                                            Rectangle {
                                                                width: 8
                                                                height: 8
                                                                radius: 4
                                                                color: "#4ade80"
                                                                
                                                                SequentialAnimation on opacity {
                                                                    running: true
                                                                    loops: Animation.Infinite
                                                                    NumberAnimation { to: 0.3; duration: 800 }
                                                                    NumberAnimation { to: 1.0; duration: 800 }
                                                                }
                                                            }
                                                            
                                                            Text {
                                                                text: processInfo.projectName || "Unknown"
                                                                font.pixelSize: 13
                                                                font.weight: Font.DemiBold
                                                                color: "#ffffff"
                                                            }
                                                            
                                                            Text {
                                                                text: " › " + (processInfo.name || "Unknown")
                                                                font.pixelSize: 12
                                                                color: "#4a90e2"
                                                            }
                                                            
                                                            Item { Layout.fillWidth: true }
                                                            
                                                            GlassButton {
                                                                text: "⏹ Stop"
                                                                width: 70
                                                                implicitHeight: 24
                                                                accentColor: "#ef4444"
                                                                
                                                                onClicked: {
                                                                    console.log("Stopping:", modelData)
                                                                    processManager.stopProcess(modelData)
                                                                    delete activeProcesses[modelData]
                                                                    dashboard.activeProcessesChanged()
                                                                }
                                                            }
                                                        }
                                                        
                                                        Text {
                                                            text: processInfo.path || "No path"
                                                            font.pixelSize: 10
                                                            color: "#888888"
                                                            elide: Text.ElideMiddle
                                                            Layout.fillWidth: true
                                                        }
                                                        
                                                        Rectangle {
                                                            Layout.fillWidth: true
                                                            Layout.fillHeight: true
                                                            radius: 4
                                                            color: "#0a0a0a"
                                                            border.width: 1
                                                            border.color: Qt.rgba(1, 1, 1, 0.1)
                                                            
                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "Process logs will appear here...\nClick 📋 to view full console"
                                                                font.pixelSize: 10
                                                                font.family: "monospace"
                                                                color: "#666666"
                                                                horizontalAlignment: Text.AlignHCenter
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                // System status panel
                GlassCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
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
                                value: Object.keys(activeProcesses).length + " active"
                                status: Object.keys(activeProcesses).length > 0 ? "optimal" : "idle"
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
