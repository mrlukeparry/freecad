// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {

    property alias text: buttonText.text
    property bool enabled: true
    width: 120
    height: 30

    id: button
    Rectangle {
        id: rectangle
        radius: 5
        border.width: 2
        border.color: "#ffffff"
        clip: true
        anchors.fill: parent
        color: "#111111"
    }

    signal clicked

    MouseArea {
        id: mouseArea
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        anchors.fill : parent
        onClicked: button.clicked()
    }
    Text {
        id: buttonText
        color: "#ffffff"
        text: "Stop Render"
        font.bold: false
        style: Text.Normal
        font.pointSize: 11
        anchors.centerIn: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    transitions: Transition {
        ColorAnimation { duration: 200 }
    }

    states: [

        State {
            name: "down"; when: mouseArea.pressed && button.enabled
            PropertyChanges { target: rectangle;  color: "green" }
        },
        State {
            name: "hover"; when: mouseArea.containsMouse && button.enabled
            PropertyChanges { target: rectangle;  color: "orange" }
        },
        State {
        name: "disabled"
        when: !enabled
        PropertyChanges {target: button; opacity: 0.2}
        }

    ]


}
