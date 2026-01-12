import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./components"

// Vue unifiée affichant workspaces en haut puis projets
Item {
    id: root
    
    signal itemSelected(var item, bool isWorkspace)
    signal processMonitorRequested()
    
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
    
    // Folder dialog for adding a single project
    FolderDialog {
        id: addProjectFolderDialog
        title: "Select Project Folder"
        onAccepted: {
            projectManager.addProject(selectedFolder)
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
    
    // Advanced workspace creation dialog with project selection
    Dialog {
        id: workspaceCreationDialog
        title: "Create New Workspace"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 600
        height: 700
        
        property var selectedProjectIds: []
        
        onOpened: {
            selectedProjectIds = []
            workspaceNameField.text = ""
            workspaceDescField.text = ""
        }
        
        background: Rectangle {
            color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
            border.color: Qt.rgba(1, 1, 1, 0.1)
            border.width: 1
            radius: 12
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 16
            
            // Workspace name
            Column {
                Layout.fillWidth: true
                spacing: 8
                
                Text {
                    text: "Workspace Name"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: "#cccccc"
                }
                
                TextField {
                    id: workspaceNameField
                    width: parent.width
                    placeholderText: "e.g., Full Stack Development"
                    font.pixelSize: 14
                    
                    background: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0.3)
                        border.color: workspaceNameField.activeFocus ? "#4a90e2" : Qt.rgba(1, 1, 1, 0.1)
                        border.width: 1
                        radius: 6
                    }
                    
                    color: "#ffffff"
                }
            }
            
            // Workspace description
            Column {
                Layout.fillWidth: true
                spacing: 8
                
                Text {
                    text: "Description (Optional)"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: "#cccccc"
                }
                
                TextField {
                    id: workspaceDescField
                    width: parent.width
                    placeholderText: "Describe this workspace..."
                    font.pixelSize: 13
                    
                    background: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0.3)
                        border.color: workspaceDescField.activeFocus ? "#4a90e2" : Qt.rgba(1, 1, 1, 0.1)
                        border.width: 1
                        radius: 6
                    }
                    
                    color: "#ffffff"
                }
            }
            
            // Scan folder button
            GlassButton {
                text: "📁 Scan Folder for Projects"
                accentColor: "#7c4dff"
                Layout.fillWidth: true
                onClicked: folderDialog.open()
            }
            
            // Project selection list
            Column {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8
                
                Text {
                    text: "Select Projects (" + workspaceCreationDialog.selectedProjectIds.length + " selected)"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: "#cccccc"
                }
                
                ScrollView {
                    width: parent.width
                    height: parent.height - 30
                    clip: true
                    
                    ListView {
                        id: projectSelectionList
                        width: parent.width
                        spacing: 8
                        
                        model: projectManager
                        
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 60
                            color: checkBox.checked ? Qt.rgba(0.5, 0.3, 0.8, 0.2) : Qt.rgba(1, 1, 1, 0.05)
                            radius: 8
                            border.width: checkBox.checked ? 1 : 0
                            border.color: Qt.rgba(0.5, 0.3, 0.8, 0.5)
                            
                            required property int index
                            required property string name
                            required property string path
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                
                                CheckBox {
                                    id: checkBox
                                    checked: false
                                    
                                    onCheckedChanged: {
                                        var arr = workspaceCreationDialog.selectedProjectIds
                                        if (checked) {
                                            if (arr.indexOf(index) === -1) {
                                                arr.push(index)
                                            }
                                        } else {
                                            const idx = arr.indexOf(index)
                                            if (idx > -1) {
                                                arr.splice(idx, 1)
                                            }
                                        }
                                        workspaceCreationDialog.selectedProjectIds = arr
                                    }
                                }
                                
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    
                                    Text {
                                        text: name
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                        color: "#ffffff"
                                        elide: Text.ElideRight
                                        width: parent.width
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
                }
            }
            
            // Action buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Cancel"
                    onClicked: workspaceCreationDialog.close()
                }
                
                Button {
                    text: "Create Workspace"
                    highlighted: true
                    enabled: workspaceNameField.text.trim() !== ""
                    onClicked: {
                        if (workspaceViewModel) {
                            // Create workspace
                            var desc = workspaceDescField.text.trim() !== "" ? 
                                workspaceDescField.text : 
                                (workspaceCreationDialog.selectedProjectIds.length + " projects")
                            
                            var workspaceId = workspaceViewModel.createWorkspace(
                                workspaceNameField.text,
                                desc
                            )
                            
                            // Add selected projects to workspace
                            if (workspaceId && workspaceCreationDialog.selectedProjectIds.length > 0) {
                                console.log("Adding", workspaceCreationDialog.selectedProjectIds.length, "projects to workspace", workspaceId)
                                for (var i = 0; i < workspaceCreationDialog.selectedProjectIds.length; i++) {
                                    var projectIdx = workspaceCreationDialog.selectedProjectIds[i]
                                    var projectData = projectManager.getProject(projectIdx)
                                    if (projectData && projectData.id) {
                                        console.log("Adding project:", projectData.name, "ID:", projectData.id)
                                        workspaceViewModel.addScannedProjectToWorkspace(workspaceId, projectData)
                                    }
                                }
                            }
                            
                            workspaceCreationDialog.close()
                        }
                    }
                }
            }
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
            
            // Process Monitor button
            GlassButton {
                text: "⚙️ Processes"
                accentColor: "#ef4444"
                width: 140
                onClicked: root.processMonitorRequested()
            }
            
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
                onClicked: addProjectFolderDialog.open()
            }
            
            // Bouton Create Workspace
            GlassButton {
                text: "Create Workspace"
                accentColor: "#e91e63"
                width: 160
                onClicked: {
                    workspaceCreationDialog.open()
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
                                required property string workspaceId
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
                                            Layout.fillHeight: true
                                            spacing: 4
                                            
                                            Text {
                                                Layout.fillWidth: true
                                                text: name
                                                font.pixelSize: 14
                                                font.weight: Font.Bold
                                                color: "#ffffff"
                                                elide: Text.ElideRight
                                            }
                                            
                                            Text {
                                                Layout.fillWidth: true
                                                text: description || "No description"
                                                font.pixelSize: 10
                                                color: "#aaaaaa"
                                                elide: Text.ElideRight
                                                wrapMode: Text.Wrap
                                                maximumLineCount: 2
                                            }
                                            
                                            Item { Layout.fillHeight: true }
                                            
                                            // Badge nombre de projets
                                            Rectangle {
                                                Layout.preferredWidth: 60
                                                Layout.preferredHeight: 20
                                                radius: 10
                                                color: Qt.rgba(0.5, 0.3, 0.8, 0.3)
                                                
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: projectCount + " 📦"
                                                    font.pixelSize: 10
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
                                            console.log("Delete workspace clicked for:", name, "ID:", workspaceId)
                                            if (workspaceViewModel && workspaceId) {
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
                            cellHeight: 140
                            
                            model: projectManager
                            
                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }
                            
                            delegate: Item {
                                width: projectGrid.cellWidth - 16
                                height: projectGrid.cellHeight - 16
                                
                                required property int index
                                required property string name
                                required property string path
                                required property int scriptCount
                                required property var scripts
                                
                                GlassCard {
                                    anchors.fill: parent
                                    glassOpacity: projectMouseArea.containsMouse ? 0.2 : 0.1
                                    
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
                                        spacing: 4
                                        
                                        Text {
                                            Layout.fillWidth: true
                                            text: name
                                            font.pixelSize: 14
                                            font.weight: Font.Bold
                                            color: "#ffffff"
                                            elide: Text.ElideRight
                                        }
                                        
                                        Text {
                                            Layout.fillWidth: true
                                            text: path
                                            font.pixelSize: 10
                                            color: "#aaaaaa"
                                            elide: Text.ElideMiddle
                                            wrapMode: Text.Wrap
                                            maximumLineCount: 2
                                        }
                                        
                                        Item { Layout.fillHeight: true }
                                        
                                        // Badge nombre de scripts
                                        Rectangle {
                                            Layout.preferredWidth: 70
                                            Layout.preferredHeight: 22
                                            radius: 11
                                            color: Qt.rgba(0.3, 0.6, 1.0, 0.3)
                                            
                                            Text {
                                                anchors.centerIn: parent
                                                text: scripts ? scripts.length + " ⚡" : "0 ⚡"
                                                font.pixelSize: 10
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
