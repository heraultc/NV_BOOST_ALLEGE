#pragma once

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QElapsedTimer>

#include <deque>
#include <atomic>

#include <opencv2/opencv.hpp>

#include "config.h"

//===========================================================================//
//  VideoProcessor — thread de capture + traitement + enregistrement OpenCV
//
//  v2 « FAST » : pipeline temps-réel pour rehaussement nocturne.
//  Principe directeur : tout ce qui est une STATISTIQUE ou une CARTE LISSE
//  (histogramme, gamma, carte d'illumination, coefficients de guided filter)
//  se calcule en BASSE RÉSOLUTION puis s'applique
//  en pleine résolution via LUT / upsampling. Seules les opérations
//  élément-par-élément touchent le plein cadre.
//===========================================================================//
class VideoProcessor : public QThread
{
    Q_OBJECT

public:
    explicit VideoProcessor(QObject* parent = nullptr);
    ~VideoProcessor() override;

    // ── Source ────────────────────────────────────────────────────────────
    bool openSource(const QString& source);
    void stopProcessing();
    void setPaused(bool paused);
    bool isPaused()  const { return m_paused.load(); }
    void restart();

    // ── Config (thread-safe) ──────────────────────────────────────────────
    void   setConfig(const Config& cfg);
    Config getConfig() const;

    // ── Enregistrement ────────────────────────────────────────────────────
    bool   startRecording(const QString& outputPath);
    void   stopRecording();
    bool   isRecording() const { return m_recording.load(); }

signals:
    void frameReady(QImage original, QImage enhanced, int fps, qint64 ms);
    // i18n : on ne transporte plus de texte figé mais une clé de traduction
    // (+ ses arguments). La résolution se fait dans Backend, à l'affichage,
    // ce qui permet de retraduire le message si la langue change ensuite.
    void sourceInfo(QString typeKey, QString source, int w, int h, double fps);
    void errorOccurred(QString key, QStringList args);
    void recordingStarted(QString path);
    void recordingStopped(QString path, int frameCount);

protected:
    void run() override;

private:
    // ── Pipeline recommandé (rapide) ──────────────────────────────────────
    cv::Mat temporalDenoise(const cv::Mat& frame, double strength);
    cv::Mat autoLut(const cv::Mat& frame, double strength);
    cv::Mat fastLime(const cv::Mat& frame, double gamma);

    // ── Algorithmes conservés / réécrits ──────────────────────────────────
    cv::Mat histogramStretch(const cv::Mat& frame);              // via calcHist
    cv::Mat agcwd(const cv::Mat& frame, double alpha);           // via histogramme
    cv::Mat mertensFusion(const cv::Mat& frame);                 // ⚠ obsolète (lent)
    cv::Mat applyCLAHE(const cv::Mat& frame, float clip);        // canal Y (YCrCb)
    cv::Mat sharpen(const cv::Mat& frame, double strength = 1.4);
    cv::Mat accumulateFrames(const cv::Mat& frame);              // héritage
    cv::Mat processFrame(const cv::Mat& frame);

    // ── Utilitaires ───────────────────────────────────────────────────────
    static cv::Mat clamp01(const cv::Mat& m);
    // Fast Guided Filter (He & Sun 2015) : box filters au 1/s, coefficients
    // a,b upsamplés — O(N/s²) au lieu de O(N).
    static cv::Mat fastGuidedFilter1C(const cv::Mat& I, const cv::Mat& p,
                                      int radius, double eps, int subsample);
    static QImage  matToQImage(const cv::Mat& mat);

    // ── État ──────────────────────────────────────────────────────────────
    cv::VideoCapture     m_cap;
    std::deque<cv::Mat>  m_frameBuffer;      // héritage accumulation
    mutable QMutex       m_mutex;
    Config               m_cfg;

    // État temporel du pipeline (réinitialisé au restart)
    cv::Mat              m_tdAccum;          // débruitage temporel (CV_32FC3)
    cv::Mat              m_tdPrevSmall;      // frame précédente (gray, 1/2 rés)
    cv::Mat              m_lutSmooth;        // Auto-LUT lissée (CV_32F 1x256)

    std::atomic<bool>    m_running{false};
    std::atomic<bool>    m_paused{false};
    std::atomic<bool>    m_restart{false};
    bool                 m_isCamera{false};
    QString              m_source;
    double               m_nativeFps{30.0};

    // ── Enregistrement ────────────────────────────────────────────────────
    QMutex               m_recMutex;
    cv::VideoWriter      m_writer;
    std::atomic<bool>    m_recording{false};
    QString              m_recordPath;
    int                  m_recordFrameCount{0};
    int                  m_recordW{0};
    int                  m_recordH{0};
};
