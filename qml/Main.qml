import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    title: "NV-BOOST"
    width: 1280; height: 780
    minimumWidth: 1100; minimumHeight: 650
    color: "#111"
    visible: true

    // Réception de frames depuis le notifier C++
    property int frameRevision: 0

    Connections {
        target: notifier
        function onFrameChanged() { root.frameRevision++ }
    }

    // ══════════════════════════════════════════════════════════════════════
    //  Layout principal : zone vidéo | panneau de contrôle
    // ══════════════════════════════════════════════════════════════════════
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Barre de titre custom ──────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 58
            color: "#161616"

            Rectangle { width: parent.width; height: 1; color: "#222"; anchors.bottom: parent.bottom }

            RowLayout {
                anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
                spacing: 10

                // Logo image
                Image {
                    id: logo
                    // Dans un RowLayout, width/height sont ignorés :
                    // il faut Layout.preferredWidth/Height.
                    property int logoHeight: 26        // ← ajuster ici
                    source: "qrc:/logoWhite.png"
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredHeight: logoHeight
                    Layout.preferredWidth: logoHeight * sourceSize.width / sourceSize.height
                    mipmap: true
                    opacity: 0.9
                }

                Text {
                    text: "NV-BOOST"
                    color: "#e0e0e0"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.5
                }

                Text {
                    text: i18n.s.appSubtitle
                    color: "#444"
                    font.pixelSize: 11
                    Layout.leftMargin: 4
                }

                Item { Layout.fillWidth: true }

                // Indicateur REC clignotant dans la titlebar
                Row {
                    spacing: 6
                    visible: backend.recording

                    Rectangle {
                        width: 7; height: 7; radius: 3.5
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#ff4444"
                        SequentialAnimation on opacity {
                            running: backend.recording; loops: Animation.Infinite
                            NumberAnimation { to: 0.2; duration: 600 }
                            NumberAnimation { to: 1.0; duration: 600 }
                        }
                    }
                    Text {
                        text: i18n.s.rec
                        color: "#ff6666"
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                // FPS badge
                Rectangle {
                    visible: backend.running
                    width: fpsBadge.width + 16; height: 22; radius: 4
                    color: "#1a1a1a"
                    border.color: "#2a2a2a"

                    Text {
                        id: fpsBadge
                        anchors.centerIn: parent
                        text: backend.fps + " " + i18n.s.fps
                        color: backend.fps > 20 ? "#44cc77"
                             : backend.fps > 10 ? "#ddaa33"
                             : "#cc4444"
                        font.pixelSize: 11
                        font.family: "Monospace"
                        Behavior on color { ColorAnimation { duration: 300 } }
                    }
                }

            }
        }

        // ── Corps : vidéo + panneau ────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ── Zone vidéo ─────────────────────────────────────────────────
            NvVideoArea {
                id: videoArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                frameRevision: root.frameRevision
            }

            // Séparateur vertical subtil
            Rectangle {
                width: 1
                Layout.fillHeight: true
                color: "#1e1e1e"
            }

            // ── Panneau de contrôle (scrollable) ───────────────────────────
            Rectangle {
                width: 290
                Layout.fillHeight: true
                color: "#141414"

                ScrollView {
                    anchors.fill: parent
                    clip: true
                    contentWidth: availableWidth
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        width: 4
                        background: Rectangle { color: "transparent" }
                        contentItem: Rectangle {
                            radius: 2
                            color: "#333"
                        }
                    }

                    ColumnLayout {
                        width: 290
                        spacing: 0

                        // Padding top
                        Item { height: 12 }

                        // ── Source ─────────────────────────────────────────
                        NvSourcePanel {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10; Layout.rightMargin: 10
                            running: backend.running

                            onOpenRequested:    (src) => backend.openSource(src)
                            onCloseRequested:   () => backend.closeSource()
                            onPauseRequested:   () => backend.togglePause()
                            onRestartRequested: () => backend.restart()
                        }

                        Divider {}

                        // ── Pipeline ───────────────────────────────────────
                        NvPipelinePanel {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10; Layout.rightMargin: 10
                        }

                        Divider {}

                        // ── Paramètres + Affichage ─────────────────────────
                        NvParamsPanel {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10; Layout.rightMargin: 10
                        }

                        Divider {}

                        // ── Enregistrement ─────────────────────────────────
                        NvRecordPanel {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10; Layout.rightMargin: 10
                            running: backend.running
                        }

                        // ── Perf mini ──────────────────────────────────────
                        Divider {}

                        NvGroupBox {
                            title: i18n.s.perfTitle
                            Layout.fillWidth: true
                            Layout.leftMargin: 10; Layout.rightMargin: 10
                            implicitHeight: perfRow.implicitHeight + 30

                            RowLayout {
                                id: perfRow
                                anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 22 }
                                spacing: 0

                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text { text: i18n.s.perfFps; color: "#555"; font.pixelSize: 10; font.letterSpacing: 0.5 }
                                    Text {
                                        text: backend.running ? backend.fps : "—"
                                        color: backend.fps > 20 ? "#44cc77"
                                             : backend.fps > 10 ? "#ddaa33" : "#cc4444"
                                        font.pixelSize: 20
                                        font.weight: Font.Light
                                        font.family: "Monospace"
                                        Behavior on color { ColorAnimation { duration: 300 } }
                                    }
                                }

                                Rectangle { width: 1; height: 36; color: "#222"; Layout.alignment: Qt.AlignVCenter }

                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Layout.leftMargin: 12
                                    Text { text: i18n.s.perfLatency; color: "#555"; font.pixelSize: 10; font.letterSpacing: 0.5 }
                                    Text {
                                        text: backend.running ? (backend.latencyMs + " ms") : "—"
                                        color: "#888"
                                        font.pixelSize: 20
                                        font.weight: Font.Light
                                        font.family: "Monospace"
                                    }
                                }
                            }
                        }

                        Item { height: 16 }
                    }
                }
            }
        }

        // ── Status bar ─────────────────────────────────────────────────────
        NvStatusBar {
            Layout.fillWidth: true
        }
    }

    // Composant inline : séparateur entre sections
    component Divider: Rectangle {
        Layout.fillWidth: true
        height: 1
        color: "#1a1a1a"
        Layout.topMargin: 8
        Layout.bottomMargin: 8
        Layout.leftMargin: 10
        Layout.rightMargin: 10
    }
}
