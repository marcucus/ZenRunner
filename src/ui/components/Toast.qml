import QtQuick
import QtQuick.Controls

// Toast notification component for in-app feedback
Rectangle {
    id: toast
    
    property string message: ""
    property int duration: 3000  // milliseconds
    property color backgroundColor: Qt.rgba(0.2, 0.2, 0.2, 0.95)
    property color textColor: "#ffffff"
    
    width: Math.min(parent.width - 40, messageText.implicitWidth + 40)
    height: 50
    radius: 12
    color: backgroundColor
    opacity: 0
    visible: false
    
    // Position at bottom center of parent
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 80
    
    // Border for visual depth
    border.width: 1
    border.color: Qt.rgba(1, 1, 1, 0.1)
    
    Text {
        id: messageText
        text: toast.message
        color: toast.textColor
        font.pixelSize: 14
        font.weight: Font.Medium
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    
    // Show animation
    ParallelAnimation {
        id: showAnimation
        NumberAnimation {
            target: toast
            property: "opacity"
            from: 0
            to: 1
            duration: 200
            easing.type: Easing.OutQuad
        }
        NumberAnimation {
            target: toast
            property: "anchors.bottomMargin"
            from: 60
            to: 80
            duration: 200
            easing.type: Easing.OutBack
        }
    }
    
    // Hide animation
    ParallelAnimation {
        id: hideAnimation
        NumberAnimation {
            target: toast
            property: "opacity"
            from: 1
            to: 0
            duration: 200
            easing.type: Easing.InQuad
        }
        NumberAnimation {
            target: toast
            property: "anchors.bottomMargin"
            from: 80
            to: 100
            duration: 200
            easing.type: Easing.InQuad
        }
        onFinished: {
            toast.visible = false
        }
    }
    
    Timer {
        id: hideTimer
        interval: toast.duration
        repeat: false
        onTriggered: {
            hideAnimation.start()
        }
    }
    
    function show(msg, bgColor) {
        if (bgColor !== undefined) {
            backgroundColor = bgColor
        }
        message = msg
        visible = true
        showAnimation.start()
        hideTimer.restart()
    }
}
