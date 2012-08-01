import QtQuick 1.1

Rectangle {
    function updatePreview(text) { picture.src = ""; picture.src = text; }

   gradient: Gradient {
       GradientStop { position: 0.0; color: "#333"}
               GradientStop { position: 1.0; color: "#555" }
           }
    // Create a flickable to view a large image.
    Flickable {

        id: view
        anchors.fill: parent
        contentWidth: picture.myWidth
        contentHeight: picture.myHeight

       PreviewImage {
           id: picture
           x: (view.width - width) / 2
           y: (view.height - height) / 2
       }
        // Only show the scrollbars when the view is moving.
        states: State {
            name: "ShowBars"
            when: (view.movingVertically || view.movingHorizontally)
            PropertyChanges { target: verticalScrollBar; opacity: 1 }
            PropertyChanges { target: horizontalScrollBar; opacity: 1 }
        }

        transitions: Transition {
            NumberAnimation { properties: "opacity"; duration: 400 }
        }
    }



    // Attach scrollbars to the right and bottom edges of the view.
    ScrollBar {
        id: verticalScrollBar
        width: 12; height: view.height-12
        anchors.right: view.right
        opacity: 0
        orientation: Qt.Vertical
        position: view.visibleArea.yPosition
        pageSize: view.visibleArea.heightRatio
    }

    ScrollBar {
        id: horizontalScrollBar
        width: view.width-12; height: 12
        anchors.bottom: view.bottom
        opacity: 0
        orientation: Qt.Horizontal
        position: view.visibleArea.xPosition
        pageSize: view.visibleArea.widthRatio
    }
}
