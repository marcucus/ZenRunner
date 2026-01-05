import QtQuick
import QtQuick.Controls

// Glassmorphism-styled button component
Button {
    id: root
    
    property color glassColor: "#ffffff"
    property real glassOpacity: 0.15
    property color textColor: "#ffffff"
    property color hoverColor: "#ffffff"
    property real hoverOpacity: 0.25
    property real pressedOpacity: 0.35
    property color accentColor: "#4a90e2"
    
    implicitWidth: 120
    implicitHeight: 40
    
    background: Rectangle {
        radius: 8
        color: {
            if (root.pressed)
                return Qt.rgba(glassColor.r, glassColor.g, glassColor.b, pressedOpacity)
            else if (root.hovered)
                return Qt.rgba(hoverColor.r, hoverColor.g, hoverColor.b, hoverOpacity)
            else
                return Qt.rgba(glassColor.r, glassColor.g, glassColor.b, glassOpacity)
        }
        
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.2)
        
        // Accent indicator when enabled
        Rectangle {
            visible: root.enabled
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 3
            radius: 1.5
            color: accentColor
            opacity: root.hovered ? 1.0 : 0.6
            
            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
        
        Behavior on color {
            ColorAnimation { duration: 150; easing.type: Easing.OutQuad }
        }
    }
    
    contentItem: Text {
        text: root.text
        font: root.font
        color: root.enabled ? textColor : Qt.rgba(textColor.r, textColor.g, textColor.b, 0.5)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    
    // Smooth hover transitions
    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }
}
