import QtQuick 2.15
import QtQuick.Layouts 1.15

// NvStatusBar — barre de statut inférieure (+ sélecteur de langue à droite)
Rectangle {
    height: 26
    color: "#111"

    Rectangle { width: parent.width; height: 1; color: "#222"; anchors.top: parent.top }

    RowLayout {
        anchors { fill: parent; leftMargin: 12; rightMargin: 8 }
        spacing: 8

        // Indicateur état
        Rectangle {
            width: 6; height: 6; radius: 3
            color: backend.running
                   ? (backend.paused ? "#dd8814" : "#44cc77")
                   : "#444"
            Behavior on color { ColorAnimation { duration: 300 } }
        }

        Text {
            Layout.fillWidth: true
            text: backend.statusMsg
            color: backend.statusMsg.startsWith("⚠") ? "#ff8844" : "#888"
            font.pixelSize: 11
            elide: Text.ElideRight
            Behavior on color { ColorAnimation { duration: 200 } }
        }

        // Version
        Text {
            text: "NV-BOOST  v2.0"
            color: "#333"
            font.pixelSize: 10
            font.letterSpacing: 0.5
        }

        Rectangle { width: 1; height: 12; color: "#252525" }

        // ── Sélecteur de langue (coin inférieur droit) ─────────────────────
        NvLangSelector {
            Layout.alignment: Qt.AlignVCenter
        }
    }
}
