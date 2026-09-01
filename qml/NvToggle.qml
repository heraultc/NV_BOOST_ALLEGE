import QtQuick 2.15
import QtQuick.Controls 2.15

// NvToggle — checkbox avec indicateur pill orange
Item {
    id: root

    property bool   checked: false
    property string label:   ""
    property string tooltip: ""

    signal toggled(bool checked)

    implicitHeight: 28
    implicitWidth:  200

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.checked = !root.checked
            root.toggled(root.checked)
        }

        // Highlight hover row
        Rectangle {
            anchors.fill: parent
            radius: 4
            color: parent.containsMouse ? Qt.rgba(1,1,1,0.04) : "transparent"
            Behavior on color { ColorAnimation { duration: 100 } }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 4
            spacing: 10

            // Indicateur pill
            Rectangle {
                width: 32; height: 16; radius: 8
                anchors.verticalCenter: parent.verticalCenter
                color: root.checked ? "#dd4814" : "#333"
                border.color: root.checked ? "#dd4814" : "#555"
                border.width: 1
                Behavior on color { ColorAnimation { duration: 150 } }

                Rectangle {
                    id: knob
                    width: 12; height: 12; radius: 6
                    anchors.verticalCenter: parent.verticalCenter
                    x: root.checked ? parent.width - width - 2 : 2
                    color: root.checked ? "white" : "#888"
                    Behavior on x     { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                    Behavior on color { ColorAnimation  { duration: 150 } }
                }
            }

            Text {
                text: root.label
                color: root.checked ? "#e0e0e0" : "#909090"
                font.pixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: 150 } }
            }
        }

        ToolTip.visible: root.tooltip !== "" && containsMouse
        ToolTip.text:    root.tooltip
        ToolTip.delay:   600
    }
}
