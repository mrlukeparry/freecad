// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: colorWidget

    // Properties
    property int red: 0
    property int blue: 0
    property int green: 0
    property string label;

    // Slot
    function setColor(val)
    {
        console.log(val)
    }

    //Signals
    signal  pickColor()

    height: col.height
    width: parent.width
    onRedChanged   : { colorWidget.valueChanged(Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)); }
    onBlueChanged  : { colorWidget.valueChanged(Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)); }
    onGreenChanged : { colorWidget.valueChanged(Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)); }

    signal valueChanged(color val)
    Column {
        id: col
        spacing: 5
        width: parent.width
        Row {
            height: 30
            Text {
                text: label
                color: "#fff"
                font.pointSize: 12
            }
        }

        Row {
            id: red
            height: redNumInput.height
            width: parent.width
            signal redChanged()

            spacing: 20
            Slider {
                id: redSlider
                width: 200
                sliderColor: "#cc0000"
                min: 0
                max: 255
                onSliderMove: { redNumInput.text = val.toString(); colorWidget.red = val; redChanged(); }
            }
            NumberInput {
                id: redNumInput
                numDigits: 3
                width: 40
                validator: IntValidator { bottom: 0; top: 255 }
                onValueChanged: { redSlider.val = parseInt(redNumInput.text); colorWidget.red = redSlider.val; redSlider.updatePos(); redChanged(); }
            }
        }
        Row {
            id: green
            height: greenNumInput.height
            width: parent.width
            signal greenChanged()

            spacing: 20
            Slider {
                id: greenSlider
                width: 200
                sliderColor: "#008000"
                min: 0
                max: 255
                onSliderMove: { greenNumInput.text = val.toString(); colorWidget.green = val; greenChanged(); }
            }
            NumberInput {
                id: greenNumInput
                numDigits: 3
                width: 40
                validator: IntValidator { bottom: 0; top: 255 }
                onValueChanged: { greenSlider.val = parseInt(greenNumInput.text); colorWidget.green = greenSlider.val; greenSlider.updatePos(); greenChanged(); }
            }
        }
        Row {
            id: blue
            height: blueNumInput.height
            width: parent.width
            signal blueChanged()

            spacing: 20
            Slider {
                id: blueSlider
                width: 200
                min: 0
                max: 255
                sliderColor: Qt.rgba(0,0,0.6,1)
                onSliderMove: { blueNumInput.text = val.toString(); colorWidget.blue = val; blueChanged(); }
            }
            NumberInput {
                id: blueNumInput
                numDigits: 3
                width: 40
                validator: IntValidator { bottom: 0; top: 255 }
                onValueChanged: { blueSlider.val = parseInt(blueNumInput.text); colorWidget.blue = blueSlider.val; blueSlider.updatePos(); blueChanged(); }
            }
        } // End Row

        Row {
            width: parent.width
            height: 30
            Rectangle {
                id: mixer

                width: blueNumInput.width
                height: blueNumInput.height
                anchors.leftMargin: 220
                anchors.left: parent.left
                radius: 3
                border.width: 2
                border.color: "#ffffff"
                color: Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)
            }

        }
    }
}
