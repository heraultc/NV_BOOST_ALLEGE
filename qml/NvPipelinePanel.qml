import QtQuick 2.15
import QtQuick.Layouts 1.15

NvGroupBox {
    id: root
    title: i18n.s.pipeTitle

    implicitHeight: col.implicitHeight + 30

    ColumnLayout {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 22 }
        spacing: 2

        // ══ PIPELINE RAPIDE (recommandé) ═══════════════════════════════════
        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.pipeA
            tooltip: i18n.s.pipeATip
            checked: backend.useTDenoise
            onToggled: (v) => backend.useTDenoise = v
        }
        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.pipeB
            tooltip: i18n.s.pipeBTip
            checked: backend.useAutoLut
            onToggled: (v) => backend.useAutoLut = v
        }
        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.pipeE
            tooltip: i18n.s.pipeETip
            checked: backend.useClahe
            onToggled: (v) => backend.useClahe = v
        }
        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.pipeF
            tooltip: i18n.s.pipeFTip
            checked: backend.useSharpen
            onToggled: (v) => backend.useSharpen = v
        }

        Rectangle { height: 1; color: "#333"; Layout.fillWidth: true; Layout.topMargin: 6; Layout.bottomMargin: 4 }

        // ══ OPTIONNEL ══════════════════════════════════════════════════════
        NvToggle {
            Layout.fillWidth: true
            label:   i18n.s.pipeLime
            tooltip: i18n.s.pipeLimeTip
            checked: backend.useLime
            onToggled: (v) => backend.useLime = v
        }
    }
}
