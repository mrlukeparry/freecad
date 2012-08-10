
import QtQuick 1.1
Item {
    id: renderTaskWidget

    // Signals
    signal previewWindow()
    signal preview()
    signal render()

    // Slots
    function renderRunning() { renderActive = true }
    function renderStopped() { renderActive = false }

    // Properties
    property bool renderActive: false


    width: 300
    height: 800
    anchors.fill: parent

    Rectangle {
        width: 100
        height: 62
        color: "#403d3d"
        anchors.fill: parent
        MouseArea {
            anchors.fill: parent
            onPressed: parent.focus = true
        }

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 25
            spacing: 10
            Row {
                spacing: 10

                 Button {
                     id: previewButton
                     anchors.bottom: parent.bottom
                     anchors.bottomMargin: 0
                     text: qsTr("Preview")
                     onClicked: renderTaskWidget.preview()
                     enabled: !renderTaskWidget.renderActive
                 }
                 Button {
                     id: previeWindowButton
                     anchors.bottom: parent.bottom
                     anchors.bottomMargin: 0
                     text: qsTr("Preview Window")
                     onClicked: renderTaskWidget.previewWindow()
                     enabled: !renderTaskWidget.renderActive
                 }
            }
            Row {
                width: parent.width
                Button {
                    id: renderButton
                    width: parent.width
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 0
                    text: qsTr("Render")
                    enabled: !renderTaskWidget.renderActive
                    onClicked: renderTaskWidget.render()

                }
            }
            Row {
                Column {
                    spacing: 10
                    Text {
                        color: "#f9f9f9"
                        text: qsTr("Render Properties:")
                        font.pointSize: 12
                    }
                    Row {
                        spacing: 20

                        NumberInput {
                            id: sizeX
                            numDigits: 4
                            text: renderFeature.getOutputX().toString()
                            inputMask: "0000"
                            onValueChanged: renderFeature.setOutputX(parseInt(sizeX.text))
                            suffix: "px"
                            label: "x:"
                        }
                        NumberInput {
                            id: sizeY
                            numDigits: 4
                            text: renderFeature.getOutputY().toString()
                            inputMask: "0000"
                            onValueChanged: renderFeature.setOutputY(parseInt(sizeY.text))
                            suffix: "px"
                            label: "y:"
                        }

                     } // Row End
                     Row {
                         width: parent.width

                         ComboBox {
                             id: renderPreset
                             model: ["directLighting", "metropolisUnbiased"]
                             selectedItem: renderFeature.getRenderPreset()
                             width: parent.width
                             height: 20
                             onComboClicked: renderFeature.setRenderPreset(renderPreset.selectedItem)
                         }
                     }
                     Row {
                         width: parent.width

                         ComboBox {
                             id: renderTemplate
                             model: ["lux_default"]
                             selectedItem: renderFeature.getRenderTemplate()
                             width: parent.width
                             height: 20
                             onComboClicked: renderFeature.setRenderTemplate(renderPreset.selectedItem)
                         }
                     }

                } // Column End
            }
        }
    }
}
