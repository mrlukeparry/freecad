import QtQuick 1.1
Item {
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
        spacing: 10
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
                model: materialParametersModel
                delegate:  Item {
                    function createParameter(mod)
                    {

                        var myComponent = Qt.createComponent("Button.qml")
                        if(mod.type == "float") {
                            var hasRange = (mod.minVal && mod.maxVal);
                            var numComp = Qt.createComponent("NumberInput.qml")
                            if(numComp.status == Component.Ready) {
                                var input = numComp.createObject(row,
                                                                 {'label': mod.label,
                                                                  'text': materialData.getPropertyValue(mod.id),
                                                                 });
                                row.height = input.height + 10
                                input.valueChanged.connect(function(){materialData.setProperty(mod.id, parseFloat(input.text));});
                            }
                        } else {
                            var button =  myComponent.createObject(row)
                            row.height = button.height + 10
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
