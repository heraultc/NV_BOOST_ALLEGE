import QtQuick 2.15

// NvGroupBox — panneau avec titre et séparateur orange subtil
Item {
    id: root
    property string title: ""
    default property alias content: contentArea.data

    implicitWidth:  200
    implicitHeight: header.height + contentArea.implicitHeight + 16

    // Titre
    Row {
        id: header
        spacing: 8
        anchors { left: parent.left; right: parent.right; top: parent.top }
        anchors.leftMargin: 2

        // Barre accent orange
        Rectangle {
            width: 3; height: 14
            radius: 1.5
            color: "#dd4814"
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.title
            color: "#c0c0c0"
            font.pixelSize: 11
            font.weight: Font.DemiBold
            font.letterSpacing: 0.8
            anchors.verticalCenter: parent.verticalCenter
        }

        // Ligne séparatrice
        Rectangle {
            height: 1
            color: "#2a2a2a"
            anchors.verticalCenter: parent.verticalCenter
            width: root.width - parent.children[0].width
                               - parent.children[1].width
                               - 2 * parent.spacing - 2
        }
    }

    // Contenu
    Item {
        id: contentArea
        anchors {
            top: header.bottom; topMargin: 8
            left: parent.left;  leftMargin: 4
            right: parent.right
        }
        implicitHeight: childrenRect.height
    }
}
