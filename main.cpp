#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QFont>
#include <QFontDatabase>
#include "backend.h"
#include "framenotifier.h"
#include "translator.h"

//===========================================================================//
//  FrameProvider — fournit les frames QImage au composant Image QML
//===========================================================================//
class FrameProvider : public QQuickImageProvider
{
public:
    FrameProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    void update(const QImage& original, const QImage& enhanced,
                bool sideBySide, bool overlay,
                int fps, qint64 ms, bool rec)
    {
        QMutexLocker lk(&m_mutex);

        if (sideBySide) {
            int W = original.width() + enhanced.width();
            int H = std::max(original.height(), enhanced.height());
            m_frame = QImage(W, H, QImage::Format_RGB888);
            m_frame.fill(Qt::black);
            QPainter p(&m_frame);
            p.drawImage(0, 0, original);
            p.drawImage(original.width(), 0, enhanced);
            if (overlay) {
                QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
                font.setPointSize(9);
                p.setFont(font);
                p.setPen(QColor(180,180,180));
                p.drawText(6, 16, TR("ovOriginal"));
                p.setPen(QColor(0,220,80));
                p.drawText(original.width()+6, 16, TR("ovEnhanced"));
                p.drawText(original.width()+6, 32,
                           Translator::instance()->fmt(TR("ovFps"), { fps, ms }));
                if (rec) {
                    p.setPen(QColor(255,60,60));
                    p.drawText(original.width()+6, 48, TR("ovRec"));
                }
            }
            p.end();
        } else {
            m_frame = enhanced;
            if (overlay) {
                QPainter p(&m_frame);
                QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
                f.setPointSize(9);
                p.setFont(f);
                p.setPen(QColor(0,220,80));
                p.drawText(6, 16,
                           Translator::instance()->fmt(TR("ovFps"), { fps, ms }));
                if (rec) {
                    p.setPen(QColor(255,60,60));
                    p.drawText(6, 32, TR("ovRec"));
                }
                p.end();
            }
        }
    }

    QImage requestImage(const QString&, QSize* size, const QSize&) override
    {
        QMutexLocker lk(&m_mutex);
        if (size) *size = m_frame.size();
        return m_frame;
    }

private:
    QMutex m_mutex;
    QImage m_frame;
};

//===========================================================================//
//  main
//===========================================================================//
int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("NV-BOOST");
    app.setOrganizationName("NightVector");

    // ⚠ Le Translator doit exister AVANT le Backend : celui-ci l'interroge
    //   dès son constructeur (message de statut initial, connexion au signal
    //   languageChanged). QSettings a besoin des noms d'app/orga ci-dessus.
    auto* i18n      = new Translator(&app);

    auto* backend   = new Backend(&app);
    auto* provider  = new FrameProvider;          // ownership passé à engine
    auto* notifier  = new FrameNotifier(&app);

    // Quand une frame est prête : mettre à jour le provider ET notifier QML
    QObject::connect(backend, &Backend::frameUpdated,
        [provider, notifier](const QImage& orig, const QImage& enh,
                             bool sbs, bool ov, int fps, qint64 ms, bool rec)
    {
        provider->update(orig, enh, sbs, ov, fps, ms, rec);
        emit notifier->frameChanged();
    });

    QQmlApplicationEngine engine;
    engine.addImageProvider("frames", provider);   // engine prend ownership
    engine.rootContext()->setContextProperty("backend",  backend);
    engine.rootContext()->setContextProperty("notifier", notifier);
    engine.rootContext()->setContextProperty("i18n",     i18n);

    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty()) return -1;

    return app.exec();
}
