
import QtQuick 1.1
Item {
    id: appearancesTaskWidget

    height: 800
    clip: true;
    anchors.fill: parent

    // Slots
    function openMaterialLibraryWidget()
    {
        parametersUiLoader.visible = false;
        appearancesTaskWidget.state = "";
    }

    function openMaterialParametersWidget()
    {
        parametersUiLoader.source = "";
        parametersUiLoader.source = "materialParametersUi.qml";
        appearancesTaskWidget.state = "viewProperties";
    }

    // Signals
    signal materialDrag(string  matId)
    signal materialPropsAccepted()
    signal materialPropsCancel()

    //Connections to Material Paremeters Panel
    Connections {
             target: parametersUiLoader.item
             onCancel: {  appearancesTaskWidget.materialPropsCancel() }
             onAccepted: appearancesTaskWidget.materialPropsAccepted()

    }
    Component {
         id: libMatDelegate
         Item {
             width: parent.width
             height: 180
             id: libMat

             property string matId: materialId

            Image {
              id: materialPreview
              source: (previewFilename == "") ? "": "file:/" + previewFilename
              width: 150
              height: 150
              anchors.horizontalCenter: parent.horizontalCenter
            }
            Rectangle {
              anchors.fill: materialPreview
              border.color: "grey"
              border.width: 2
              radius: 3
              color: "transparent"
            }

            Text {
              id: matLabel
              text: label
              horizontalAlignment: Text.AlignHCenter
              anchors.topMargin: 3
              anchors.top: materialPreview.bottom
              anchors.horizontalCenter: parent.horizontalCenter
            }
            MouseArea {

                property variant startPos
                property bool dragInit: false

                function startDrag(mouse, matId) {
                    var dist = Math.sqrt( Math.pow((mouse.x-startPos.x), 2) + Math.pow((mouse.y-startPos.y), 2))
                    if(dist > 4) {
                        materialDrag(matId);
                        dragInit = true;
                    }
                }
                function setStartPos(mouse) {startPos = Qt.point(mouse.x, mouse.y)}

                id: dragMouseArea
                anchors.fill: materialPreview
                preventStealing: true
                onPressed: setStartPos(mouse)
                onMousePositionChanged: startDrag(mouse, matId)
                onReleased: dragInit = false;
            }
         }
     }
    ListView {
        id: materialList
        width: 300
        height: 700
        snapMode: ListView.SnapToItem
        flickDeceleration: 7000
        focus: true
        model: appearancesModel
        delegate: libMatDelegate

        clip: true
        ScrollBar {
            flickable: parent
            vertical: true
        }

    }

    Loader {
        id: parametersUiLoader
        anchors.left: materialList.right
        visible: false
    }

    transitions: Transition {
        //NumberAnimation {properties: "x";  duration: 400;}
    }

    states: [
        State {
        name: "viewProperties"
        PropertyChanges {target: materialList;
                         x: -materialList.width}
        PropertyChanges {target: parametersUiLoader; visible: true}
        }
    ]
}
