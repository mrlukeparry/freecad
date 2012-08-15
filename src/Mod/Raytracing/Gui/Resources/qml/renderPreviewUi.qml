import QtQuick 1.1
import FreeCAD 1.0

Item {

    id: previewWidget
    // Slots
    function updatePreview() {
        // Bit of a hack to reload the image
        if(previewImage.source = "image://previewImage/reload/picture"){
             previewImage.source = "image://previewImage/picture"
        } else {
              previewImage.source = "image://previewImage/reload/picture"
        }
        if(!previewLoaded)
        {
            loadingAnim.start()
        }

        renderActive = true;
        previewLoaded = true;
    }

    function viewAll() {

        var yScale = previewWidget.height / (previewImage.myHeight);
        previewImage.zoomAnim.newX = 0;
        previewImage.zoomAnim.newY = 0;
        previewImage.zoomAnim.newZoom = yScale;
        previewImage.zoomAnim.start();
    }

    function renderActive() {
        console.log("Render Active")
        renderActive = true;
    }

    function renderStopped() {
                console.log("Render Stopped")
        renderActive = false;
    }

    // Signals
    signal stopRender()
    signal saveOutput()

    // Properties
    property bool previewLoaded: false
    property bool renderActive: false

    width: 800
    height: 600

    Rectangle {
        id: rectangle1
        anchors.fill: parent
        gradient: Gradient {
                    GradientStop { position: 0.0; color: "#333"}
                    GradientStop { position: 1.0; color: "#555" }
               }

        WheelArea {
            id: mouseWheelArea
            anchors.fill: parent
            onVerticalWheel: function(){ if(!previewLoaded){return;}  previewImage.mouseWheelCallback(delta,mouseArea.mouseX, mouseArea.mouseY ); }()
        }

        MouseArea {
                hoverEnabled : true
                id: mouseArea
                anchors.fill: parent
        }

        Loading {
          id: imageLoading
           anchors.centerIn:  rectangle1
        }

        PreviewImage {
            id: previewImage
            opacity: 0
            anchors.centerIn : parent
            property alias zoomAnim: zoomOut;

            MouseArea {
                id: dragMouseArea
                drag.target: parent
                anchors.fill: previewImage.children[1]
                onPressed:  parent.anchors.centerIn = undefined
                drag.axis: Drag.XandYAxis
                drag.minimumX: -parent.width / 2
                drag.maximumX: previewWidget.width -parent.width / 2
                drag.minimumY: -parent.height /2
                drag.maximumY: previewWidget.height -parent.height / 2
            }

            ParallelAnimation {
                id: zoomOut
                property int newX;
                property int newY;
                property real newZoom;

                NumberAnimation  { target: previewImage; property: "zoom"; to: zoomOut.newZoom; easing.type: Easing.OutCubic; duration: 300 }
                NumberAnimation  { target: previewImage; property: "x"; to:  zoomOut.newX; easing.type: Easing.OutCubic; duration: 300 }
                NumberAnimation  { target: previewImage; property: "y"; to:  zoomOut.newY; easing.type: Easing.OutCubic; duration: 300 }
            }
        }

       Row {
           anchors.right: parent.right
           anchors.rightMargin: 30
           anchors.bottom: parent.bottom
           anchors.bottomMargin: 25
           spacing: 10

           Button {
               id: fitAll
               anchors.bottom: parent.bottom
               anchors.bottomMargin: 0
               text: qsTr("Fit All")
               enabled: previewWidget.previewLoaded
               onClicked: previewWidget.viewAll()
           }

            Button {
                id: saveButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                text: qsTr("Save")
                enabled: previewWidget.previewLoaded
                onClicked: previewWidget.saveOutput()

            }
            Button {
                id: stopButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                text: qsTr("Stop Render")
                enabled: previewWidget.renderActive
                onClicked: previewWidget.stopRender()
            }
       }
    }
    SequentialAnimation {
        id: loadingAnim
             running: false
             NumberAnimation  { target: imageLoading; property: "opacity"; to: 0; duration: 500 }
             NumberAnimation { target: previewImage; property: "opacity"; to: 1; duration: 500 }
         }

}
