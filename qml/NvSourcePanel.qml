import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs

NvGroupBox {
    id: root
    title: i18n.s.srcTitle

    property int  sourceType: 0   // 0=Camera 1=Fichier 2=URL
    property bool running: false

    signal openRequested(string src)
    signal closeRequested()
    signal pauseRequested()
    signal restartRequested()

    // Expose isPaused from backend
    property bool paused: backend.paused

    implicitHeight: col.implicitHeight + 30

    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 22 }
        spacing: 10

        // ── Sélecteur de type ──────────────────────────────────────────────
        Row {
            spacing: 4
            Layout.fillWidth: true

            NvRadioButton {
                label: i18n.s.srcCamera
                checked: root.sourceType === 0
                onClicked: root.sourceType = 0
            }
            NvRadioButton {
                label: i18n.s.srcFile
                checked: root.sourceType === 1
                onClicked: root.sourceType = 1
            }
            NvRadioButton {
                label: i18n.s.srcUrl
                checked: root.sourceType === 2
                onClicked: root.sourceType = 2
            }
        }

        // ── Input dynamique ────────────────────────────────────────────────
        StackLayout {
            currentIndex: root.sourceType
            Layout.fillWidth: true

            // Caméra
            RowLayout {
                spacing: 6
                Text { text: i18n.s.srcIndex; color: "#888"; font.pixelSize: 12 }
                TextField {
                    id: cameraEdit
                    text: "0"
                    placeholderText: i18n.s.srcCamPh
                    implicitWidth: 60
                    color: "#ddd"
                    font.pixelSize: 12
                    background: Rectangle {
                        color: "#1a1a1a"; radius: 4
                        border.color: cameraEdit.activeFocus ? "#dd4814" : "#3a3a3a"
                        Behavior on border.color { ColorAnimation { duration: 120 } }
                    }
                    padding: 6
                }
                Item { Layout.fillWidth: true }
            }

            // Fichier
            RowLayout {
                spacing: 4
                TextField {
                    id: fileEdit
                    Layout.fillWidth: true
                    placeholderText: i18n.s.srcFilePh
                    color: "#ddd"
                    font.pixelSize: 12
                    background: Rectangle {
                        color: "#1a1a1a"; radius: 4
                        border.color: fileEdit.activeFocus ? "#dd4814" : "#3a3a3a"
                        Behavior on border.color { ColorAnimation { duration: 120 } }
                    }
                    padding: 6
                }
                NvButton {
                    text: "…"; implicitWidth: 30
                    onClicked: fileDialog.open()
                }
            }

            // URL
            TextField {
                id: urlEdit
                placeholderText: i18n.s.srcUrlPh
                color: "#ddd"
                font.pixelSize: 12
                background: Rectangle {
                    color: "#1a1a1a"; radius: 4
                    border.color: urlEdit.activeFocus ? "#dd4814" : "#3a3a3a"
                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }
                padding: 6
            }
        }

        // ── Boutons lecture ────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            NvButton {
                text: root.running ? ("⏹  " + i18n.s.btnClose)
                                   : ("▶  " + i18n.s.btnOpen)
                active: root.running
                danger: root.running
                Layout.fillWidth: true
                onClicked: {
                    if (root.running) {
                        root.closeRequested()
                    } else {
                        var src = ""
                        if (root.sourceType === 0) src = cameraEdit.text.trim()
                        else if (root.sourceType === 1) src = fileEdit.text.trim()
                        else src = urlEdit.text.trim()
                        root.openRequested(src)
                    }
                }
            }

            NvButton {
                text: root.paused ? ("▶  " + i18n.s.btnResume)
                                  : ("⏸  " + i18n.s.btnPause)
                active: root.paused
                enabled: root.running
                Layout.fillWidth: true
                onClicked: root.pauseRequested()
            }

            NvButton {
                text: "↺"
                implicitWidth: 34
                enabled: root.running
                ToolTip.visible: hovered; ToolTip.text: i18n.s.tipRestart
                onClicked: root.restartRequested()
            }
        }
    }

    // File dialog
    FileDialog {
        id: fileDialog
        title: i18n.s.dlgOpenVideo
        nameFilters: [ i18n.s.filterVideos + " (*.mp4 *.avi *.mkv *.mov *.wmv *.flv *.webm *.ts)",
                       i18n.s.filterAll + " (*.*)" ]
        onAccepted: fileEdit.text = selectedFile.toString()
                      .replace(/^file:\/\/\//, Qt.platform.os === "windows" ? "" : "/")
    }
}
