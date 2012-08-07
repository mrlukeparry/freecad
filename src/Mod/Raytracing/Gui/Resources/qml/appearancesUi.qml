
import QtQuick 1.1
Item {
    signal materialDrag(string  matId)
    height: 800
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
        anchors.fill: parent
        snapMode: ListView.SnapToItem
        highlightRangeMode: ListView.StrictlyEnforceRange

        flickDeceleration: 7000
        focus: true
        height: 800
        model: appearancesModel
        delegate: libMatDelegate

        ScrollBar {
            flickable: parent
            vertical: true
        }
    }
}
