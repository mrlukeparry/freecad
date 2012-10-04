
import QtQuick 1.1
Item {
    id: appearancesTaskWidget
    height: 800
    width: 300
    anchors.fill: parent

    // Slots
    function openMaterialLibraryWidget()
    {
        materialLibraryLoader.source = "";
        materialLibraryLoader.source = "materialLibraryUi.qml";
        parametersUiLoader.visible = false;
        appearancesTaskWidget.state = "";
    }

    function openMaterialParametersWidget()
    {
        parametersUiLoader.source = "";
        parametersUiLoader.source = "materialParametersUi.qml";
        appearancesTaskWidget.state = "viewProperties";
    }

    // Signals
    signal materialDrag(string matId)
    signal materialPropsAccepted()
    signal materialLibraryCancel()
    signal materialPropsCancel()

    //Connections to Material Parameters Panel
    Connections {
             target: parametersUiLoader.item
             onCancel: appearancesTaskWidget.materialPropsCancel()
             onAccepted: appearancesTaskWidget.materialPropsAccepted()
    }

    //Connections to Material Library Panel
    Connections {
             target: materialLibraryLoader.item
             onCancel: appearancesTaskWidget.materialLibraryCancel()
    }

    Rectangle {
        width: 100
        height: 62
        color: "#403d3d"
        anchors.fill: parent

        Loader {
            id: materialLibraryLoader
            anchors.horizontalCenter: parent.horizontalCenter
            visible: true
        }

        Loader {
            id: parametersUiLoader
            anchors.horizontalCenter: parent.horizontalCenter
            visible: false
        }
    }
    transitions: Transition {
        //NumberAnimation {properties: "x";  duration: 400;}
    }

    states: [
        State {
        name: "viewProperties"
        PropertyChanges {target: materialLibraryLoader; visible: false}
        PropertyChanges {target: parametersUiLoader; visible: true}
        }
    ]
}
