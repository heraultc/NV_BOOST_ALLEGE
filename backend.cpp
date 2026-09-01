#include "backend.h"
#include "translator.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

Backend::Backend(QObject* parent) : QObject(parent)
{
    m_processor = new VideoProcessor(this);

    connect(m_processor, &VideoProcessor::frameReady,
            this, &Backend::onFrameReady,        Qt::QueuedConnection);
    connect(m_processor, &VideoProcessor::sourceInfo,
            this, &Backend::onSourceInfo,         Qt::QueuedConnection);
    connect(m_processor, &VideoProcessor::errorOccurred,
            this, &Backend::onError,              Qt::QueuedConnection);
    connect(m_processor, &VideoProcessor::recordingStarted,
            this, &Backend::onRecordingStarted,   Qt::QueuedConnection);
    connect(m_processor, &VideoProcessor::recordingStopped,
            this, &Backend::onRecordingStopped,   Qt::QueuedConnection);

    // Le message de statut est stocké sous forme de clé : il suffit de
    // re-notifier QML quand la langue change pour qu'il se retraduise.
    if (Translator* tr = Translator::instance())
        connect(tr, &Translator::languageChanged,
                this, &Backend::statusChanged);

    pushConfig();
}

Backend::~Backend()
{
    if (m_processor) {
        m_processor->stopProcessing();
        m_processor->wait(3000);
    }
}

//===========================================================================//
//  Slots publics
//===========================================================================//
void Backend::openSource(const QString& source)
{
    if (source.trimmed().isEmpty()) {
        setStatus("stEmptySrc", {}, true);
        return;
    }

    if (m_running) {
        closeSource();
    }

    pushConfig();
    m_processor->openSource(source.trimmed());
    m_processor->start();
    m_running = true;
    emit runningChanged();
    setStatus("stConnecting", { source.trimmed() });
}

void Backend::closeSource()
{
    if (!m_processor) return;
    m_processor->stopProcessing();
    m_processor->wait(2000);
    m_running = false;
    m_fps = 0;
    m_latencyMs = 0;
    emit runningChanged();
    emit statsChanged();
    setStatus("stClosed");
}

void Backend::togglePause()
{
    if (!m_processor || !m_running) return;
    bool nowPaused = !m_processor->isPaused();
    m_processor->setPaused(nowPaused);
    emit pausedChanged();
}

void Backend::restart()
{
    if (m_processor && m_running) m_processor->restart();
}

void Backend::startRecording(const QString& outputPath)
{
    if (!m_processor) return;

    QString path = outputPath.trimmed();
    if (path.isEmpty()) path = QDir::homePath() + "/enhanced.mp4";

    // Ajouter timestamp si fichier existant
    if (QFile::exists(path)) {
        QString ts   = QDateTime::currentDateTime().toString("_yyyyMMdd_HHmmss");
        QString base = path.section('.', 0, -2);
        QString ext  = path.section('.', -1);
        path = base + ts + "." + ext;
    }

    if (!m_processor->startRecording(path)) {
        setStatus("stRecordFail", {}, true);
    }
}

void Backend::stopRecording()
{
    if (m_processor) m_processor->stopRecording();
}

//===========================================================================//
//  Slots privés — signaux VideoProcessor
//===========================================================================//
void Backend::onFrameReady(QImage original, QImage enhanced, int fps, qint64 ms)
{
    m_fps = fps;
    m_latencyMs = ms;
    emit statsChanged();

    emit frameUpdated(original, enhanced,
                      m_cfg.show_side_by_side,
                      m_cfg.show_overlay,
                      fps, ms,
                      m_processor && m_processor->isRecording());
}

void Backend::onSourceInfo(QString typeKey, QString source, int w, int h, double fps)
{
    Translator* tr = Translator::instance();
    setStatus("stSourceInfo", {
        tr ? tr->t(typeKey) : typeKey,
        source,
        QString::number(w),
        QString::number(h),
        QString::number(fps, 'f', 1)
    });
}

void Backend::onError(QString key, QStringList args)
{
    setStatus(key, args, true);

    // Échec d'ouverture de la source : on repasse à l'état « arrêté »
    if (key == QLatin1String("stOpenFail")) {
        m_running = false;
        emit runningChanged();
    }
}

void Backend::onRecordingStarted(QString path)
{
    m_recFile = QFileInfo(path).fileName();
    m_recFrames = 0;
    emit recordingChanged();
    setStatus("stRecStarted", { m_recFile });
}

void Backend::onRecordingStopped(QString path, int frames)
{
    m_recFile   = QFileInfo(path).fileName();
    m_recFrames = frames;
    emit recordingChanged();
    setStatus("stRecStopped", { QString::number(frames), m_recFile });
}

//===========================================================================//
//  Utilitaires
//===========================================================================//
void Backend::pushConfig()
{
    if (m_processor) m_processor->setConfig(m_cfg);
}

QString Backend::statusMsg() const
{
    Translator* tr = Translator::instance();
    if (!tr) return m_statusKey;

    const QString text = tr->t(m_statusKey, m_statusArgs);
    // Le préfixe ⚠ est ajouté ici (et non dans les fichiers de langue) :
    // NvStatusBar s'en sert pour colorer la ligne en orange.
    return m_statusWarn ? QStringLiteral("⚠  ") + text : text;
}

void Backend::setStatus(const QString& key, const QStringList& args, bool warning)
{
    m_statusKey  = key;
    m_statusArgs = args;
    m_statusWarn = warning;
    emit statusChanged();
}
