import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

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
    
    // Performance optimizations for 60 FPS target
    // GPU acceleration is configured globally in main.cpp via RHI
    // This QML file focuses on efficient structure and animations
    
    // View state management
    property bool showDetailView: false
    property var selectedItem: null
    property bool isWorkspace: false
    
    // Main container with glassmorphism effect
    Item {
        anchors.fill: parent
        
        // Background gradient - use layer for GPU compositing
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1a1a2e" }
                GradientStop { position: 1.0; color: "#0f0f1e" }
            }
        }
        
        // Optimized animated background particles for depth
        // Reduced count from 20 to 10 for better performance
        Repeater {
            model: 10
            
            Rectangle {
                id: particle
                width: Math.random() * 4 + 2
                height: width
                radius: width / 2
                color: Qt.rgba(0.3, 0.5, 0.8, Math.random() * 0.3 + 0.1)
                x: Math.random() * mainWindow.width
                y: Math.random() * mainWindow.height
                
                // Optimized float animation with longer duration to reduce GPU load
                SequentialAnimation on y {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation {
                        from: particle.y
                        to: particle.y - 50
                        duration: 4000 + Math.random() * 3000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        from: particle.y - 50
                        to: particle.y
                        duration: 4000 + Math.random() * 3000
                        easing.type: Easing.InOutSine
                    }
                }
                
                SequentialAnimation on opacity {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0.8
                        duration: 3000 + Math.random() * 2000
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 0.2
                        duration: 3000 + Math.random() * 2000
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
        
        // Main content area with new layout
        StackView {
            id: stackView
            anchors.fill: parent
            anchors.margins: 20
            
            initialItem: mainView
            
            // Main view - unified list + statistics
            Component {
                id: mainView
                
                UnifiedListView {
                    onItemSelected: (item, workspace) => {
                        mainWindow.selectedItem = item
                        mainWindow.isWorkspace = workspace
                        stackView.push(detailView)
                    }
                }
            }
            
            // Detail view - project/workspace details with terminals
            Component {
                id: detailView
                
                DetailView {
                    selectedItem: mainWindow.selectedItem
                    isWorkspace: mainWindow.isWorkspace
                    
                    onBackClicked: {
                        stackView.pop()
                    }
                }
            }
        }
    }
}
