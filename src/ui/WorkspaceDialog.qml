import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "./components"

// Dialog for creating or editing a workspace
Dialog {
    id: workspaceDialog
    
    property bool isEditMode: false
    property string workspaceId: ""
    property alias workspaceName: nameField.text
    property alias workspaceDescription: descriptionField.text
    
    title: isEditMode ? "Edit Workspace" : "Create New Workspace"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    width: 500
    height: 300
    
    anchors.centerIn: parent
    
    // Custom background for glassmorphism
    background: Rectangle {
        color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1
        radius: 12
        
        // Subtle glow effect
        layer.enabled: true
        layer.effect: ShaderEffect {
            fragmentShader: "qrc:/ui/shaders/glow.frag"
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 20
        
        // Workspace name field
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
                id: nameField
                width: parent.width
                placeholderText: "e.g., Full Stack Development"
                font.pixelSize: 14
                
                background: Rectangle {
                    color: Qt.rgba(0, 0, 0, 0.3)
                    border.color: nameField.activeFocus ? "#4a90e2" : Qt.rgba(1, 1, 1, 0.1)
                    border.width: 1
                    radius: 6
                }
                
                color: "#ffffff"
                
                // Focus highlight
                Keys.onReturnPressed: {
                    if (nameField.text.trim() !== "") {
                        workspaceDialog.accept()
                    }
                }
            }
        }
        
        // Workspace description field
        Column {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            
            Text {
                text: "Description (Optional)"
                font.pixelSize: 13
                font.weight: Font.Medium
                color: "#cccccc"
            }
            
            ScrollView {
                width: parent.width
                height: 120
                clip: true
                
                TextArea {
                    id: descriptionField
                    placeholderText: "Describe this workspace and its purpose..."
                    font.pixelSize: 13
                    wrapMode: TextArea.Wrap
                    
                    background: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0.3)
                        border.color: descriptionField.activeFocus ? "#4a90e2" : Qt.rgba(1, 1, 1, 0.1)
                        border.width: 1
                        radius: 6
                    }
                    
                    color: "#ffffff"
                }
            }
        }
        
        // Info text
        Text {
            Layout.fillWidth: true
            text: isEditMode ? 
                "Update the workspace details above." :
                "Create a workspace to group related projects together."
            font.pixelSize: 12
            color: "#888888"
            wrapMode: Text.WordWrap
        }
    }
    
    // Custom button styling
    onAboutToShow: {
        // Style the OK button
        for (var i = 0; i < footer.count; i++) {
            var button = footer.itemAt(i)
            if (button && button.text === "OK") {
                button.background.color = Qt.binding(function() {
                    return button.down ? "#3a7bc2" : "#4a90e2"
                })
            }
        }
    }
    
    onAccepted: {
        // Validation
        if (nameField.text.trim() === "") {
            // Show error - name is required
            return
        }
        
        // The actual creation/update will be handled by the parent component
    }
    
    onRejected: {
        // Clear fields
        nameField.text = ""
        descriptionField.text = ""
    }
    
    // Function to reset dialog for new workspace
    function resetForNew() {
        isEditMode = false
        workspaceId = ""
        nameField.text = ""
        descriptionField.text = ""
        nameField.forceActiveFocus()
    }
    
    // Function to populate dialog for editing
    function loadWorkspace(id, name, description) {
        isEditMode = true
        workspaceId = id
        nameField.text = name
        descriptionField.text = description
        nameField.forceActiveFocus()
    }
}
