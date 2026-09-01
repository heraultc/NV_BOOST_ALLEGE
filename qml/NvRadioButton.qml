import QtQuick 2.15

// NvRadioButton — radio avec uniquement le point central en orange
Item {
    id: root

    property bool   checked: false
    property string label:   ""

    signal clicked()

    implicitHeight: 28
    // Largeur dictée par le libellé traduit (et non figée à 80 px)
    implicitWidth:  Math.max(64, contentRow.implicitWidth + 14)

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: parent.containsMouse ? Qt.rgba(1,1,1,0.04) : "transparent"
            Behavior on color { ColorAnimation { duration: 100 } }
        }

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: 7

            // Indicateur radio : anneau gris + point orange si checked
            Item {
                width: 16; height: 16
                anchors.verticalCenter: parent.verticalCenter

                // Anneau extérieur
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "transparent"
                    border.width: 1.5
                    border.color: root.checked ? "#dd4814" : (parent.parent.parent.containsMouse ? "#888" : "#555")
                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }

                // Point central — visible uniquement si checked
                Rectangle {
                    anchors.centerIn: parent
                    width:  7; height: 7; radius: 3.5
                    color: "#dd4814"
                    opacity: root.checked ? 1 : 0
                    scale:   root.checked ? 1 : 0.3
                    Behavior on opacity { NumberAnimation { duration: 150 } }
                    Behavior on scale   { NumberAnimation { duration: 150; easing.type: Easing.OutBack } }
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
    }
}
