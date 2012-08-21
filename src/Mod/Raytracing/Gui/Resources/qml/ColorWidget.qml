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
        colorWidget.red = val[0]; colorWidget.green = val[1]; colorWidget.blue=val[2];
    }

    // TODO put in Javascript helper
    //http://ejohn.org/blog/numbers-hex-and-colors/
    function toNumbers( str ){
        var ret = [];
        str.replace(/(..)/g, function(str){
        ret.push( parseInt( str, 16 ) );
      });
      return ret;
    }

    //Signals
    signal  pickColor()

    height: col.height
    width: parent.width
    onRedChanged   : { colorWidget.valueChanged(Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)); }
    onBlueChanged  : { colorWidget.valueChanged(Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)); }
    onGreenChanged : { colorWidget.valueChanged(Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)); ;}

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
            id: redRow
            height: redNumInput.height
            width: parent.width
            signal redChanged()

            spacing: 20
            Slider {
                id: redSlider
                width: parent.width * 0.8
                sliderColor: "#cc0000"
                min: 0
                val: updateValue(colorWidget.red)
                max: 255
                onSliderMove: { redNumInput.text = val.toString(); colorWidget.red = val; redChanged(); }
            }
            NumberInput {
                id: redNumInput
                numDigits: 3
                width: 40
                text: colorWidget.red.toString()
                validator: IntValidator { bottom: 0; top: 255 }
                onValueChanged: { redSlider.val = parseInt(redNumInput.text); colorWidget.red = redSlider.val; redSlider.updatePos(); redChanged(); }
            }
        }
        Row {
            id: greenRow
            height: greenNumInput.height
            width: parent.width
            signal greenChanged()

            spacing: 20
            Slider {
                id: greenSlider
                width: parent.width * 0.8
                sliderColor: "#008000"
                min: 0
                val: updateValue(colorWidget.green)
                max: 255
                onSliderMove: { greenNumInput.text = val.toString(); colorWidget.green = val; greenChanged(); }
            }
            NumberInput {
                id: greenNumInput
                numDigits: 3
                width: 40
                text: colorWidget.green.toString()
                validator: IntValidator { bottom: 0; top: 255 }
                onValueChanged: { greenSlider.val = parseInt(greenNumInput.text); colorWidget.green = greenSlider.val; greenSlider.updatePos(); greenChanged(); }
            }
        }
        Row {
            id: blueRow
            height: blueNumInput.height
            width: parent.width
            signal blueChanged()

            spacing: 20
            Slider {
                id: blueSlider
                width: parent.width * 0.8
                min: 0
                val: updateValue(colorWidget.blue)
                max: 255
                sliderColor: Qt.rgba(0,0,0.6,1)
                onSliderMove: { blueNumInput.text = val.toString(); colorWidget.blue = val; blueChanged(); }
            }
            NumberInput {
                id: blueNumInput
                numDigits: 3
                width: 40
                text: colorWidget.blue.toString()
                validator: IntValidator { bottom: 0; top: 255 }
                onValueChanged: { blueSlider.val = parseInt(blueNumInput.text); colorWidget.blue = blueSlider.val; blueSlider.updatePos(); blueChanged(); }
            }
        } // End Row

        Rectangle {
            id: mixer
            anchors.leftMargin: parent.width * 0.8 + 20
            anchors.left: parent.left
            width: blueNumInput.width
            height: blueNumInput.height
            radius: 3
            border.width: 2
            border.color: "#ffffff"
            color: Qt.rgba(colorWidget.red / 255,colorWidget.green /255,  colorWidget.blue / 255, 1)
        }

    }
}
