import QtQuick 2.15
import QtQuick.Layouts 1.15

NvGroupBox {
    id: root
    title: i18n.s.parTitle

    implicitHeight: col.implicitHeight + 30

    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 22 }
        spacing: 2

        // ── Pipeline rapide ─────────────────────────────────────────────────
        NvSlider {
            Layout.fillWidth: true
            label: i18n.s.parTDenoise
            from: 0.0; to: 0.95; value: backend.tdenoiseStrength
            stepSize: 0
            displayValue: value.toFixed(2)
            onSliderMoved: (v) => backend.tdenoiseStrength = v
        }
        NvSlider {
            Layout.fillWidth: true
            label: i18n.s.parAutoLut
            from: 0.0; to: 1.0; value: backend.autoLutStrength
            stepSize: 0
            displayValue: value.toFixed(2)
            onSliderMoved: (v) => backend.autoLutStrength = v
        }
        NvSlider {
            Layout.fillWidth: true
            label: i18n.s.parClaheClip
            from: 1.0; to: 8.0; value: backend.claheClip
            stepSize: 0
            displayValue: value.toFixed(1)
            onSliderMoved: (v) => backend.claheClip = v
        }
        NvSlider {
            Layout.fillWidth: true
            label: i18n.s.parLimeGamma
            from: 0.4; to: 0.9; value: backend.limeGamma
            stepSize: 0
            displayValue: value.toFixed(2)
            onSliderMoved: (v) => backend.limeGamma = v
        }

        // ── Affichage ──────────────────────────────────────────────────────
        Rectangle { height: 1; color: "#222"; Layout.fillWidth: true; Layout.topMargin: 4 }

        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.dispOverlay
            checked: backend.showOverlay
            onToggled: (v) => backend.showOverlay = v
        }
        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.dispSideBySide
            checked: backend.showSideBySide
            onToggled: (v) => backend.showSideBySide = v
        }
    }
}
