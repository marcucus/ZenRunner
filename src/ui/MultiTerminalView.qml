import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "./components"

// Vue multi-terminaux avec tabs pour les projets d'un workspace
Item {
    id: root
    
    property bool isWorkspace: false
    property var projects: []
    property int currentProjectIndex: 0
    property var activeProcesses: ({})
    
    // Map pour stocker les outputs de chaque projet
    property var projectOutputs: ({})
    property var pendingUpdates: ({})
    property int maxBufferLines: 500
    
    // Timer pour batch les updates (réduit les ralentissements)
    Timer {
        id: updateTimer
        interval: 250
        repeat: false
        onTriggered: {
            flushPendingUpdates()
        }
    }
    
    // Connections pour écouter les sorties des processus
    Connections {
        target: processManager
        
        function onProcessOutput(id, output) {
            if (activeProcesses[id]) {
                var projectIndex = activeProcesses[id].projectIndex
                queueOutputUpdate(projectIndex, output)
            }
        }
        
        function onProcessError(id, error) {
            if (activeProcesses[id]) {
                var projectIndex = activeProcesses[id].projectIndex
                queueOutputUpdate(projectIndex, "\n❌ Error: " + error + "\n")
            }
        }
        
        function onProcessFinished(id, exitCode) {
            if (activeProcesses[id]) {
                var projectIndex = activeProcesses[id].projectIndex
                queueOutputUpdate(projectIndex, "\n✅ Process finished with exit code: " + exitCode + "\n")
            }
        }
    }
    
    function queueOutputUpdate(projectIndex, text) {
        if (!pendingUpdates[projectIndex]) {
            pendingUpdates[projectIndex] = ""
        }
        pendingUpdates[projectIndex] += text
        
        if (!updateTimer.running) {
            updateTimer.start()
        }
    }
    
    function flushPendingUpdates() {
        for (var projectIndex in pendingUpdates) {
            if (pendingUpdates[projectIndex]) {
                appendOutputToProject(parseInt(projectIndex), pendingUpdates[projectIndex])
                pendingUpdates[projectIndex] = ""
            }
        }
    }
    
    function appendOutputToProject(projectIndex, text) {
        if (!projectOutputs[projectIndex]) {
            projectOutputs[projectIndex] = "Terminal ready. Run a script to see output...\n"
        }
        
        projectOutputs[projectIndex] += text
        
        // Limiter la taille du buffer pour éviter les ralentissements
        // Ne faire le trim que si on dépasse significativement la limite
        if (projectOutputs[projectIndex].length > maxBufferLines * 120) {
            var lines = projectOutputs[projectIndex].split('\n')
            if (lines.length > maxBufferLines) {
                lines = lines.slice(lines.length - maxBufferLines)
                projectOutputs[projectIndex] = lines.join('\n')
            }
        }
        
        // Mettre à jour seulement si c'est le projet visible
        if (projectIndex === currentProjectIndex) {
            terminalOutput.text = projectOutputs[projectIndex]
            // Auto-scroll seulement si on est proche du bas
            if (terminalOutput.length - terminalOutput.cursorPosition < 500) {
                Qt.callLater(function() {
                    terminalOutput.cursorPosition = terminalOutput.length
                })
            }
        }
    }
    
    function appendOutput(text) {
        appendOutputToProject(currentProjectIndex, text)
    }
    
    // Mettre à jour l'affichage quand on change de projet
    onCurrentProjectIndexChanged: {
        if (!projectOutputs[currentProjectIndex]) {
            projectOutputs[currentProjectIndex] = "Terminal ready. Run a script to see output...\n"
        }
        terminalOutput.text = projectOutputs[currentProjectIndex]
        terminalOutput.cursorPosition = terminalOutput.length
    }

GlassCard {
    id: terminalCard
    anchors.fill: parent
    
    glassOpacity: 0.1
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        
        // Header avec tabs si workspace
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: isWorkspace && projects.length > 1
            
            Text {
                text: "Terminal Output:"
                font.pixelSize: 13
                color: "#888888"
            }
            
            // Tabs pour changer de projet
            Repeater {
                model: projects.length
                
                Rectangle {
                    required property int index
                    
                    width: 120
                    height: 32
                    radius: 8
                    color: currentProjectIndex === index ? 
                        Qt.rgba(0.3, 0.6, 1.0, 0.3) : 
                        Qt.rgba(1, 1, 1, 0.05)
                    border.width: currentProjectIndex === index ? 1 : 0
                    border.color: Qt.rgba(0.3, 0.6, 1.0, 0.5)
                    
                    Text {
                        anchors.centerIn: parent
                        text: projects[index] ? projects[index].name : "Project " + (index + 1)
                        font.pixelSize: 12
                        font.weight: currentProjectIndex === index ? Font.Bold : Font.Normal
                        color: currentProjectIndex === index ? "#4a90e2" : "#888888"
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: currentProjectIndex = index
                    }
                }
            }
            
            Item { Layout.fillWidth: true }
            
            // Bouton clear
            GlassButton {
                text: "🗑️ Clear"
                accentColor: "#ef4444"
                width: 90
                onClicked: {
                    projectOutputs[currentProjectIndex] = ""
                    terminalOutput.text = ""
                    var temp = projectOutputs
                    projectOutputs = temp
                }
            }
        }
        
        // Header simple si projet simple
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: !isWorkspace || projects.length <= 1
            
            Text {
                text: "📟 Terminal Output"
                font.pixelSize: 14
                font.weight: Font.Bold
                color: "#ffffff"
            }
            
            Item { Layout.fillWidth: true }
            
            GlassButton {
                text: "🗑️ Clear"
                accentColor: "#ef4444"
                width: 90
                onClicked: {
                    projectOutputs[currentProjectIndex] = ""
                    terminalOutput.text = ""
                    var temp = projectOutputs
                    projectOutputs = temp
                }
            }
        }
        
        // Zone de sortie du terminal
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: Qt.rgba(0, 0, 0, 0.6)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                
                TextEdit {
                    id: terminalOutput
                    width: parent.width
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.NoWrap
                    textFormat: TextEdit.PlainText
                    renderType: Text.NativeRendering
                    
                    // Désactiver les animations de cursor
                    cursorVisible: false
                    
                    // Optimisations de performance
                    persistentSelection: false
                    
                    font.family: "Menlo, Monaco, Courier"
                    font.pixelSize: 10
                    color: "#00ff00"
                    
                    text: "Terminal ready. Run a script to see output...\n"
                    
                    // Style du texte sélectionné
                    selectionColor: Qt.rgba(0.3, 0.6, 1.0, 0.5)
                    selectedTextColor: "#ffffff"
                }
            }
            
            // Indicateur de processus actif
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 12
                width: 100
                height: 28
                radius: 14
                color: Qt.rgba(0, 0, 0, 0.7)
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.2)
                visible: Object.keys(activeProcesses).length > 0
                
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 6
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "#4ade80"
                        
                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 800 }
                            NumberAnimation { to: 1.0; duration: 800 }
                        }
                    }
                    
                    Text {
                        text: Object.keys(activeProcesses).length + " active"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: "#4ade80"
                    }
                }
            }
        }
        
        // Barre d'info en bas
        Rectangle {
            Layout.fillWidth: true
            height: 32
            radius: 6
            color: Qt.rgba(1, 1, 1, 0.05)
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 16
                
                Text {
                    text: "💡"
                    font.pixelSize: 14
                }
                
                Text {
                    text: isWorkspace ? 
                        "Click project tabs above to switch between terminal outputs" :
                        "Terminal output will appear here when you run a script"
                    font.pixelSize: 11
                    color: "#888888"
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: Qt.formatDateTime(new Date(), "hh:mm:ss")
                    font.pixelSize: 11
                    color: "#666666"
                    font.family: "Monaco, Consolas, monospace"
                    
                    Timer {
                        interval: 1000
                        running: true
                        repeat: true
                        onTriggered: parent.text = Qt.formatDateTime(new Date(), "hh:mm:ss")
                    }
                }
            }
        }
    }
}

}


