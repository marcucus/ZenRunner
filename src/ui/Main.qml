import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Effects

ApplicationWindow {
    id: mainWindow
    
    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: "ZenRunner"
    
    // Window background - dark theme for developer focus
    color: "#1a1a1a"
    
    // Window flags for native integration
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | 
           Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint | 
           Qt.WindowCloseButtonHint
    
    // Main container with glassmorphism effect
    Item {
        anchors.fill: parent
        
        // Background gradient
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1a1a2e" }
                GradientStop { position: 1.0; color: "#0f0f1e" }
            }
        }
        
        // Animated background particles for depth
        Repeater {
            model: 20
            
            Rectangle {
                id: particle
                width: Math.random() * 4 + 2
                height: width
                radius: width / 2
                color: Qt.rgba(0.3, 0.5, 0.8, Math.random() * 0.3 + 0.1)
                x: Math.random() * mainWindow.width
                y: Math.random() * mainWindow.height
                
                // Subtle float animation
                SequentialAnimation on y {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation {
                        from: particle.y
                        to: particle.y - 50
                        duration: 3000 + Math.random() * 2000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        from: particle.y - 50
                        to: particle.y
                        duration: 3000 + Math.random() * 2000
                        easing.type: Easing.InOutSine
                    }
                }
                
                SequentialAnimation on opacity {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0.8
                        duration: 2000 + Math.random() * 1000
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 0.2
                        duration: 2000 + Math.random() * 1000
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
        
        // Main content area with Dashboard
        Dashboard {
            id: dashboard
            anchors.fill: parent
            anchors.margins: 20
        }
    }
    
    // Memory optimization: Use ShaderEffectSource for background blur only when needed
    property bool useBlur: false
}
