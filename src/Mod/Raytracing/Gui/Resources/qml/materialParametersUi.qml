import QtQuick 1.1
Item {
    clip: true
    id: materialParametersWidget
    width: 300
    height: 700

    // Signals
    signal accepted()
    signal cancel()

    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#333"
        MouseArea {
            onPressed: parent.focus = true;
        }
    }
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.8
        spacing: 20
        Row {
            spacing: 10
            Button {
                id: acceptButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                text: qsTr("Save")
                onClicked: materialParametersWidget.accepted()

            }

            Button {
                id: cancelButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                text: qsTr("Cancel")
                onClicked: materialParametersWidget.cancel()

            }
        }
        Row {
            width: parent.width
            ListView {
                snapMode: ListView.SnapToItem
                highlightRangeMode: ListView.StrictlyEnforceRange
                flickDeceleration: 7000
                focus: true
                height: 800
                width: parent.width
                model: materialParametersModel
                delegate:  Item {
                    function createParameter(mod)
                    {
                        var myComponent = Qt.createComponent("Button.qml")
                        if(mod.type == "float") {
                            var hasRange = (mod.minVal && mod.maxVal);
                            var numComp = Qt.createComponent("NumberInput.qml")
                            if(numComp.status == Component.Ready) {
                                var numValue = (materialData.getPropertyValue(mod.id) === undefined) ? "": materialData.getPropertyValue(mod.id);
                                var input = numComp.createObject(row,
                                                                 {'label': mod.label,
                                                                  'text':  numValue
                                                                 });
                                row.height = input.height + 10
                                input.width = parent.width
                                input.valueChanged.connect(function(){materialData.setProperty(mod.id, parseFloat(input.text));});
                            }
                        } else if(mod.type == "color") {
                            var colorComp = Qt.createComponent("ColorWidget.qml")
                            if(colorComp.status == Component.Ready) {
                                var colorValue = materialData.getPropertyValue(mod.id);
                                var colorInput = colorComp.createObject(row,  {'label': mod.label});
                                if(colorValue !== undefined) {
                                    colorValue[0] *= 255;
                                    colorValue[1] *= 255;
                                    colorValue[2] *= 255;
                                    colorInput.setColor(colorValue);}
                                row.height = colorInput.height + 10
                                colorInput.valueChanged.connect(function(val){ materialData.setProperty(mod.id, val);});
                                colorInput.pickColor.connect(function(){ var colorPicked = materialData.pickColor(); colorInput.setColor(colorPicked);});
                            }
                        }
                    }
                    id: parametersDelegate
                    height: row.height
                    width: parent.width
                    Row {
                        id: row
                        width: parent.width
                    }

                    property variant modelData: {'id': paramId, 'label': label, 'description': description, 'type': type}
                    Component.onCompleted: createParameter(parametersDelegate.modelData);
                }
                ScrollBar {
                    flickable: parent
                    vertical: true
                }
            }
        }
    }


}
