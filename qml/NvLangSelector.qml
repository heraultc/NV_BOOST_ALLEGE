import QtQuick 2.15
import QtQuick.Controls 2.15

// NvLangSelector — sélecteur de langue compact, popup vers le haut
// Conçu pour vivre dans la barre de statut (hauteur 26 px).
Item {
    id: root

    implicitHeight: 20
    implicitWidth:  labelRow.width + 14

    // Nom lisible de la langue courante
    function currentName() {
        for (var i = 0; i < i18n.languages.length; ++i)
            if (i18n.languages[i].code === i18n.language)
                return i18n.languages[i].name
        return i18n.language
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 3
        color: mouse.containsMouse || popup.visible ? "#1e1e1e" : "transparent"
        border.width: 1
        border.color: popup.visible ? "#dd4814"
                    : mouse.containsMouse ? "#333" : "transparent"
        Behavior on color        { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }
    }

    Row {
        id: labelRow
        anchors.centerIn: parent
        spacing: 5

        // Petit globe
        Text {
            text: "\u2295"
            color: mouse.containsMouse || popup.visible ? "#dd4814" : "#555"
            font.pixelSize: 11
            anchors.verticalCenter: parent.verticalCenter
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        Text {
            text: root.currentName()
            color: mouse.containsMouse || popup.visible ? "#ccc" : "#666"
            font.pixelSize: 10
            font.letterSpacing: 0.3
            anchors.verticalCenter: parent.verticalCenter
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        // Chevron
        Text {
            text: "\u25B4"
            color: "#555"
            font.pixelSize: 8
            anchors.verticalCenter: parent.verticalCenter
            rotation: popup.visible ? 180 : 0
            Behavior on rotation { NumberAnimation { duration: 140 } }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: popup.visible ? popup.close() : popup.open()

        ToolTip.visible: containsMouse && !popup.visible
        ToolTip.text:    i18n.s.langLabel
        ToolTip.delay:   700
    }

    // ── Popup : s'ouvre vers le haut ───────────────────────────────────────
    Popup {
        id: popup
        y: -height - 4
        x: parent.width - width
        width:  150
        padding: 4
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#181818"
            radius: 5
            border.color: "#2e2e2e"
            border.width: 1
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 120 }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 100 }
        }

        contentItem: Column {
            spacing: 1

            Repeater {
                model: i18n.languages

                delegate: Rectangle {
                    width:  popup.availableWidth
                    height: 28
                    radius: 3
                    color: itemMouse.containsMouse ? "#262626"
                         : (modelData.code === i18n.language ? "#1f1f1f" : "transparent")
                    Behavior on color { ColorAnimation { duration: 100 } }

                    // Liseré orange sur la langue active
                    Rectangle {
                        width: 3; height: 14; radius: 1.5
                        color: "#dd4814"
                        visible: modelData.code === i18n.language
                        anchors { left: parent.left; leftMargin: 3
                                  verticalCenter: parent.verticalCenter }
                    }

                    Text {
                        anchors { left: parent.left; leftMargin: 14
                                  verticalCenter: parent.verticalCenter }
                        text: modelData.name
                        color: modelData.code === i18n.language ? "#e8e8e8" : "#9a9a9a"
                        font.pixelSize: 12
                    }

                    Text {
                        anchors { right: parent.right; rightMargin: 8
                                  verticalCenter: parent.verticalCenter }
                        text: modelData.flag
                        color: modelData.code === i18n.language ? "#dd4814" : "#4a4a4a"
                        font.pixelSize: 9
                        font.letterSpacing: 0.5
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            i18n.setLanguage(modelData.code)
                            popup.close()
                        }
                    }
                }
            }
        }
    }
}
