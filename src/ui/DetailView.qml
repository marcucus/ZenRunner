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
                            
                            GridLayout {
                                width: parent.width
                                columns: 3
                                rowSpacing: 8
                                columnSpacing: 8
                                
                                Repeater {
                                    model: workspaceProjects.length > 0 && workspaceProjects[currentProjectIndex] ? 
                                        workspaceProjects[currentProjectIndex].scripts : []
                                    
                                    GlassButton {
                                        required property var modelData
                                        
                                        text: modelData.name || "Script"
                                        accentColor: "#4a90e2"
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 150
                                        
                                        onClicked: {
                                            if (workspaceProjects[currentProjectIndex] && processManager) {
                                                const projectPath = workspaceProjects[currentProjectIndex].path
                                                const processId = processManager.startScript(
                                                    projectPath,
                                                    modelData.name,
                                                    modelData.command
                                                )
                                                activeProcesses[processId] = {
                                                    projectIndex: currentProjectIndex,
                                                    scriptName: modelData.name
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
                    
                    GridLayout {
                        width: parent.width
                        columns: 3
                        rowSpacing: 8
                        columnSpacing: 8
                        
                        Repeater {
                            model: !isWorkspace && selectedItem && selectedItem.scripts ? selectedItem.scripts : []
                            
                            GlassButton {
                                required property var modelData
                                
                                text: modelData.name || "Script"
                                accentColor: "#4a90e2"
                                Layout.fillWidth: true
                                Layout.minimumWidth: 150
                                
                                onClicked: {
                                    if (selectedItem && processManager) {
                                        const processId = processManager.startScript(
                                            selectedItem.path,
                                            modelData.name,
                                            modelData.command
                                        )
                                        activeProcesses[processId] = {
                                            projectIndex: 0,
                                            scriptName: modelData.name
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
