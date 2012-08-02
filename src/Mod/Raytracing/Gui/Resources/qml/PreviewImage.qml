// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: previewImage
    property alias myWidth: pictureSrc.width
    property alias myHeight: pictureSrc.height
    property alias source: pictureSrc.source
    width: 800
    height: 600
    Image {
        cache: false
        id: pictureSrc
        asynchronous: true
    }

    Rectangle {
        anchors.fill: pictureSrc
        border.color: "white"
        border.width: 2
        radius: 3
        color: "transparent"
    }

}
