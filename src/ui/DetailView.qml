import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "./components"

// Vue détaillée d'un projet ou workspace avec terminaux
Item {
    id: root
    
    property var selectedItem: null
    property bool isWorkspace: false
    
    signal backClicked()
    
    // Track active processes for terminals
    property var activeProcesses: ({})
    
    // Connections pour nettoyer les processus terminés
    Connections {
        target: processManager
        
        function onProcessFinished(id, exitCode) {
            if (activeProcesses[id]) {
                console.log("Process finished:", id, "- removing from active processes")
                delete activeProcesses[id]
                activeProcesses = Object.assign({}, activeProcesses)
            }
        }
    }
    
    // Workspace projects (si c'est un workspace)
    property var workspaceProjects: []
    property int currentProjectIndex: 0
    
    Component.onCompleted: {
        if (isWorkspace && workspaceViewModel && selectedItem) {
            console.log("Loading workspace projects for index:", selectedItem.index)
            // Charger les projets du workspace
            var projects = workspaceViewModel.getWorkspaceProjects(selectedItem.index)
            console.log("Loaded projects:", projects.length)
            workspaceProjects = projects
        }
    }
    
    // Recharger quand selectedItem change
    onSelectedItemChanged: {
        if (isWorkspace && workspaceViewModel && selectedItem) {
            console.log("Workspace changed, reloading projects for index:", selectedItem.index)
            var projects = workspaceViewModel.getWorkspaceProjects(selectedItem.index)
            console.log("Reloaded projects:", projects.length)
            workspaceProjects = projects
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Header avec bouton retour
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            // Bouton retour
            GlassButton {
                text: "← Back"
                accentColor: "#666666"
                width: 100
                onClicked: root.backClicked()
            }
            
            Item { width: 20 }
            
            // Icône et titre
            Rectangle {
                width: 40
                height: 40
                radius: 10
                color: isWorkspace ? 
                    Qt.rgba(0.5, 0.3, 0.8, 0.3) : 
                    Qt.rgba(0.3, 0.6, 1.0, 0.3)
                border.width: 1
                border.color: isWorkspace ? 
                    Qt.rgba(0.5, 0.3, 0.8, 0.5) : 
                    Qt.rgba(0.3, 0.6, 1.0, 0.5)
                
                Text {
                    anchors.centerIn: parent
                    text: isWorkspace ? "🗂️" : "📦"
                    font.pixelSize: 20
                }
            }
            
            ColumnLayout {
                spacing: 2
                
                Text {
                    text: selectedItem ? selectedItem.name : "Unknown"
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    color: "#ffffff"
                }
                
                Text {
                    text: isWorkspace ? 
                        (selectedItem && selectedItem.description ? selectedItem.description : "Workspace") :
                        (selectedItem && selectedItem.path ? selectedItem.path : "Project")
                    font.pixelSize: 11
                    color: "#888888"
                }
            }
            
            Item { Layout.fillWidth: true }
        }
        
        // Zone des projets avec leurs commandes
        GlassCard {
            Layout.fillWidth: true
            Layout.preferredHeight: isWorkspace ? 300 : 200
            glassOpacity: 0.1
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 12
                
                Text {
                    text: isWorkspace ? "Projects in Workspace" : "Available Scripts"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    color: "#ffffff"
                }
                
                // Si c'est un workspace avec plusieurs projets
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: isWorkspace
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12
                        
                        // Boutons pour sélectionner les projets
                        ScrollView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 60
                            
                            RowLayout {
                                spacing: 8
                                
                                Repeater {
                                    model: workspaceProjects.length
                                    
                                    GlassButton {
                                        required property int index
                                        
                                        text: workspaceProjects[index] ? workspaceProjects[index].name : "Project " + (index + 1)
                                        accentColor: currentProjectIndex === index ? "#7c4dff" : "#666666"
                                        width: 120
                                        
                                        onClicked: {
                                            currentProjectIndex = index
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Scripts du projet sélectionné
                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            contentWidth: availableWidth
                            
                            ColumnLayout {
                                width: parent.width
                                spacing: 0
                                
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 4
                                    columnSpacing: 12
                                    rowSpacing: 12
                                
                                    Repeater {
                                        model: workspaceProjects.length > 0 && workspaceProjects[currentProjectIndex] ? 
                                            workspaceProjects[currentProjectIndex].scripts : []
                                        
                                        delegate: GlassButton {
                                            required property var modelData
                                            required property int index
                                            
                                            property string cachedProcessId: ""
                                            property bool cachedIsRunning: false
                                            
                                            text: cachedIsRunning ? "⏹ Stop" : ("▶ " + (modelData.name || "Script"))
                                            accentColor: cachedIsRunning ? "#ef4444" : "#4a90e2"
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: 0
                                            Layout.preferredHeight: 50
                                            
                                            Component.onCompleted: {
                                                updateState()
                                            }
                                            
                                            // Watcher pour activeProcesses
                                            Connections {
                                                target: root
                                                function onActiveProcessesChanged() {
                                                    updateState()
                                                }
                                            }
                                            
                                            function updateState() {
                                                if (workspaceProjects[currentProjectIndex]) {
                                                    cachedProcessId = workspaceProjects[currentProjectIndex].name + "_" + modelData.name
                                                    cachedIsRunning = activeProcesses[cachedProcessId] !== undefined
                                                }
                                            }
                                            
                                            onClicked: {
                                                if (workspaceProjects[currentProjectIndex] && processManager) {
                                                    const project = workspaceProjects[currentProjectIndex]
                                                    const pid = project.name + "_" + modelData.name
                                                    
                                                    if (activeProcesses[pid]) {
                                                        console.log("Stopping process:", pid)
                                                        processManager.stopProcess(pid)
                                                        delete activeProcesses[pid]
                                                        // Force update
                                                        activeProcesses = Object.assign({}, activeProcesses)
                                                        updateState()
                                                    } else {
                                                        console.log("Starting process:", pid)
                                                        const success = processManager.runScript(
                                                            pid,
                                                            modelData.name,
                                                            project.path
                                                        )
                                                        
                                                        if (success) {
                                                            activeProcesses[pid] = {
                                                                projectIndex: currentProjectIndex,
                                                                scriptName: modelData.name
                                                            }
                                                            // Force update
                                                            activeProcesses = Object.assign({}, activeProcesses)
                                                            updateState()
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
                
                // Si c'est un projet simple
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: !isWorkspace
                    clip: true
                    contentWidth: availableWidth
                    
                    ColumnLayout {
                        width: parent.width
                        spacing: 0
                        
                        GridLayout {
                            Layout.fillWidth: true
                            columns: 4
                            columnSpacing: 12
                            rowSpacing: 12
                        
                            Repeater {
                                model: !isWorkspace && selectedItem && selectedItem.scripts ? selectedItem.scripts : []
                                
                                delegate: GlassButton {
                                    required property var modelData
                                    required property int index
                                    
                                    property string cachedProcessId: ""
                                    property bool cachedIsRunning: false
                                    
                                    text: cachedIsRunning ? "⏹ Stop" : ("▶ " + (modelData.name || "Script"))
                                    accentColor: cachedIsRunning ? "#ef4444" : "#4a90e2"
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredHeight: 50
                                    
                                    Component.onCompleted: {
                                        updateState()
                                    }
                                    
                                    Connections {
                                        target: root
                                        function onActiveProcessesChanged() {
                                            updateState()
                                        }
                                    }
                                    
                                    function updateState() {
                                        if (selectedItem) {
                                            cachedProcessId = selectedItem.name + "_" + modelData.name
                                            cachedIsRunning = activeProcesses[cachedProcessId] !== undefined
                                        }
                                    }
                                    
                                    onClicked: {
                                        if (selectedItem && processManager) {
                                            const pid = selectedItem.name + "_" + modelData.name
                                            
                                            if (activeProcesses[pid]) {
                                                console.log("Stopping process:", pid)
                                                processManager.stopProcess(pid)
                                                delete activeProcesses[pid]
                                                // Force update
                                                activeProcesses = Object.assign({}, activeProcesses)
                                                updateState()
                                            } else {
                                                console.log("Starting process:", pid)
                                                const success = processManager.runScript(
                                                    pid,
                                                    modelData.name,
                                                    selectedItem.path
                                                )
                                                
                                                if (success) {
                                                    activeProcesses[pid] = {
                                                        projectIndex: 0,
                                                        scriptName: modelData.name
                                                    }
                                                    // Force update
                                                    activeProcesses = Object.assign({}, activeProcesses)
                                                    updateState()
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
        
        // Zone des terminaux
        MultiTerminalView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            isWorkspace: root.isWorkspace
            projects: root.workspaceProjects
            currentProjectIndex: root.currentProjectIndex
            activeProcesses: root.activeProcesses
        }
    }
}
