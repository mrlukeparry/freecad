// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: previewImage
    property alias myWidth: pictureSrc.width
    property alias myHeight: pictureSrc.height
    property alias src: pictureSrc.source

    height: pictureSrc.height
    width: pictureSrc.width
    Image {
        id: pictureSrc
        source: "images/luxout.png"
        asynchronous: false
        cache: false
        onStatusChanged: if (pictureSrc.status == Image.Ready) console.log('Loaded')
    }

    Rectangle {
        anchors.fill: pictureSrc
        border.color: "white"
        border.width: 3
        radius: 2
        color: "transparent"
    }


}
