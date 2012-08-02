import QtQuick 1.1
Item {

    id: previewWidget
    function updatePreview(text) {
        previewImage.source = "images/luxout.png"
        previewLoading = false;
        previewImage.source = text;
    }

    signal stopRender()
    signal saveOutput()

    width: 1600
    height: 1000
    property bool previewLoading: true

    Rectangle {
        id: rectangle1
        anchors.fill: parent
       gradient: Gradient {
           GradientStop { position: 0.0; color: "#333"}
                   GradientStop { position: 1.0; color: "#555" }
               }

      Loading {
          id: imageLoading
           anchors.centerIn:  rectangle1

           transitions: Transition {
               ColorAnimation { duration: 200 }
           }
        }

        PreviewImage {
            id: previewImage
            anchors.centerIn: parent;
        }

       Row {
           anchors.right: parent.right
           anchors.rightMargin: 30
           anchors.bottom: parent.bottom
           anchors.bottomMargin: 25
           spacing: 10

            Button {
                id: saveButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                text: qsTr("Save")
                enabled: false
                onClicked: previewWidget.saveOutput()

            }
            Button {
                id: stopButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                text: qsTr("Stop Render")
                onClicked: previewWidget.stopRender()
            }
       }
    }
    states: [
        State {name:"enabled"; when: !previewLoading;
            PropertyChanges { target: saveButton; enabled: true}
            PropertyChanges { target: imageLoading; visible: false}
    }]

}
