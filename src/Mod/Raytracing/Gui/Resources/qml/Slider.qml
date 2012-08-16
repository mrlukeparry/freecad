// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    width: 300 + 8
    id: slider
    height: control.height + 8 // Largest element in slider

    property int min:0;
    property int max: 100;
    property int val;

    // For theming
    property alias sliderColor: bar.color
    property alias controlColor: control.color

    anchors.margins: control.width

    signal sliderMove(int myVal)

    function updatePos()
    {
        control.x = bar.width * (slider.val / (max-min)) - control.width / 2
    }

    function updateValue(value)
    {
        if(value)
            slider.val = value;
        control.x = bar.width * (slider.val / (max-min)) - control.width / 2
        return slider.val;
    }


    function setValue()
    {
        slider.val = min + (max-min) * control.xPos / bar.width
        return slider.val;
    }

    //width: parent.width
    Rectangle
    {
        id: bar
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        height: 6
        radius: 2
        border.width: 2
        border.color: "#ffffff"
        color: "#111111"

        Rectangle
        {
            id: control
            property int xPos: x + width / 2
            anchors.verticalCenter: bar.verticalCenter
            width: 16
            height: 16
            radius: 8
            border.width: 2
            border.color: "#ffffff"
            clip: true
            color: "#111111"
        }
        MouseArea
        {
            id: controlMouseArea
            anchors.fill: control
            drag.target: control
            drag.axis: Drag.XAxis
            drag.minimumX: -width / 2
            drag.maximumX: parent.width -width / 2
            onMouseXChanged: {if(drag.active) {slider.sliderMove(slider.setValue())} }
        }
    }
}
