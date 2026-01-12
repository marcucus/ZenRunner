import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "./components"

// Vue multi-terminaux avec tabs pour les projets d'un workspace
// Optimisé pour utiliser ListView au lieu de TextEdit pour des performances maximales
Item {
    id: root
    
    property bool isWorkspace: false
    property var projects: []
    property int currentProjectIndex: 0
    property var activeProcesses: ({})
    
    // Map pour stocker les LogViewModels de chaque projet
    property var projectLogModels: ({})
    
    // Fonction pour obtenir ou créer un LogViewModel pour un projet
    function getOrCreateLogModel(projectIndex) {
        if (!projectLogModels[projectIndex]) {
            // Le LogViewModel sera créé par le composant LogConsoleOptimized
            projectLogModels[projectIndex] = null
        }
        return projectLogModels[projectIndex]
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
                    // Clear the current project's log buffer
                    if (logConsoleRepeater.itemAt(currentProjectIndex)) {
                        logConsoleRepeater.itemAt(currentProjectIndex).clearLogs()
                    }
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
                    // Clear the current project's log buffer
                    if (logConsoleRepeater.itemAt(currentProjectIndex)) {
                        logConsoleRepeater.itemAt(currentProjectIndex).clearLogs()
                    }
                }
            }
        }
        
        // Zone de sortie du terminal - Utilise ListView pour de meilleures performances
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: Qt.rgba(0, 0, 0, 0.6)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)
            
            // Utilise un StackLayout pour afficher le bon terminal
            StackLayout {
                anchors.fill: parent
                anchors.margins: 8
                currentIndex: currentProjectIndex
                
                // Crée un LogConsoleOptimized pour chaque projet
                Repeater {
                    id: logConsoleRepeater
                    model: Math.max(1, projects.length)
                    
                    LogConsoleOptimized {
                        required property int index
                        projectName: projects[index] ? projects[index].name : "Project " + (index + 1)
                        projectIndex: index
                    }
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
                        "Terminal output uses optimized ListView for smooth performance"
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


