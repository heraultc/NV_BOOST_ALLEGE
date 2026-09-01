#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QUrl>
#include <QStringList>
#include "videoprocessor.h"
#include "config.h"

//===========================================================================//
//  Backend — pont QML ↔ VideoProcessor
//  Exposé au moteur QML via QML_ELEMENT / setContextProperty
//===========================================================================//
class Backend : public QObject
{
    Q_OBJECT

    // ── États ────────────────────────────────────────────────────────────────
    Q_PROPERTY(bool   running     READ isRunning     NOTIFY runningChanged)
    Q_PROPERTY(bool   paused      READ isPaused      NOTIFY pausedChanged)
    Q_PROPERTY(bool   recording   READ isRecording   NOTIFY recordingChanged)
    Q_PROPERTY(int    fps         READ fps           NOTIFY statsChanged)
    Q_PROPERTY(qint64   latencyMs   READ latencyMs     NOTIFY statsChanged)
    Q_PROPERTY(QString statusMsg  READ statusMsg     NOTIFY statusChanged)
    Q_PROPERTY(QString recFile    READ recFile       NOTIFY recordingChanged)
    Q_PROPERTY(int    recFrames   READ recFrames     NOTIFY recordingChanged)

    // ── Pipeline rapide (v2) ──────────────────────────────────────────────────
    Q_PROPERTY(bool useTDenoise      READ useTDenoise      WRITE setUseTDenoise      NOTIFY configChanged)
    Q_PROPERTY(bool useAutoLut       READ useAutoLut       WRITE setUseAutoLut       NOTIFY configChanged)
    Q_PROPERTY(double autoLutStrength  READ autoLutStrength  WRITE setAutoLutStrength  NOTIFY configChanged)
    Q_PROPERTY(double tdenoiseStrength READ tdenoiseStrength WRITE setTdenoiseStrength NOTIFY configChanged)
    Q_PROPERTY(double limeGamma        READ limeGamma        WRITE setLimeGamma        NOTIFY configChanged)

    // ── Pipeline toggles ──────────────────────────────────────────────────────
    Q_PROPERTY(bool useStretch  READ useStretch  WRITE setUseStretch  NOTIFY configChanged)
    Q_PROPERTY(bool useAgcwd    READ useAgcwd    WRITE setUseAgcwd    NOTIFY configChanged)
    Q_PROPERTY(bool useMertens  READ useMertens  WRITE setUseMertens  NOTIFY configChanged)
    Q_PROPERTY(bool useLime     READ useLime     WRITE setUseLime     NOTIFY configChanged)
    Q_PROPERTY(bool useClahe    READ useClahe    WRITE setUseClahe    NOTIFY configChanged)
    Q_PROPERTY(bool useSharpen  READ useSharpen  WRITE setUseSharpen  NOTIFY configChanged)

    // ── Paramètres ────────────────────────────────────────────────────────────
    Q_PROPERTY(double agcwdAlpha  READ agcwdAlpha  WRITE setAgcwdAlpha  NOTIFY configChanged)
    Q_PROPERTY(double claheClip   READ claheClip   WRITE setClaheClip   NOTIFY configChanged)
    Q_PROPERTY(int    accumulation READ accumulation WRITE setAccumulation NOTIFY configChanged)

    // ── Affichage ─────────────────────────────────────────────────────────────
    Q_PROPERTY(bool showOverlay   READ showOverlay   WRITE setShowOverlay   NOTIFY configChanged)
    Q_PROPERTY(bool showSideBySide READ showSideBySide WRITE setShowSideBySide NOTIFY configChanged)

public:
    explicit Backend(QObject* parent = nullptr);
    ~Backend() override;

    // Propriétés read
    bool    isRunning()    const { return m_running; }
    bool    isPaused()     const { return m_processor ? m_processor->isPaused() : false; }
    bool    isRecording()  const { return m_processor ? m_processor->isRecording() : false; }
    int     fps()          const { return m_fps; }
    qint64  latencyMs()    const { return m_latencyMs; }
    QString statusMsg()    const;   // résolu à la volée dans la langue courante
    QString recFile()      const { return m_recFile; }
    int     recFrames()    const { return m_recFrames; }

    bool   useTDenoise()      const { return m_cfg.use_tdenoise; }
    bool   useAutoLut()       const { return m_cfg.use_autolut; }
    double autoLutStrength()  const { return m_cfg.autolut_strength; }
    double tdenoiseStrength() const { return m_cfg.tdenoise_strength; }
    double limeGamma()        const { return m_cfg.lime_gamma; }

    bool   useStretch()   const { return m_cfg.use_histstretch; }
    bool   useAgcwd()     const { return m_cfg.use_agcwd; }
    bool   useMertens()   const { return m_cfg.use_mertens; }
    bool   useLime()      const { return m_cfg.use_lime; }
    bool   useClahe()     const { return m_cfg.use_clahe; }
    bool   useSharpen()   const { return m_cfg.use_sharpen; }
    double agcwdAlpha()   const { return m_cfg.agcwd_alpha; }
    double claheClip()    const { return m_cfg.clahe_clip; }
    int    accumulation() const { return m_cfg.accumulation; }
    bool   showOverlay()   const { return m_cfg.show_overlay; }
    bool   showSideBySide() const { return m_cfg.show_side_by_side; }

    // Setters pipeline v2
    void setUseTDenoise(bool v)      { m_cfg.use_tdenoise      = v; pushConfig(); emit configChanged(); }
    void setUseAutoLut(bool v)       { m_cfg.use_autolut       = v; pushConfig(); emit configChanged(); }
    void setAutoLutStrength(double v){ m_cfg.autolut_strength  = v; pushConfig(); emit configChanged(); }
    void setTdenoiseStrength(double v){ m_cfg.tdenoise_strength = v; pushConfig(); emit configChanged(); }
    void setLimeGamma(double v)      { m_cfg.lime_gamma        = v; pushConfig(); emit configChanged(); }

    // Setters pipeline
    void setUseStretch(bool v)  { m_cfg.use_histstretch  = v; pushConfig(); emit configChanged(); }
    void setUseAgcwd(bool v)    { m_cfg.use_agcwd        = v; pushConfig(); emit configChanged(); }
    void setUseMertens(bool v)  { m_cfg.use_mertens      = v; pushConfig(); emit configChanged(); }
    void setUseLime(bool v)     { m_cfg.use_lime         = v; pushConfig(); emit configChanged(); }
    void setUseClahe(bool v)    { m_cfg.use_clahe        = v; pushConfig(); emit configChanged(); }
    void setUseSharpen(bool v)  { m_cfg.use_sharpen      = v; pushConfig(); emit configChanged(); }
    void setAgcwdAlpha(double v){ m_cfg.agcwd_alpha      = v; pushConfig(); emit configChanged(); }
    void setClaheClip(double v) { m_cfg.clahe_clip       = (float)v; pushConfig(); emit configChanged(); }
    void setAccumulation(int v) { m_cfg.accumulation     = v; pushConfig(); emit configChanged(); }
    void setShowOverlay(bool v)    { m_cfg.show_overlay      = v; emit configChanged(); }
    void setShowSideBySide(bool v) { m_cfg.show_side_by_side = v; emit configChanged(); }

public slots:
    // Source
    void openSource(const QString& source);
    void closeSource();
    void togglePause();
    void restart();

    // Enregistrement
    void startRecording(const QString& outputPath);
    void stopRecording();

signals:
    void runningChanged();
    void pausedChanged();
    void recordingChanged();
    void statsChanged();
    void statusChanged();
    void configChanged();

    // Frames — transmises au QML via image provider
    void frameUpdated(QImage original, QImage enhanced, bool sideBySide, bool overlay,
                      int fps, qint64 ms, bool rec);

private slots:
    void onFrameReady(QImage original, QImage enhanced, int fps, qint64 ms);
    void onSourceInfo(QString typeKey, QString source, int w, int h, double fps);
    void onError(QString key, QStringList args);
    void onRecordingStarted(QString path);
    void onRecordingStopped(QString path, int frames);

private:
    void pushConfig();
    // On mémorise la CLÉ et ses arguments, jamais le texte final : le statut
    // affiché suit donc la langue même s'il a été produit avant le changement.
    void setStatus(const QString& key,
                   const QStringList& args = {},
                   bool warning = false);

    VideoProcessor* m_processor{nullptr};
    Config          m_cfg;

    bool    m_running{false};
    int     m_fps{0};
    qint64  m_latencyMs{0};
    QString     m_statusKey{"stReady"};
    QStringList m_statusArgs;
    bool        m_statusWarn{false};
    QString m_recFile;
    int     m_recFrames{0};
};
