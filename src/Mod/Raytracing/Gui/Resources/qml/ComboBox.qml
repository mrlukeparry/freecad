// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
        id:comboBox
        property alias model: listView.model
        property alias delegate: listView.delegate
        property alias selectedItem: chosenItemText.text;
        property alias selectedIndex: listView.currentIndex;
        signal comboClicked;
        width: 250;
        height: 30;
        z: 100;

        onFocusChanged: function() { if (!comboBox.focus){ comboBox.state = ""} }()
        Rectangle {

            id:chosenItem
            radius: 5
            border.width: 2
            border.color: "#ffffff"
            color: "#111111"
            clip: true
            width: parent.width
            height:parent.height;

            Text {
                id:chosenItemText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 3
                text:comboBox.model[0];
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 10;
                color: "#fff"
            }

            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    comboBox.state = comboBox.state=== "dropDown" ? "" : "dropDown"
                    comboBox.focus = true;
                }
            }



            ListView {

                id:listView
                height:150;
                clip: true
                width: parent.width
                anchors.left: parent.left
                anchors.topMargin: 10
                anchors.top: chosenItemText.bottom
                snapMode: ListView.SnapToItem
                model: []
                flickDeceleration: 7000
                delegate: Item {
                    id: defaultDelegate
                    width:parent.width;
                    height: label.height + 10

                    Text {
                        id: label
                        text: modelData
                        anchors.verticalCenter: parent.center
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        color: "#fff"
                    }
                    MouseArea {
                        anchors.fill: parent;
                        onClicked: {
                            comboBox.state = ""
                            var prevSelection = chosenItemText.text
                            chosenItemText.text = modelData
                            if(chosenItemText.text != prevSelection){
                                comboBox.comboClicked();
                            }
                            listView.currentIndex = index;
                        }
                    }
                }

                ScrollBar {
                    flickable: parent
                    vertical: true
                    color: "#fff"
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                }
            }
        }




        Component {
            id: highlight
            Rectangle {
                width:comboBox.width;
                height:comboBox.height;
                color: "#555";
            }
        }

        states: State {
            name: "dropDown";
            PropertyChanges { target: comboBox; height: 40 + listView.height }
        }

        transitions: Transition {
            NumberAnimation { target: comboBox; properties: "height"; easing.type: Easing.OutExpo; duration: 600 }
        }
}
