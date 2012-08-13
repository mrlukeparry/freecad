// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: numberInput
    property alias numDigits: textInput.maximumLength
    property alias label: inputLabel.text
    property alias suffix: suffixLabel.text
    property alias text: textInput.text
    property alias inputMask: textInput.inputMask
    property alias validator: textInput.validator
    width: 100
    height: 20

    signal valueChanged()

    Rectangle {
        x: 0
        y: 0
       anchors.fill: parent
        radius: 3
        border.width: 2
        border.color: "#ffffff"
        clip: true
        color: "#111111"
    }

    Text {
        id: inputLabel
        anchors.left:  parent.left
        anchors.leftMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        font.pointSize: 10
        color: textInput.color
        opacity: 0.8
    }

    MouseArea {
        anchors.fill: parent
        onClicked: textInput.forceActiveFocus()
    }

    TextInput {
        id: textInput
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 5
        anchors.right: suffixLabel.left
        color: "#ffffff"
        selectByMouse: true
        font.pointSize: 10
        horizontalAlignment: TextInput.AlignLeft
        onFocusChanged: function() {if(validator == undefined || textInput.acceptableInput) { numberInput.valueChanged()} }()
    }
    Text {
        id: suffixLabel
        anchors.right:  parent.right
        anchors.rightMargin: 5
        anchors.bottomMargin: 3
        anchors.bottom: parent.bottom
        font.pointSize: 7
        color: textInput.color
    }


}
