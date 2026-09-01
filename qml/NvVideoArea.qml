import QtQuick 2.15
import QtQuick.Layouts 1.15

// NvVideoArea — zone d'affichage vidéo avec placeholder élégant
Item {
    id: root

    property int  frameRevision: 0   // incrémenté à chaque frame

    // Placeholder quand pas de source
    property bool hasSource: backend.running

    // Image de frame (rechargée via source)
    Image {
        id: videoImg
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: root.hasSource ? ("image://frames/frame?" + root.frameRevision) : ""
        cache: false
        visible: root.hasSource
        asynchronous: false
    }

    // Placeholder élégant
    Rectangle {
        anchors.fill: parent
        color: "#0d0d0d"
        visible: !root.hasSource

        Column {
            anchors.centerIn: parent
            spacing: 16

            // Icône caméra stylée
            Item {
                width: 80; height: 80
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    anchors.fill: parent
                    radius: 40
                    color: "#1a1a1a"
                    border.color: "#2a2a2a"
                    border.width: 1
                }

                Text {
                    anchors.centerIn: parent
                    text: "⬡"
                    font.pixelSize: 36
                    color: "#dd4814"
                    opacity: 0.6
                }

                // Anneau pulsant
                Rectangle {
                    anchors.centerIn: parent
                    width: 64; height: 64; radius: 32
                    color: "transparent"
                    border.color: "#dd4814"
                    border.width: 1
                    opacity: 0

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.35; duration: 1200; easing.type: Easing.OutSine }
                        NumberAnimation { to: 0;    duration: 1200; easing.type: Easing.InSine  }
                    }
                    SequentialAnimation on scale {
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.3; duration: 1200; easing.type: Easing.OutSine }
                        NumberAnimation { to: 1.0; duration: 1200; easing.type: Easing.InSine  }
                    }
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: i18n.s.videoNoSource
                color: "#555"
                font.pixelSize: 15
                font.weight: Font.Light
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: i18n.s.videoHint
                color: "#3a3a3a"
                font.pixelSize: 12
            }
        }
    }

    // Compteur FPS en overlay (coin supérieur gauche, discret)
    Rectangle {
        visible: root.hasSource
        anchors { top: parent.top; left: parent.left; margins: 10 }
        width: fpsRow.width + 16; height: 24; radius: 4
        color: Qt.rgba(0,0,0,0.55)

        Row {
            id: fpsRow
            anchors.centerIn: parent
            spacing: 6
            Rectangle { width: 6; height: 6; radius: 3; color: "#dd4814"; anchors.verticalCenter: parent.verticalCenter }
            Text {
                text: backend.fps + " " + i18n.s.fps + "  ·  " + backend.latencyMs + " ms"
                color: "#ccc"
                font.pixelSize: 11
                font.family: "Monospace"
            }
        }
    }
}
