import QtQuick 2.9
import Cube 1.0

Item {
    visible: true
    width: 640
    height: 480
    Rectangle{
        anchors.fill: parent;
        anchors.margins: 10
        color: Qt.rgba(1,1,1,1)
        Image {
            id: background
            visible: true
            source: "qrc:/texture2.jpg"
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            z:0
        }
        Cube{
            anchors.fill: parent
        }
        Rectangle{
            visible: true
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            radius: 10
            border.color: Qt.rgba(1,1,1,1)
            color: Qt.rgba(1,1,1,0.75)
            height: parent.height * 0.35
            Text {
                anchors.fill: parent
                anchors.margins: 10
                text: qsTr("Test")
            }
        }
    }
}
