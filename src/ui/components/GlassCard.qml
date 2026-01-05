import QtQuick

// Reusable glass card component with glassmorphism design
Item {
    id: root
    
    property real glassOpacity: 0.15
    property real borderOpacity: 0.3
    property color glassColor: "#ffffff"
    property color borderColor: "#ffffff"
    property real cornerRadius: 16
    
    // Content container - child items go here
    default property alias content: contentItem.children
    
    // Background with glass effect
    Rectangle {
        id: glassBackground
        anchors.fill: parent
        radius: root.cornerRadius
        color: Qt.rgba(glassColor.r, glassColor.g, glassColor.b, glassOpacity)
        
        // Border with gradient for depth
        border.width: 1
        border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, borderOpacity)
        
        // Inner highlight for glassmorphism effect
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: root.cornerRadius - 1
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)
        }
        
        // Subtle gradient overlay for depth
        Rectangle {
            anchors.fill: parent
            radius: root.cornerRadius
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.05) }
                GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.05) }
            }
        }
    }
    
    // Content layer
    Item {
        id: contentItem
        anchors.fill: parent
        anchors.margins: 16
        clip: true
    }
    
    // Hover effect for interactivity
    property bool hovered: false
    
    Behavior on glassOpacity {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }
    
    Behavior on scale {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }
    
    // Mouse area for hover detection (optional, can be overridden)
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        
        onEntered: root.hovered = true
        onExited: root.hovered = false
    }
}
