import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./components"

// Vue unifiée affichant workspaces en haut puis projets
Item {
    id: root
    
    signal itemSelected(var item, bool isWorkspace)
    
    // Workspace creation dialog
    WorkspaceDialog {
        id: workspaceDialog
        parent: Overlay.overlay
        
        onAccepted: {
            if (workspaceDialog.isEditMode) {
                if (workspaceViewModel) {
                    workspaceViewModel.updateWorkspace(
                        workspaceDialog.workspaceId,
                        workspaceDialog.workspaceName,
                        workspaceDialog.workspaceDescription
                    )
                }
            } else {
                if (workspaceViewModel) {
                    workspaceViewModel.createWorkspace(
                        workspaceDialog.workspaceName,
                        workspaceDialog.workspaceDescription
                    )
                }
            }
        }
    }
    
    // Project addition dialog
    Dialog {
        id: addProjectDialog
        title: "Add Project"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 500
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 16
            
            Text {
                text: "Project Path"
                font.pixelSize: 14
                color: "#ffffff"
            }
            
            TextField {
                id: projectPathField
                Layout.fillWidth: true
                placeholderText: "Enter project path..."
                font.pixelSize: 14
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Cancel"
                    onClicked: addProjectDialog.close()
                }
                
                Button {
                    text: "Add"
                    highlighted: true
                    onClicked: {
                        if (projectPathField.text.length > 0) {
                            projectManager.addProject(projectPathField.text)
                            addProjectDialog.close()
                        }
                    }
                }
            }
        }
    }
    
    // Folder dialog for scanning
    FolderDialog {
        id: folderDialog
        title: "Select Project Folder to Scan"
        onAccepted: {
            projectManager.scanFolder(selectedFolder, 3)
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Header avec boutons d'action
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            Text {
                text: "🚀 ZenRunner"
                font.pixelSize: 24
                font.weight: Font.Bold
                color: "#ffffff"
            }
            
            Item { Layout.fillWidth: true }
            
            // Bouton Scan Folder
            GlassButton {
                text: "📁 Scan Folder"
                accentColor: "#4a90e2"
                width: 140
                onClicked: folderDialog.open()
            }
            
            // Bouton Add Project
            GlassButton {
                text: "+ Add Project"
                accentColor: "#7c4dff"
                width: 130
                onClicked: addProjectDialog.open()
            }
            
            // Bouton Create Workspace
            GlassButton {
                text: "Create Workspace"
                accentColor: "#e91e63"
                width: 160
                onClicked: {
                    workspaceDialog.resetForNew()
                    workspaceDialog.open()
                }
            }
        }
        
        // Liste unifiée (workspaces + projets)
        GlassCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.65
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // Section workspaces
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8
                        
                        // Titre workspaces
                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: Qt.rgba(0.5, 0.3, 0.8, 0.1)
                            radius: 8
                            
                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 16
                                anchors.verticalCenter: parent.verticalCenter
                                text: "🗂️ Workspaces"
                                font.pixelSize: 16
                                font.weight: Font.Bold
                                color: "#b794f6"
                            }
                        }
                        
                        // Grille des workspaces
                        GridView {
                            id: workspaceGrid
                            Layout.fillWidth: true
                            Layout.preferredHeight: contentHeight
                            Layout.maximumHeight: parent.height * 0.4
                            clip: true
                            interactive: contentHeight > height
                            
                            cellWidth: Math.floor(width / 2)
                            cellHeight: 90
                            
                            model: workspaceViewModel
                            
                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }
                            
                            delegate: Item {
                                width: workspaceGrid.cellWidth - 8
                                height: workspaceGrid.cellHeight - 8
                                
                                required property int index
                                required property string name
                                required property string description
                                required property int projectCount
                                
                                GlassCard {
                                    anchors.fill: parent
                                    glassOpacity: workspaceMouseArea.containsMouse ? 0.2 : 0.1
                                    
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 8
                                    
                                        // Icône workspace
                                        Rectangle {
                                            Layout.preferredWidth: 38
                                            Layout.preferredHeight: 38
                                            Layout.alignment: Qt.AlignVCenter
                                            radius: 10
                                            color: Qt.rgba(0.5, 0.3, 0.8, 0.3)
                                            border.width: 1
                                            border.color: Qt.rgba(0.5, 0.3, 0.8, 0.5)
                                            
                                            Text {
                                                anchors.centerIn: parent
                                                text: "🗂️"
                                                font.pixelSize: 18
                                            }
                                        }
                                        
                                        // Info workspace
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            
                                            Text {
                                                Layout.fillWidth: true
                                                text: name
                                                font.pixelSize: 13
                                                font.weight: Font.Bold
                                                color: "#ffffff"
                                                elide: Text.ElideRight
                                                maximumLineCount: 1
                                            }
                                            
                                            Text {
                                                Layout.fillWidth: true
                                                text: description || "No description"
                                                font.pixelSize: 9
                                                color: "#888888"
                                                elide: Text.ElideRight
                                                maximumLineCount: 2
                                                wrapMode: Text.WordWrap
                                            }
                                            
                                            Item { Layout.fillHeight: true }
                                            
                                            // Badge nombre de projets
                                            Rectangle {
                                                Layout.preferredWidth: 50
                                                Layout.preferredHeight: 18
                                                radius: 9
                                                color: Qt.rgba(0.5, 0.3, 0.8, 0.3)
                                                
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: projectCount + " 📦"
                                                    font.pixelSize: 9
                                                    font.weight: Font.Medium
                                                    color: "#b794f6"
                                                }
                                            }
                                        }
                                    }
                                    
                                    MouseArea {
                                        id: workspaceMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        
                                        onClicked: {
                                            console.log("Opening workspace:", name)
                                            root.itemSelected({
                                                index: index,
                                                name: name,
                                                description: description,
                                                projectCount: projectCount
                                            }, true)
                                        }
                                    }
                                }
                                
                                // Bouton de suppression - positionné en absolu au-dessus
                                Rectangle {
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 6
                                    width: 28
                                    height: 28
                                    radius: 14
                                    color: deleteMouseArea.containsMouse ? Qt.rgba(0.9, 0.2, 0.2, 0.7) : Qt.rgba(0.6, 0.2, 0.2, 0.4)
                                    border.width: 1
                                    border.color: deleteMouseArea.containsMouse ? "#ff4444" : "#cc3333"
                                    z: 100
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: "🗑️"
                                        font.pixelSize: 12
                                    }
                                    
                                    MouseArea {
                                        id: deleteMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        
                                        onClicked: {
                                            console.log("Delete workspace clicked for:", name)
                                            if (workspaceViewModel) {
                                                var workspaceId = workspaceViewModel.data(workspaceViewModel.index(index, 0), 256)
                                                console.log("Deleting workspace ID:", workspaceId)
                                                workspaceViewModel.deleteWorkspace(workspaceId)
                                            }
                                        }
                                    }
                                }
                            } // fin delegate
                        } // fin GridView
                        
                        // Titre projets
                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: Qt.rgba(0.3, 0.6, 1.0, 0.1)
                            radius: 8
                            Layout.topMargin: 16
                            
                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 16
                                anchors.verticalCenter: parent.verticalCenter
                                text: "📦 Projects"
                                font.pixelSize: 16
                                font.weight: Font.Bold
                                color: "#4a90e2"
                            }
                        }
                        
                        // Grille des projets
                        GridView {
                            id: projectGrid
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            
                            cellWidth: Math.floor(width / 2)
                            cellHeight: 100
                            
                            model: projectManager
                            
                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }
                            
                            delegate: Item {
                                width: workspaceGrid.cellWidth - 8
                                height: workspaceGrid.cellHeight - 8
                                
                                required property int index
                                required property string name
                                required property string description
                                required property int projectCount
                                
                                GlassCard {
                                    anchors.fill: parent
                                    glassOpacity: workspaceMouseArea.containsMouse ? 0.2 : 0.1
                                    
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 8
                                    
                                    // Icône projet
                                    Rectangle {
                                        Layout.preferredWidth: 40
                                        Layout.preferredHeight: 40
                                        Layout.alignment: Qt.AlignVCenter
                                        radius: 10
                                        color: Qt.rgba(0.3, 0.6, 1.0, 0.3)
                                        border.width: 1
                                        border.color: Qt.rgba(0.3, 0.6, 1.0, 0.5)
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "📦"
                                            font.pixelSize: 20
                                        }
                                    }
                                    
                                    // Info projet
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        spacing: 2
                                        
                                        Text {
                                            Layout.fillWidth: true
                                            text: name
                                            font.pixelSize: 13
                                            font.weight: Font.Bold
                                            color: "#ffffff"
                                            elide: Text.ElideRight
                                            maximumLineCount: 1
                                        }
                                        
                                        Text {
                                            Layout.fillWidth: true
                                            text: path
                                            font.pixelSize: 9
                                            color: "#888888"
                                            elide: Text.ElideMiddle
                                            maximumLineCount: 2
                                            wrapMode: Text.WordWrap
                                        }
                                        
                                        // Badge nombre de scripts
                                        Rectangle {
                                            Layout.preferredWidth: 55
                                            Layout.preferredHeight: 20
                                            radius: 10
                                            color: Qt.rgba(0.3, 0.6, 1.0, 0.3)
                                            
                                            Text {
                                                anchors.centerIn: parent
                                                text: scripts ? scripts.length + " ⚡" : "0 ⚡"
                                                font.pixelSize: 9
                                                font.weight: Font.Medium
                                                color: "#4a90e2"
                                            }
                                        }
                                    }
                                    
                                    // Badge nombre de scripts
                                    Rectangle {
                                        Layout.preferredWidth: 60
                                        Layout.minimumWidth: 60
                                        height: 24
                                        radius: 12
                                        color: Qt.rgba(0.3, 0.6, 1.0, 0.3)
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: scripts ? scripts.length + " ⚡" : "0 ⚡"
                                            font.pixelSize: 11
                                            font.weight: Font.Medium
                                            color: "#4a90e2"
                                        }
                                    }
                                }
                                
                                MouseArea {
                                    id: projectMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    
                                    onClicked: {
                                        root.itemSelected({
                                            index: index,
                                            name: name,
                                            path: path,
                                            scripts: scripts
                                        }, false)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Panneau de statistiques compact
        StatisticsPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.3
            Layout.minimumHeight: 180
        }
    }
}
}
