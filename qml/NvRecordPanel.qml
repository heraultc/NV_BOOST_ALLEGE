import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs

NvGroupBox {
    id: root
    title: i18n.s.recTitle

    property bool running: false

    implicitHeight: col.implicitHeight + 30

    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 22 }
        spacing: 8

        // ── Dossier ────────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: i18n.s.recFolder
                color: "#888"; font.pixelSize: 12
                Layout.preferredWidth: 56
                elide: Text.ElideRight
            }
            TextField {
                id: dirEdit
                Layout.fillWidth: true
                text: "~"
                color: "#ddd"; font.pixelSize: 11
                padding: 5
                background: Rectangle {
                    color: "#1a1a1a"; radius: 4
                    border.color: dirEdit.activeFocus ? "#dd4814" : "#3a3a3a"
                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
            }
            NvButton { text: "…"; implicitWidth: 30; onClicked: dirDialog.open() }
        }

        // ── Nom de fichier ────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: i18n.s.recFile
                color: "#888"; font.pixelSize: 12
                Layout.preferredWidth: 56
                elide: Text.ElideRight
            }
            TextField {
                id: nameEdit
                Layout.fillWidth: true
                text: "enhanced.mp4"
                color: "#ddd"; font.pixelSize: 11
                padding: 5
                background: Rectangle {
                    color: "#1a1a1a"; radius: 4
                    border.color: nameEdit.activeFocus ? "#dd4814" : "#3a3a3a"
                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
            }
        }

        // ── Bouton Enregistrer ─────────────────────────────────────────────
        NvButton {
            text: backend.recording ? ("⏹  " + i18n.s.recStop)
                                    : ("⏺  " + i18n.s.recStart)
            active:  backend.recording
            danger:  backend.recording
            enabled: root.running
            Layout.fillWidth: true
            implicitHeight: 38

            onClicked: {
                if (backend.recording) {
                    backend.stopRecording()
                } else {
                    var dir  = dirEdit.text.trim()
                    var name = nameEdit.text.trim()
                    if (dir === "" || dir === "~") dir = "."
                    backend.startRecording(dir + "/" + name)
                }
            }
        }

        // ── Statut ─────────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 28
            radius: 4
            color: "#111"
            visible: backend.recording || backend.recFrames > 0

            Row {
                anchors.centerIn: parent
                spacing: 6

                // Point clignotant
                Rectangle {
                    width: 8; height: 8; radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: backend.recording ? "#ff4444" : "#00cc55"

                    SequentialAnimation on opacity {
                        running: backend.recording
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.2; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: backend.recording
                          ? (i18n.s.rec + "  " + backend.recFile)
                          : ("✓ " + i18n.fmt(i18n.s.recFramesDone,
                                             [backend.recFrames, backend.recFile]))
                    color: backend.recording ? "#ff6666" : "#44cc77"
                    font.pixelSize: 11
                    font.weight: backend.recording ? Font.DemiBold : Font.Normal
                }
            }
        }
    }

    FolderDialog {
        id: dirDialog
        title: i18n.s.dlgOutFolder
        onAccepted: dirEdit.text = selectedFolder.toString()
                      .replace(/^file:\/\/\//, Qt.platform.os === "windows" ? "" : "/")
    }
}
