# NV-BOOST — Intégration QML ↔ VideoProcessor

## Structure du projet

```
NV-BOOST-QML/
├── NightVector.pro      ← fichier projet Qt
├── main.cpp             ← point d'entrée (QGuiApplication + QML engine)
├── MainWindow.qml       ← interface complète
├── qml.qrc              ← ressources QML
├── videoprocessor.h     ← COPIER depuis le projet original
├── videoprocessor.cpp   ← COPIER depuis le projet original
├── config.h             ← COPIER depuis le projet original
└── logoWhite.png        ← COPIER depuis le projet original
```

## Étapes d'intégration (3 étapes)

### 1 — Exposer VideoProcessor au QML dans main.cpp

```cpp
VideoProcessor* processor = new VideoProcessor(&app);
engine.rootContext()->setContextProperty("videoProcessor", processor);
```

### 2 — Recevoir les frames (VideoProcessor → QML)

Ajoutez une propriété Q_PROPERTY dans VideoProcessor pour exposer
l'image courante en tant que QImage ou URL de source :

```cpp
// Dans videoprocessor.h
Q_PROPERTY(QImage currentFrame READ currentFrame NOTIFY frameReady)
```

Côté QML, l'Image se met à jour via un ImageProvider C++ :

```cpp
// Dans main.cpp
engine.addImageProvider("frames", new VideoFrameProvider(processor));
```

```qml
// Dans MainWindow.qml — l'Image vidéo utilise :
source: "image://frames/current?" + frameCounter  // incrémenté à chaque frameReady
```

### 3 — Connecter les boutons QML → VideoProcessor

Dans MainWindow.qml, les onClicked sont déjà préparés avec des commentaires.
Remplacez les commentaires par des appels réels :

```qml
// Ouvrir source
onClicked: {
    if (state.isRunning) {
        videoProcessor.stopProcessing()
    } else {
        var src = ""
        if (state.sourceType === 0) src = cameraIndexField.text
        else if (state.sourceType === 1) src = filePathField.text
        else src = urlField.text
        videoProcessor.openSource(src)
        videoProcessor.start()
        state.isRunning = true
    }
}

// Connecter les signaux VideoProcessor → state QML
Connections {
    target: videoProcessor
    function onFrameReady(original, enhanced, fps, ms) {
        state.fps = fps
        state.ms  = ms
        frameCounter++   // force le rechargement de l'image
    }
    function onSourceInfo(info) { state.statusMsg = info }
    function onErrorOccurred(msg) { state.statusMsg = "⚠ " + msg }
    function onRecordingStarted(path) {
        state.isRecording = true
        state.recStatus   = "● REC " + path
        state.recOk       = false
    }
    function onRecordingStopped(path, frames) {
        state.isRecording = false
        state.recStatus   = "✓ " + frames + " frames → " + path
        state.recOk       = true
    }
}
```

### 4 — Transmettre la config au VideoProcessor

Ajoutez une fonction JS dans MainWindow.qml et appelez-la depuis
un `onChanged` sur les propriétés de `state` :

```qml
function syncConfig() {
    videoProcessor.setConfig(
        state.useStretch, state.useAgcwd, state.useMertens,
        state.useLime, state.useClahe, state.useSharpen,
        state.agcwdAlpha, state.claheClip, state.accumulation,
        state.showOverlay, state.showSideBySide
    )
}
```

Côté C++, exposez un slot :
```cpp
Q_INVOKABLE void setConfigFromQml(bool stretch, bool agcwd, bool mertens,
    bool lime, bool clahe, bool sharpen,
    double alpha, float clip, int accum, bool overlay, bool sideBySide);
```

## Dépendances

- Qt 6.x (Quick + QML)
- OpenCV 4.x (modules core/imgproc/videoio/photo/highgui)
