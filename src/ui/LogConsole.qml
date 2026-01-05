import QtQuick
import QtQuick.Layouts
import "./components"

// Terminal-style log console with ANSI color support
GlassCard {
    id: logConsole
    
    property alias model: logView.model
    property string projectName: "Console"
    
    glassOpacity: 0.08
    cornerRadius: 12
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        
        // Console header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            Text {
                text: "📋 " + projectName + " Logs"
                font.pixelSize: 14
                font.weight: Font.DemiBold
                font.family: "monospace"
                color: "#ffffff"
            }
            
            Item { Layout.fillWidth: true }
            
            // Control buttons
            Row {
                spacing: 8
                
                Rectangle {
                    width: 24
                    height: 24
                    radius: 4
                    color: Qt.rgba(1, 1, 1, 0.1)
                    
                    Text {
                        anchors.centerIn: parent
                        text: "🔍"
                        font.pixelSize: 12
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            // Toggle search
                        }
                    }
                }
                
                Rectangle {
                    width: 24
                    height: 24
                    radius: 4
                    color: Qt.rgba(1, 1, 1, 0.1)
                    
                    Text {
                        anchors.centerIn: parent
                        text: "🗑️"
                        font.pixelSize: 11
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            // Clear logs
                        }
                    }
                }
            }
        }
        
        // Log view with terminal styling
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#0a0a0a"
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)
            
            ListView {
                id: logView
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                
                // Performance optimizations for 60 FPS
                // Increase cache buffer to reduce re-rendering
                cacheBuffer: 1000
                
                // Enable asynchronous delegate loading
                asynchronous: true
                
                // Optimize for fast scrolling
                maximumFlickVelocity: 5000
                
                // Use optimal pixel-aligned positioning
                pixelAligned: true
                
                // Scroll to bottom on new items
                onCountChanged: {
                    if (logView.count > 0) {
                        logView.positionViewAtEnd()
                    }
                }
                
                // Placeholder model for demonstration
                model: ListModel {
                    ListElement { 
                        text: "[ZenRunner] Process manager initialized"
                        level: "info"
                        timestamp: "00:00:00"
                    }
                    ListElement { 
                        text: "[System] Ready to receive commands"
                        level: "success"
                        timestamp: "00:00:01"
                    }
                }
                
                delegate: Item {
                    width: logView.width
                    height: logLineContainer.height + 4
                    
                    Row {
                        id: logLineContainer
                        spacing: 8
                        
                        // Timestamp
                        Text {
                            text: model.formattedTime || model.timestamp
                            font.pixelSize: 11
                            font.family: "monospace"
                            color: "#666666"
                            anchors.verticalCenter: parent.verticalCenter
                            renderType: Text.NativeRendering  // Better performance
                        }
                        
                        // Level indicator
                        Rectangle {
                            width: 6
                            height: 6
                            radius: 3
                            anchors.verticalCenter: parent.verticalCenter
                            color: {
                                if (model.isError) return "#ef4444"
                                return "#3b82f6"
                            }
                        }
                        
                        // Log text with ANSI color support
                        Item {
                            width: logView.width - 100
                            height: logTextFlow.height
                            anchors.verticalCenter: parent.verticalCenter
                            
                            // Use Flow to render styled segments inline
                            Flow {
                                id: logTextFlow
                                width: parent.width
                                
                                // Check if we have styled segments (ANSI codes)
                                Repeater {
                                    model: (typeof styledSegments !== 'undefined' && styledSegments) ? styledSegments : []
                                    
                                    delegate: Text {
                                        text: modelData.text
                                        font.pixelSize: 12
                                        font.family: "monospace"
                                        font.bold: modelData.bold || false
                                        font.italic: modelData.italic || false
                                        font.underline: modelData.underline || false
                                        color: modelData.fgColor || "#e5e7eb"
                                        wrapMode: Text.NoWrap
                                        renderType: Text.NativeRendering
                                        
                                        // Background color support (if not transparent)
                                        Rectangle {
                                            anchors.fill: parent
                                            color: modelData.bgColor || "transparent"
                                            z: -1
                                            visible: modelData.bgColor && modelData.bgColor !== "#000000" && modelData.bgColor !== "transparent"
                                        }
                                    }
                                }
                                
                                // Fallback: plain text if no styled segments
                                Text {
                                    text: model.plainText || model.text
                                    font.pixelSize: 12
                                    font.family: "monospace"
                                    color: model.isError ? "#f87171" : "#e5e7eb"
                                    wrapMode: Text.Wrap
                                    width: logTextFlow.width
                                    renderType: Text.NativeRendering
                                    visible: !model.styledSegments || model.styledSegments.length === 0
                                }
                            }
                        }
                    }
                }
                
                // Custom scrollbar
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    
                    contentItem: Rectangle {
                        implicitWidth: 6
                        radius: 3
                        color: Qt.rgba(1, 1, 1, 0.3)
                    }
                    
                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }
        
        // Status bar
        Rectangle {
            Layout.fillWidth: true
            height: 24
            radius: 6
            color: Qt.rgba(0, 0, 0, 0.3)
            
            Row {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 16
                
                Text {
                    text: "Lines: " + logView.count + " / 5000"
                    font.pixelSize: 10
                    font.family: "monospace"
                    color: "#888888"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Rectangle {
                    width: 1
                    height: 12
                    color: "#333333"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Text {
                    text: "Auto-scroll: ON"
                    font.pixelSize: 10
                    font.family: "monospace"
                    color: "#4ade80"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: "⚡ Circular buffer active"
                    font.pixelSize: 10
                    font.family: "monospace"
                    color: "#666666"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
