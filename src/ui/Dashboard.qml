import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./components"

// Main dashboard view - project overview and quick actions
Item {
    id: dashboard
    
    // Toast notification for in-app feedback
    Toast {
        id: toast
        z: 1000
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
    
    // Listen to processManager signals for global notifications
    Connections {
        target: processManager
        
        function onProcessFinished(id, exitCode) {
            console.log("Dashboard: Process finished:", id, "exit code:", exitCode)
            if (exitCode === 0) {
                toast.show("Process '" + id + "' completed successfully", Qt.rgba(0.2, 0.6, 0.3, 0.95))
            } else {
                toast.show("Process '" + id + "' exited with code " + exitCode, Qt.rgba(0.8, 0.6, 0.2, 0.95))
            }
        }
        
        function onProcessCrashed(id, exitCode) {
            console.log("Dashboard: Process crashed:", id, "exit code:", exitCode)
            toast.show("Process '" + id + "' crashed!", Qt.rgba(0.8, 0.2, 0.2, 0.95))
        }
        
        function onProcessError(id, error) {
            console.log("Dashboard: Process error:", id, error)
            toast.show("Error in '" + id + "': " + error, Qt.rgba(0.8, 0.2, 0.2, 0.95))
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
        property var selectedProjectIds: []  // Store project IDs for adding to workspace
        property string pendingWorkspaceId: ""  // Store workspace ID after creation
        property bool isCreating: false  // Flag to prevent multiple simultaneous creations
        
        onOpened: {
            selectedProjectIndices = []
            selectedProjectIds = []
            pendingWorkspaceId = ""
            isCreating = false
            workspaceNameField.text = ""
        }
        
        // Listen for workspace creation to add projects
        Connections {
            target: workspaceViewModel
            
            function onWorkspaceCreated(workspaceId) {
                // Check if this is our pending workspace creation
                // Only proceed if we're in creation mode, have projects to add, and haven't processed yet
                if (workspaceCreationDialog.isCreating && 
                    workspaceCreationDialog.selectedProjectIds.length > 0 && 
                    workspaceCreationDialog.pendingWorkspaceId === "") {
                    workspaceCreationDialog.pendingWorkspaceId = workspaceId
                    
                    // Add each selected project to the workspace
                    console.log("Adding", workspaceCreationDialog.selectedProjectIds.length, "projects to workspace", workspaceId)
                    for (var i = 0; i < workspaceCreationDialog.selectedProjectIds.length; i++) {
                        const projectId = workspaceCreationDialog.selectedProjectIds[i]
                        console.log("Adding project", projectId, "to workspace", workspaceId)
                        workspaceViewModel.addProjectToWorkspace(workspaceId, projectId)
                    }
                    
                    // Clear state and close
                    workspaceCreationDialog.isCreating = false
                    workspaceNameField.text = ""
                    workspaceCreationDialog.selectedProjectIndices = []
                    workspaceCreationDialog.selectedProjectIds = []
                    workspaceCreationDialog.close()
                }
            }
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
                    required property string projectId
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
                                    
                                    var ids = workspaceCreationDialog.selectedProjectIds
                                    ids.push(projectId)
                                    workspaceCreationDialog.selectedProjectIds = ids
                                } else {
                                    var arr = workspaceCreationDialog.selectedProjectIndices
                                    const idx = arr.indexOf(index)
                                    if (idx > -1) {
                                        arr.splice(idx, 1)
                                        workspaceCreationDialog.selectedProjectIndices = arr
                                    }
                                    
                                    var ids = workspaceCreationDialog.selectedProjectIds
                                    const idIdx = ids.indexOf(projectId)
                                    if (idIdx > -1) {
                                        ids.splice(idIdx, 1)
                                        workspaceCreationDialog.selectedProjectIds = ids
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
                    enabled: !workspaceCreationDialog.isCreating && 
                             workspaceNameField.text.trim() !== "" && 
                             workspaceCreationDialog.selectedProjectIds.length > 0
                    onClicked: {
                        console.log("Creating workspace:", workspaceNameField.text, "with", workspaceCreationDialog.selectedProjectIds.length, "projects")
                        
                        // Set flag to indicate we're creating a workspace
                        workspaceCreationDialog.isCreating = true
                        
                        // Create the workspace - projects will be added via the workspaceCreated signal handler
                        workspaceViewModel.createWorkspace(
                            workspaceNameField.text, 
                            workspaceCreationDialog.selectedProjectIds.length + " projects"
                        )
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
                                        
                                        GlassButton {
                                            required property var modelData
                                            required property int index
                                            
                                            // Access parent delegate's name and path properties
                                            property string projectName: name
                                            property string projectPath: path
                                            property string processId: (projectName || "unknown") + "_" + (modelData && modelData.name ? modelData.name : "")
                                            property bool isRunning: false
                                            
                                            // ProcessState enum values for clarity
                                            readonly property int processStateRunning: 2
                                            
                                            text: {
                                                if (!modelData || !modelData.name) return "Unknown"
                                                if (isRunning) return modelData.name + " ●"
                                                return modelData.name
                                            }
                                            implicitHeight: 32
                                            width: Math.max(80, implicitWidth)
                                            accentColor: {
                                                if (isRunning) return "#4ade80"
                                                if (!modelData || !modelData.name) return "#4a90e2"
                                                const scriptName = modelData.name.toLowerCase()
                                                if (scriptName === "start" || scriptName === "dev") return "#4ade80"
                                                if (scriptName === "test") return "#fbbf24"
                                                if (scriptName === "build") return "#60a5fa"
                                                if (scriptName === "lint") return "#a78bfa"
                                                return "#4a90e2"
                                            }
                                            
                                            // Listen for process state changes
                                            Connections {
                                                target: processManager
                                                
                                                function onProcessStateChanged(id, newState) {
                                                    if (id === processId) {
                                                        // ProcessState enum: NotStarted=0, Starting=1, Running=2, Paused=3, Stopping=4, Stopped=5, Finished=6, Crashed=7
                                                        isRunning = (newState === processStateRunning)
                                                    }
                                                }
                                            }
                                            
                                            onClicked: {
                                                if (modelData && modelData.name && modelData.command) {
                                                    console.log("Running script:", modelData.name, "in project:", projectName)
                                                    console.log("Path:", projectPath, "Command:", modelData.command)
                                                    
                                                    var success = processManager.runScript(processId, modelData.command, projectPath)
                                                    if (success) {
                                                        console.log("Process started successfully:", processId)
                                                        // Show in-app toast notification
                                                        toast.show("Started '" + modelData.name + "' in " + projectName, Qt.rgba(0.2, 0.6, 0.3, 0.95))
                                                        // Show native notification if available
                                                        if (typeof platformManager !== 'undefined' && platformManager && platformManager.showNotification) {
                                                            platformManager.showNotification(
                                                                "Script Started",
                                                                "Running '" + modelData.name + "' in " + projectName
                                                            )
                                                        }
                                                    } else {
                                                        console.log("Failed to start process:", processId)
                                                        // Show error toast notification
                                                        toast.show("Failed to start '" + modelData.name + "'", Qt.rgba(0.8, 0.2, 0.2, 0.95))
                                                        // Show native notification if available
                                                        if (typeof platformManager !== 'undefined' && platformManager && platformManager.showNotification) {
                                                            platformManager.showNotification(
                                                                "Failed to Start",
                                                                "Could not start '" + modelData.name + "' in " + projectName
                                                            )
                                                        }
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
                    
                    // Pass the global workspaceViewModel context property
                    workspaceViewModel: workspaceViewModel
                }
                
                // Statistics panel
                Statistics {
                    id: statisticsPanel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    // This property will be set from C++ (main.cpp)
                    // statisticsViewModel: statisticsViewModelInstance
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
