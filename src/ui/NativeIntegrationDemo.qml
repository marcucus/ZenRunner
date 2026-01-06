import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import "./components"

/**
 * Example QML file demonstrating QuickControlBar usage
 * with Dynamic Island-like positioning for macOS notch
 */
Window {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: "ZenRunner - Native Integration Demo"
    
    // Make window background transparent for native effects
    color: "transparent"
    
    // Background
    Rectangle {
        anchors.fill: parent
        color: "#1E1E1E"
        
        // Main content area
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "ZenRunner - Native OS Integration"
                color: "white"
                font.pixelSize: 24
                font.weight: Font.Bold
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: "Platform: " + platformHelper.platformName
                color: "#CCCCCC"
                font.pixelSize: 16
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: platformHelper.hasNotch ? 
                      "✓ Notch detected (Safe area top: " + platformHelper.notchSafeAreaTop + "px)" :
                      "No notch detected"
                color: platformHelper.hasNotch ? "#00FF00" : "#CCCCCC"
                font.pixelSize: 14
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            // Workspace status (example)
            Column {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                
                Repeater {
                    model: workspaceModel
                    
                    Rectangle {
                        width: 300
                        height: 60
                        radius: 8
                        color: "#2A2A2A"
                        border.color: "#3A3A3A"
                        border.width: 1
                        
                        Row {
                            anchors.centerIn: parent
                            spacing: 15
                            
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: modelData.isRunning ? "#00FF00" : "#808080"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            
                            Text {
                                text: modelData.name
                                color: "white"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Quick Control Bar - positioned at top center with notch awareness
    QuickControlBar {
        id: quickControlBar
        parent: mainWindow.contentItem
        
        // Pass notch safe area from platform helper
        notchSafeAreaTop: platformHelper.notchSafeAreaTop
        
        // Pass workspace data
        workspaces: workspaceModel
        
        // Handle signals
        onWorkspaceStartRequested: function(workspaceName) {
            console.log("Starting workspace:", workspaceName)
            // Call C++ backend to start workspace
            // projectManager.startWorkspace(workspaceName)
            
            // Update tray state
            platformManager.setTrayState(1) // Active
            platformManager.showNotification("Workspace Started", workspaceName + " is now running")
        }
        
        onWorkspaceStopRequested: function(workspaceName) {
            console.log("Stopping workspace:", workspaceName)
            // Call C++ backend to stop workspace
            // projectManager.stopWorkspace(workspaceName)
            
            // Update tray state
            platformManager.setTrayState(0) // Idle
        }
        
        onShowMainWindow: {
            mainWindow.show()
            mainWindow.raise()
            mainWindow.requestActivate()
        }
    }
    
    // Example workspace model
    ListModel {
        id: workspaceModel
        
        ListElement {
            name: "Frontend Dev"
            isRunning: true
            hasError: false
        }
        
        ListElement {
            name: "Backend API"
            isRunning: true
            hasError: false
        }
        
        ListElement {
            name: "Database"
            isRunning: false
            hasError: false
        }
        
        ListElement {
            name: "Build Tools"
            isRunning: false
            hasError: false
        }
    }
    
    // Minimize to tray instead of closing
    onClosing: function(close) {
        close.accepted = false
        mainWindow.hide()
        
        // Show notification
        platformManager.showNotification(
            "ZenRunner Minimized",
            "Application is still running in the system tray"
        )
    }
    
    Component.onCompleted: {
        console.log("Native integration demo loaded")
        console.log("Platform:", platformHelper.platformName)
        console.log("Has notch:", platformHelper.hasNotch)
        
        if (platformHelper.hasNotch) {
            console.log("Notch safe area top:", platformHelper.notchSafeAreaTop)
        }
        
        // Show initial notification
        platformManager.showNotification(
            "ZenRunner Started",
            "Native process manager is ready"
        )
    }
}
