// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
Item {
    id: loader
    width: 80
    height: 80
    Rectangle
    {

        color: "#ffffff"
        anchors.fill: parent
        radius: width / 2

    }
    Text {
        smooth:false
        anchors.centerIn: parent
        id: idText

        text: "loading"

         RotationAnimation on rotation {
                 loops: Animation.Infinite
                 from: 0
                 to: 360
                 duration: 2000
             easing.type: Easing.OutQuad
         }
    }
}
