import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "./components"

// Process Monitor page - displays all processes with kill buttons
Item {
    id: root
    
    signal backClicked()
    
    // Process state constants (matching ProcessState enum in C++)
    readonly property int stateNotStarted: 0
    readonly property int stateStarting: 1
    readonly property int stateRunning: 2
    readonly property int statePaused: 3
    readonly property int stateStopping: 4
    readonly property int stateStopped: 5
    readonly property int stateCrashed: 6
    readonly property int stateFinished: 7
    
    // Timer to auto-refresh process list
    Timer {
        id: refreshTimer
        interval: 1000 // Refresh every second
        running: true
        repeat: true
        onTriggered: {
            refreshProcessList()
        }
    }
    
    // Process list model
    ListModel {
        id: processListModel
    }
    
    Component.onCompleted: {
        refreshProcessList()
    }
    
    // Connections to update on process state changes
    Connections {
        target: processManager
        
        function onProcessStateChanged(id, newState) {
            refreshProcessList()
        }
        
        function onProcessFinished(id, exitCode) {
            refreshProcessList()
        }
    }
    
    function refreshProcessList() {
        if (!processManager) return
        
        processListModel.clear()
        
        var processes = processManager.getAllProcessesInfo()
        for (var i = 0; i < processes.length; i++) {
            processListModel.append(processes[i])
        }
    }
    
    function getStateColor(state) {
        switch(state) {
            case root.stateRunning: // Running
                return "#4ade80" // Green
            case root.stateStarting: // Starting
                return "#60a5fa" // Blue
            case root.statePaused: // Paused
                return "#fbbf24" // Yellow
            case root.stateStopping: // Stopping
                return "#fb923c" // Orange
            case root.stateStopped: // Stopped
                return "#94a3b8" // Gray
            case root.stateCrashed: // Crashed
                return "#f87171" // Red
            case root.stateFinished: // Finished
                return "#a78bfa" // Purple
            default: // NotStarted
                return "#71717a" // Gray
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            // Back button
            GlassButton {
                text: "← Back"
                accentColor: "#666666"
                width: 100
                onClicked: root.backClicked()
            }
            
            Item { width: 20 }
            
            // Title
            Rectangle {
                width: 40
                height: 40
                radius: 10
                color: Qt.rgba(0.8, 0.3, 0.3, 0.3)
                border.width: 1
                border.color: Qt.rgba(0.8, 0.3, 0.3, 0.5)
                
                Text {
                    anchors.centerIn: parent
                    text: "⚙️"
                    font.pixelSize: 20
                }
            }
            
            ColumnLayout {
                spacing: 2
                
                Text {
                    text: "Process Monitor"
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    color: "#ffffff"
                }
                
                Text {
                    text: processListModel.count + " process(es) registered"
                    font.pixelSize: 13
                    color: "#888888"
                }
            }
            
            Item { Layout.fillWidth: true }
            
            // Running count indicator
            Rectangle {
                width: countText.width + 24
                height: 32
                radius: 16
                color: Qt.rgba(0.3, 0.8, 0.4, 0.2)
                border.width: 1
                border.color: Qt.rgba(0.3, 0.8, 0.4, 0.3)
                
                Text {
                    id: countText
                    anchors.centerIn: parent
                    text: processManager ? processManager.getRunningCount() + " running" : "0 running"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: "#4ade80"
                }
            }
            
            // Stop All button
            GlassButton {
                text: "Stop All"
                accentColor: "#ef4444"
                width: 120
                enabled: processManager && processManager.getRunningCount() > 0
                onClicked: {
                    if (processManager) {
                        processManager.stopAllProcesses(5000)
                        toastMessage.show("Stopping all processes...")
                    }
                }
            }
            
            // Kill All button
            GlassButton {
                text: "Kill All"
                accentColor: "#dc2626"
                width: 120
                enabled: processManager && processManager.getRunningCount() > 0
                onClicked: {
                    // Show confirmation before killing all
                    killAllDialog.open()
                }
            }
        }
        
        // Process list
        GlassCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 0
                
                // Column headers
                Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    color: Qt.rgba(1, 1, 1, 0.05)
                    radius: 6
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 12
                        
                        Text {
                            Layout.preferredWidth: 120
                            text: "Status"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            color: "#999999"
                        }
                        
                        Text {
                            Layout.preferredWidth: 200
                            text: "Process ID"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            color: "#999999"
                        }
                        
                        Text {
                            Layout.preferredWidth: 80
                            text: "PID"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            color: "#999999"
                        }
                        
                        Text {
                            Layout.fillWidth: true
                            text: "Command / Working Directory"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            color: "#999999"
                        }
                        
                        Text {
                            Layout.preferredWidth: 200
                            text: "Actions"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            color: "#999999"
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
                
                // Process list scroll area
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    
                    ListView {
                        id: processListView
                        model: processListModel
                        spacing: 8
                        
                        delegate: Rectangle {
                            width: processListView.width
                            height: 80
                            color: Qt.rgba(1, 1, 1, 0.03)
                            radius: 8
                            border.width: 1
                            border.color: Qt.rgba(1, 1, 1, 0.08)
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                
                                // Status indicator
                                Rectangle {
                                    Layout.preferredWidth: 120
                                    height: 32
                                    radius: 16
                                    color: Qt.rgba(0, 0, 0, 0.3)
                                    border.width: 1
                                    border.color: getStateColor(model.state)
                                    
                                    RowLayout {
                                        anchors.centerIn: parent
                                        spacing: 6
                                        
                                        Rectangle {
                                            width: 8
                                            height: 8
                                            radius: 4
                                            color: getStateColor(model.state)
                                        }
                                        
                                        Text {
                                            text: model.stateString || "Unknown"
                                            font.pixelSize: 11
                                            font.weight: Font.Medium
                                            color: getStateColor(model.state)
                                        }
                                    }
                                }
                                
                                // Process ID
                                Text {
                                    Layout.preferredWidth: 200
                                    text: model.id || "N/A"
                                    font.pixelSize: 13
                                    font.weight: Font.Medium
                                    color: "#ffffff"
                                    elide: Text.ElideRight
                                }
                                
                                // PID
                                Text {
                                    Layout.preferredWidth: 80
                                    text: model.pid > 0 ? model.pid.toString() : "-"
                                    font.pixelSize: 13
                                    color: "#cccccc"
                                }
                                
                                // Command and working directory
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    
                                    Text {
                                        Layout.fillWidth: true
                                        text: model.command || "N/A"
                                        font.pixelSize: 12
                                        font.weight: Font.Medium
                                        color: "#dddddd"
                                        elide: Text.ElideRight
                                    }
                                    
                                    Text {
                                        Layout.fillWidth: true
                                        text: "📁 " + (model.workingDir || "N/A")
                                        font.pixelSize: 11
                                        color: "#888888"
                                        elide: Text.ElideMiddle
                                    }
                                }
                                
                                // Action buttons
                                RowLayout {
                                    Layout.preferredWidth: 200
                                    spacing: 8
                                    
                                    Item { Layout.fillWidth: true }
                                    
                                    // Stop button
                                    Rectangle {
                                        width: 80
                                        height: 32
                                        radius: 6
                                        color: model.state === root.stateRunning ? Qt.rgba(0.8, 0.6, 0.2, 0.2) : Qt.rgba(0.3, 0.3, 0.3, 0.2)
                                        border.width: 1
                                        border.color: model.state === root.stateRunning ? Qt.rgba(0.8, 0.6, 0.2, 0.4) : Qt.rgba(0.3, 0.3, 0.3, 0.4)
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Stop"
                                            font.pixelSize: 11
                                            font.weight: Font.Medium
                                            color: model.state === root.stateRunning ? "#fbbf24" : "#666666"
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            enabled: model.state === root.stateRunning // Only enabled if running
                                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                            
                                            onClicked: {
                                                if (processManager) {
                                                    processManager.stopProcess(model.id, 5000)
                                                    toastMessage.show("Stopping process: " + model.id)
                                                }
                                            }
                                        }
                                    }
                                    
                                    // Kill button
                                    Rectangle {
                                        width: 80
                                        height: 32
                                        radius: 6
                                        color: model.state === root.stateRunning ? Qt.rgba(0.9, 0.2, 0.2, 0.2) : Qt.rgba(0.3, 0.3, 0.3, 0.2)
                                        border.width: 1
                                        border.color: model.state === root.stateRunning ? Qt.rgba(0.9, 0.2, 0.2, 0.4) : Qt.rgba(0.3, 0.3, 0.3, 0.4)
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Kill"
                                            font.pixelSize: 11
                                            font.weight: Font.Medium
                                            color: model.state === root.stateRunning ? "#ef4444" : "#666666"
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            enabled: model.state === root.stateRunning // Only enabled if running
                                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                            
                                            onClicked: {
                                                if (processManager) {
                                                    processManager.killProcess(model.id)
                                                    toastMessage.show("Force killing process: " + model.id)
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                // Empty state
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: processListModel.count === 0
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 12
                        
                        Text {
                            text: "📋"
                            font.pixelSize: 48
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text {
                            text: "No Processes"
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#ffffff"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text {
                            text: "Start a project or script to see processes here"
                            font.pixelSize: 13
                            color: "#888888"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
        }
    }
    
    // Toast message component
    Toast {
        id: toastMessage
    }
    
    // Kill All confirmation dialog
    Dialog {
        id: killAllDialog
        title: "Confirm Kill All"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 400
        
        background: Rectangle {
            color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
            border.color: Qt.rgba(1, 0, 0, 0.3)
            border.width: 1
            radius: 12
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 16
            
            Text {
                text: "Are you sure you want to force kill all running processes?"
                font.pixelSize: 13
                color: "#ffffff"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            Text {
                text: "⚠️ This will immediately terminate all processes without graceful shutdown."
                font.pixelSize: 12
                color: "#ef4444"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Cancel"
                    onClicked: killAllDialog.close()
                }
                
                Button {
                    text: "Kill All"
                    onClicked: {
                        if (processManager) {
                            var processes = processManager.getAllProcessesInfo()
                            for (var i = 0; i < processes.length; i++) {
                                if (processes[i].state === root.stateRunning) { // Running
                                    processManager.killProcess(processes[i].id)
                                }
                            }
                            toastMessage.show("Force killing all running processes...")
                        }
                        killAllDialog.close()
                    }
                    
                    background: Rectangle {
                        color: "#dc2626"
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
