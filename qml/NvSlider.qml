import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// NvSlider — slider fin avec label de valeur
Item {
    id: root

    property string label:        ""
    property real   from:         0
    property real   to:           1
    property real   value:        0
    property int    stepSize:     0
    property string displayValue: value.toFixed(2)
    // Largeur réservée au libellé — élargie pour absorber les traductions
    // plus longues (es/pt) ; le texte tronqué reste lisible via le tooltip.
    property int    labelWidth:   104

    // Signal renommé pour éviter le conflit avec valueChanged de Slider
    signal sliderMoved(real v)

    implicitHeight: 36
    implicitWidth:  200

    RowLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            id: labelText
            text: root.label
            color: "#b0b0b0"
            font.pixelSize: 12
            Layout.preferredWidth: root.labelWidth
            elide: Text.ElideRight

            // Libellé complet au survol lorsqu'il est tronqué
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
                ToolTip.visible: containsMouse && labelText.truncated
                ToolTip.text:    root.label
                ToolTip.delay:   500
            }
        }

        Slider {
            id: sl
            Layout.fillWidth: true
            from:     root.from
            to:       root.to
            value:    root.value
            stepSize: root.stepSize

            onValueChanged: root.sliderMoved(value)

            background: Item {
                implicitHeight: 16

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: 2
                    radius: 1
                    color: "#2a2a2a"
                }
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: sl.visualPosition * parent.width
                    height: 2
                    radius: 1
                    color: "#dd4814"
                    Behavior on width { NumberAnimation { duration: 60 } }
                }
            }

            handle: Rectangle {
                x: sl.leftPadding + sl.visualPosition * (sl.availableWidth - width)
                y: sl.topPadding  + sl.availableHeight / 2 - height / 2
                width:  12
                height: 12
                radius: 6
                color: sl.pressed ? "#ff6030" : "#dd4814"
                border.color: sl.hovered || sl.pressed ? Qt.rgba(1,1,1,0.25) : "transparent"
                border.width: 2
                Behavior on color { ColorAnimation { duration: 80 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: 20; height: 20; radius: 10
                    color: Qt.rgba(0xdd/255, 0x48/255, 0x14/255, sl.hovered ? 0.18 : 0)
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }
        }

        Text {
            text: root.displayValue
            color: "#dd4814"
            font.pixelSize: 12
            Layout.preferredWidth: 36
            horizontalAlignment: Text.AlignRight
        }
    }
}
