import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * QuickControlBar - Dynamic Island-like quick control menu
 * 
 * Positions itself at the top center of the screen, with automatic
 * adjustment for macOS notch safe area. Provides quick access to
 * workspace controls and application actions.
 */
Item {
    id: root
    
    // Public properties
    property int notchSafeAreaTop: 0  // Set from C++ based on safe area insets
    property bool expanded: false
    property var workspaces: []  // Array of workspace objects
    
    // Signals
    signal workspaceStartRequested(string workspaceName)
    signal workspaceStopRequested(string workspaceName)
    signal showMainWindow()
    
    // Positioning
    anchors.top: parent.top
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.topMargin: notchSafeAreaTop > 0 ? notchSafeAreaTop + 8 : 8
    
    width: expanded ? 360 : 200
    height: expanded ? contentLayout.implicitHeight + 16 : 40
    
    // Visual properties
    opacity: hoverArea.containsMouse || expanded ? 1.0 : 0.7
    
    Behavior on width {
        NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
    }
    
    Behavior on height {
        NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
    }
    
    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }
    
    // Background with glassmorphism effect
    Rectangle {
        id: background
        anchors.fill: parent
        radius: height / 2
        color: "#80000000"  // Semi-transparent black
        border.color: "#40FFFFFF"
        border.width: 1
        
        // Subtle shadow
        layer.enabled: true
        layer.effect: DropShadow {
            radius: 8
            samples: 17
            color: "#40000000"
            horizontalOffset: 0
            verticalOffset: 2
        }
    }
    
    // Content
    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8
        
        // Collapsed view - Status indicator
        RowLayout {
            Layout.fillWidth: true
            visible: !expanded
            spacing: 12
            
            // Status icon
            Rectangle {
                width: 24
                height: 24
                radius: 12
                color: getStatusColor()
                
                Behavior on color {
                    ColorAnimation { duration: 300 }
                }
            }
            
            // App name
            Text {
                text: "ZenRunner"
                color: "white"
                font.pixelSize: 14
                font.weight: Font.Medium
                Layout.fillWidth: true
            }
            
            // Expand indicator
            Text {
                text: "⋯"
                color: "#CCFFFFFF"
                font.pixelSize: 16
                rotation: 90
            }
        }
        
        // Expanded view - Full controls
        ColumnLayout {
            Layout.fillWidth: true
            visible: expanded
            spacing: 8
            
            // Header
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Quick Controls"
                    color: "white"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    Layout.fillWidth: true
                }
                
                // Close button
                Rectangle {
                    width: 20
                    height: 20
                    radius: 10
                    color: closeHover.containsMouse ? "#40FFFFFF" : "transparent"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: "white"
                        font.pixelSize: 16
                    }
                    
                    MouseArea {
                        id: closeHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.expanded = false
                    }
                }
            }
            
            // Workspaces list
            Repeater {
                model: workspaces
                
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    radius: 8
                    color: workspaceHover.containsMouse ? "#40FFFFFF" : "#20FFFFFF"
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8
                        
                        // Workspace status indicator
                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: modelData.isRunning ? "#00FF00" : "#808080"
                        }
                        
                        // Workspace name
                        Text {
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        
                        // Action button
                        Rectangle {
                            width: 24
                            height: 24
                            radius: 12
                            color: actionHover.containsMouse ? "#60FFFFFF" : "#40FFFFFF"
                            
                            Text {
                                anchors.centerIn: parent
                                text: modelData.isRunning ? "■" : "▶"
                                color: "white"
                                font.pixelSize: 10
                            }
                            
                            MouseArea {
                                id: actionHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (modelData.isRunning) {
                                        root.workspaceStopRequested(modelData.name)
                                    } else {
                                        root.workspaceStartRequested(modelData.name)
                                    }
                                }
                            }
                        }
                    }
                    
                    MouseArea {
                        id: workspaceHover
                        anchors.fill: parent
                        hoverEnabled: true
                        propagateComposedEvents: true
                        z: -1
                    }
                }
            }
            
            // Divider
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#40FFFFFF"
            }
            
            // Show main window button
            Rectangle {
                Layout.fillWidth: true
                height: 32
                radius: 8
                color: showHover.containsMouse ? "#40FFFFFF" : "#20FFFFFF"
                
                Text {
                    anchors.centerIn: parent
                    text: "Show Main Window"
                    color: "white"
                    font.pixelSize: 12
                }
                
                MouseArea {
                    id: showHover
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.showMainWindow()
                        root.expanded = false
                    }
                }
            }
        }
    }
    
    // Hover area for expand/collapse
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            root.expanded = !root.expanded
        }
    }
    
    // Helper function to get status color
    function getStatusColor() {
        if (!workspaces || workspaces.length === 0) {
            return "#808080"  // Gray - Idle
        }
        
        var hasError = false
        var hasRunning = false
        
        for (var i = 0; i < workspaces.length; i++) {
            if (workspaces[i].hasError) {
                hasError = true
            }
            if (workspaces[i].isRunning) {
                hasRunning = true
            }
        }
        
        if (hasError) {
            return "#FF0000"  // Red - Error
        } else if (hasRunning) {
            return "#00FF00"  // Green - Active
        } else {
            return "#808080"  // Gray - Idle
        }
    }
}
