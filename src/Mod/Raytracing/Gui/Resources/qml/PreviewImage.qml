// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: previewImage
    property alias myWidth: pictureSrc.width
    property alias myHeight: pictureSrc.height
    property alias source: pictureSrc.source
    property double zoom
    property int zoomX
    property int zoomY

    function mouseWheelCallback(delta, mouseX, mouseY)
    {
        var zoomScale = previewImage.zoom + delta * 0.0004;
        if(zoomScale < 0.2) {
            zoomScale = 0.2;
        }
        previewImage.zoom = zoomScale
    }

    zoom: 1.0
    zoomX: width /2
    zoomY: height/2
    width: 800
    height: 600

    Image {
        cache: false
        id: pictureSrc
        asynchronous: true
        scale: previewImage.zoom

    }

    Rectangle {
        anchors.centerIn: previewImage
        width: myWidth * previewImage.zoom
        height: myHeight * previewImage.zoom
        border.color: "white"
        border.width: 2
        radius: 3
        color: "transparent"
    }


}
