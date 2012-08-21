import QtQuick 1.1
Item {
    id: renderTaskWidget

    // Signals
    signal previewWindow()
    signal preview()
    signal render()
    signal saveCamera()

    // Material Signals
    signal materialSelectionChanged(int val)
    signal editMaterial(int index)

    // Slots
    function renderRunning() { renderActive = true }
    function renderStopped() { renderActive = false }

    // Material Slots
    function getMaterialSelection()
    {
        return materialListView.selection
    }

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
                width: parent.width
                spacing: width * 0.05
                 Button {
                     id: previewButton
                     width: parent.width * 0.33
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
                      width: parent.width * 0.62
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
               layoutDirection: Qt.RightToLeft
               width: parent.width
               Button {
                   id: saveCameraButton
                   anchors.bottom: parent.bottom
                   anchors.bottomMargin: 0
                   text: qsTr("Save Camera")
                   onClicked: renderTaskWidget.saveCamera()

               }
            }
            Row {
                Column {
                    spacing: 10
                    Text {
                        color: "#f9f9f9"
                        text: qsTr("Render Properties")
                        font.pointSize: 12
                    }
                    Row {
                        spacing: 20

                        NumberInput {
                            id: sizeX
                            text: renderFeature.getOutputX().toString()
                            validator: IntValidator { bottom: 1; top: 10000 }
                            onValueChanged: renderFeature.setOutputX(parseInt(sizeX.text))
                            suffix: "px"
                            label: "x:"
                        }
                        NumberInput {
                            id: sizeY
                            text: renderFeature.getOutputY().toString()
                            onValueChanged: renderFeature.setOutputY(parseInt(sizeY.text))
                            validator: IntValidator { bottom: 1; top: 10000 }
                            suffix: "px"
                            label: "y:"
                        }

                     } // Row End
                     Row {
                        width: parent.width
                        NumberInput {
                            id: updateInterval
                            width: parent.width
                            numDigits: 4
                            validator: IntValidator { bottom: 3; top: 1000 }
                            text: (renderFeature.getUpdateInterval() / 1000).toString() // in seconds
                            inputMask: "0000"
                            onValueChanged: renderFeature.setUpdateInterval(parseInt(updateInterval.text) * 1000) // in milliseconds
                            suffix: "s"
                            label: qsTr("Update Interval")
                        }
                      } // Row End
                     Row {
                         width: parent.width

                         ComboBox {
                             id: renderPreset
                             model: presetsModel
                             property string preset;
                             width: parent.width
                             height: 20
                             onComboClicked: renderFeature.setRenderPreset(renderPreset.preset)
                             delegate: Item {
                                 id: presetsDelegate
                                 width: parent.width;
                                 height: presDelLabel.height + presDelDesc.height + 15

                                 Text {
                                     id: presDelLabel
                                     text: label
                                     anchors.verticalCenter: parent.center
                                     anchors.right: parent.right
                                     anchors.rightMargin: 10
                                     color: "#fff"
                                 }

                                 Text {
                                     id: presDelDesc
                                     text: description
                                     anchors.top: presDelLabel.bottom
                                     anchors.topMargin: 5
                                     anchors.right: parent.right
                                     anchors.rightMargin: 10
                                     width: parent.width
                                     horizontalAlignment: TextInput.AlignRight
                                     font.pointSize: 7
                                     color: "#fff"
                                     opacity: 0.6
                                     states: State {name: "display"; when: preDelMouseArea.containsMouse
                                                    PropertyChanges { target: presDelDesc; opacity: 1.0} }
                                 }
                                 MouseArea {
                                     id: presDelMouseArea
                                     hoverEnabled: true;
                                     anchors.fill: parent
                                     onClicked: {
                                         comboBox.state = ""
                                         var prevSelection = chosenItemText.text
                                         chosenItemText.text = label
                                         if(chosenItemText.text != prevSelection){
                                             renderPreset.preset = id
                                             comboBox.comboClicked();
                                         }
                                         listView.currentIndex = index;
                                     }
                                 }
                             }
                             selectedItem: { var item = renderPreset.model.getById(renderFeature.getRenderPreset()); if(item !== undefined) { return item.label; } }
                         }
                     } // Row End
                     Row {
                         width: parent.width
                         ComboBox {

                             id: renderTemplate
                             property string templateName;

                             width: parent.width
                             height: 20
                             onComboClicked: renderFeature.setRenderTemplate(renderTemplate.templateName)
                             model: templatesModel
                             delegate: Item {
                                 id: templatesDelegate
                                 width: parent.width;
                                 height: tempDelLabel.height + tempDelDesc.height + 15

                                 Text {
                                     id: tempDelLabel
                                     text: label
                                     anchors.verticalCenter: parent.center
                                     anchors.right: parent.right
                                     anchors.rightMargin: 10
                                     color: "#fff"
                                 }

                                 Text {
                                     id: tempDelDesc
                                     text: description
                                     anchors.top: tempDelLabel.bottom
                                     anchors.topMargin: 5
                                     anchors.right: parent.right
                                     anchors.rightMargin: 10
                                     width: parent.width
                                     horizontalAlignment: TextInput.AlignRight
                                     font.pointSize: 7
                                     color: "#fff"
                                     opacity: 0.6
                                     states: State {name: "display"; when: tempDelMouseArea.containsMouse
                                                    PropertyChanges { target: tempDelDesc; opacity: 1.0} }
                                 }
                                 MouseArea {
                                     id: tempDelMouseArea
                                     hoverEnabled: true;
                                     anchors.fill: parent
                                     onClicked: {
                                         comboBox.state = ""
                                         var prevSelection = chosenItemText.text
                                         chosenItemText.text = label
                                         if(chosenItemText.text != prevSelection){
                                             renderTemplate.templateName = id
                                             comboBox.comboClicked();
                                         }
                                         listView.currentIndex = index;
                                     }
                                 }
                             }
                             selectedItem: { var item = renderTemplate.model.getById(renderFeature.getRenderTemplate()); if(item !== undefined) { return item.label; } }

                         }
                     } // Row End


                } // Column End
            } // End Material Params Row
            Row {
                width: parent.width
                Column {
                    width: parent.width
                    spacing: 10
                    Text {
                        color: "#f9f9f9"
                        text: qsTr("Render Materials")
                        font.pointSize: 12
                    }
                    Rectangle {
                        id: materialListBorder
                        border.width: 2
                        border.color: "#ffffff"
                        clip: true
                        radius: 2
                        color: "#ccc"

                        width: parent.width
                        height: renderTaskWidget.height * 0.3
                        ListView {
                            property variant selection: []
                            id:materialListView
                            clip: true
                            anchors.fill: parent
                            anchors.margins: 1
                            snapMode: ListView.SnapToItem
                            model: materialsModel
                            flickDeceleration: 7000

                            function clearSelection(){
                                model.clearSelection();
                            }

                            function find(idx) {
                                var i = materialListView.selection.length;
                                var fndIndex = -1;
                                while (i--) {
                                   if (materialListView.selection[i] === idx) {
                                       fndIndex = i;
                                   }
                                }
                                return fndIndex;
                            }

                            onFocusChanged: {materialListView.clearSelection()}
                            delegate: Item {

                                id: delg
                                height: 30
                                width: materialListView.width

                                Rectangle {
                                    id: delgBackground
                                    width: parent.width
                                    height: parent.height
                                    color: (index % 2) ? "#ccc" : "#ddd"
                                    states: [State {
                                        name: "hover"; when: mouseArea.containsMouse
                                        PropertyChanges { target: delgBackground;  color: "orange" }
                                    }, State {
                                            name: "selected"; when: selected
                                        PropertyChanges { target: delgBackground;  color: "blue" }
                                    }]
                                }
                                Text { id: matLabel; text: label  }
                                Text { id: matLinkLabel; anchors.top: matLabel.bottom; text: linkLabel }
                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    onDoubleClicked: renderTaskWidget.editMaterial(index)

                                    onClicked: {
                                        if(!(mouse.modifiers & Qt.ShiftModifier)) { materialListView.clearSelection()}
                                        materialListView.model.setState(index, !selected);
                                        renderTaskWidget.materialSelectionChanged(index);
                                    }
                                    hoverEnabled: true


                                }
                            }
                        }
                    }

                } // End Column
            } // End Render Materials Row
        }
    }
}
