import QtQuick 2.15
import QtQuick.Controls 2.15

// NvButton — bouton pro NV-BOOST
// active : true = liseret orange épais
// danger : true = teinte rouge
Button {
    id: root

    property bool  active   : false
    property bool  danger   : false
    property color baseColor: danger ? "#5a1515" : "#2a2a2a"
    property color accentColor: "#dd4814"

    implicitHeight: 34
    font.pixelSize: 13
    font.weight: Font.Medium

    background: Rectangle {
        radius: 5
        color: root.pressed ? Qt.darker(root.baseColor, 1.3)
             : root.hovered ? Qt.lighter(root.baseColor, 1.25)
             : root.baseColor

        border.width: root.active ? 2 : 1
        border.color: root.active  ? root.accentColor
                    : root.hovered ? Qt.rgba(root.accentColor.r,
                                             root.accentColor.g,
                                             root.accentColor.b, 0.55)
                    : "#444"

        Behavior on border.color { ColorAnimation { duration: 120 } }
        Behavior on color         { ColorAnimation { duration: 100 } }

        // Subtle glow quand actif
        Rectangle {
            visible: root.active
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(0xdd/255, 0x48/255, 0x14/255, 0.25)
            anchors.margins: -3
        }
    }

    contentItem: Text {
        text: root.text
        font: root.font
        color: root.enabled ? "#e8e8e8" : "#666"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment:   Text.AlignVCenter
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
