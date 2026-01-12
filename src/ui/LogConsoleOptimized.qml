import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Optimized terminal console using ListView for maximum performance
// This component is designed to handle large amounts of log output without freezing
Item {
    id: root
    
    property string projectName: "Console"
    property int projectIndex: 0
    
    // Function to clear logs
    function clearLogs() {
        logListModel.clear()
    }
    
    // Simple list model for logs - much faster than TextEdit with concatenation
    ListModel {
        id: logListModel
    }
    
    // Connect to process manager output
    Connections {
        target: processManager
        
        function onProcessOutput(id, output, isStderr) {
            // Check if this output is for a process associated with this project
            var activeProc = root.parent.parent.parent.parent.activeProcesses[id]
            if (activeProc && activeProc.projectIndex === projectIndex) {
                // Split output into lines
                var lines = output.split('\n')
                for (var i = 0; i < lines.length; i++) {
                    if (lines[i].length > 0) {
                        addLogLine(lines[i], isStderr)
                    }
                }
            }
        }
        
        function onProcessError(id, error) {
            var activeProc = root.parent.parent.parent.parent.activeProcesses[id]
            if (activeProc && activeProc.projectIndex === projectIndex) {
                addLogLine("❌ Error: " + error, true)
            }
        }
        
        function onProcessFinished(id, exitCode) {
            var activeProc = root.parent.parent.parent.parent.activeProcesses[id]
            if (activeProc && activeProc.projectIndex === projectIndex) {
                addLogLine("✅ Process finished with exit code: " + exitCode, false)
            }
        }
    }
    
    // Add a log line to the model
    function addLogLine(text, isError) {
        // Limit buffer to 2000 lines for better performance
        // This is more aggressive than the 5000 line circular buffer to ensure UI responsiveness
        if (logListModel.count >= 2000) {
            // Remove oldest 500 lines at once to reduce frequent operations
            for (var i = 0; i < 500; i++) {
                logListModel.remove(0)
            }
        }
        
        var timestamp = Qt.formatTime(new Date(), "hh:mm:ss.zzz")
        logListModel.append({
            "text": text,
            "timestamp": timestamp,
            "isError": isError
        })
        
        // Auto-scroll to bottom
        Qt.callLater(function() {
            if (logListView.count > 0) {
                logListView.positionViewAtEnd()
            }
        })
    }
    
    // Placeholder when no logs
    Text {
        anchors.centerIn: parent
        text: "Terminal ready. Run a script to see output..."
        font.pixelSize: 12
        font.family: "monospace"
        color: "#666666"
        visible: logListModel.count === 0
    }
    
    // Optimized ListView for logs
    ListView {
        id: logListView
        anchors.fill: parent
        clip: true
        model: logListModel
        
        // Performance optimizations
        cacheBuffer: 2000  // Cache more items for smoother scrolling
        asynchronous: true  // Load delegates asynchronously
        maximumFlickVelocity: 8000  // Faster scrolling
        pixelAligned: true  // Pixel-perfect rendering
        
        // Prevent flickering during rapid updates
        reuseItems: true
        
        // Use simple delegate for maximum performance
        delegate: Item {
            width: logListView.width
            height: logLine.height + 2
            
            Row {
                id: logLine
                spacing: 8
                width: parent.width
                
                // Timestamp
                Text {
                    text: model.timestamp
                    font.pixelSize: 9
                    font.family: "monospace"
                    color: "#555555"
                    anchors.verticalCenter: parent.verticalCenter
                    renderType: Text.NativeRendering
                }
                
                // Level indicator dot
                Rectangle {
                    width: 4
                    height: 4
                    radius: 2
                    anchors.verticalCenter: parent.verticalCenter
                    color: model.isError ? "#ef4444" : "#3b82f6"
                }
                
                // Log text - single Text element for best performance
                Text {
                    text: model.text
                    font.pixelSize: 10
                    font.family: "Menlo, Monaco, Courier, monospace"
                    color: model.isError ? "#f87171" : "#00ff00"
                    wrapMode: Text.NoWrap
                    renderType: Text.NativeRendering
                    elide: Text.ElideNone
                    
                    // Limit text width for performance
                    width: Math.min(implicitWidth, logListView.width - 100)
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
    
    // Line count indicator
    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 4
        width: lineCountText.width + 12
        height: 18
        radius: 9
        color: Qt.rgba(0, 0, 0, 0.7)
        visible: logListModel.count > 0
        
        Text {
            id: lineCountText
            anchors.centerIn: parent
            text: logListModel.count + " / 2000"
            font.pixelSize: 9
            font.family: "monospace"
            color: logListModel.count > 1800 ? "#f59e0b" : "#888888"
        }
    }
}
