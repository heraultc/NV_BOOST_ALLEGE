#include "videoprocessor.h"

#include <QDebug>
#include <QDir>
#include <cmath>
#include <algorithm>

//===========================================================================//
//  Ctor / Dtor
//===========================================================================//
VideoProcessor::VideoProcessor(QObject* parent)
    : QThread(parent)
{}

VideoProcessor::~VideoProcessor()
{
    stopProcessing();
    wait();
}

//===========================================================================//
//  Contrôle du thread
//===========================================================================//
bool VideoProcessor::openSource(const QString& source)
{
    m_source = source;
    bool isNumeric = false;
    source.toInt(&isNumeric);
    m_isCamera = isNumeric;
    return true;
}

void VideoProcessor::stopProcessing()
{
    stopRecording();
    m_running = false;
    m_paused  = false;
}

void VideoProcessor::setPaused(bool paused) { m_paused = paused; }

void VideoProcessor::restart()
{
    m_restart = true;
    m_paused  = false;
}

void VideoProcessor::setConfig(const Config& cfg)
{
    QMutexLocker lock(&m_mutex);
    m_cfg = cfg;
}

Config VideoProcessor::getConfig() const
{
    QMutexLocker lock(&m_mutex);
    return m_cfg;
}

//===========================================================================//
//  Enregistrement
//===========================================================================//
bool VideoProcessor::startRecording(const QString& outputPath)
{
    QMutexLocker lock(&m_recMutex);
    if (m_recording) return false;
    if (m_recordW == 0 || m_recordH == 0) return false;

    QString ext = outputPath.section('.', -1).toLower();
    int fourcc;
    if      (ext == "avi")  fourcc = cv::VideoWriter::fourcc('M','J','P','G');
    else if (ext == "mkv")  fourcc = cv::VideoWriter::fourcc('X','2','6','4');
    else                    fourcc = cv::VideoWriter::fourcc('m','p','4','v');

    bool ok = m_writer.open(outputPath.toStdString(), fourcc,
                            m_nativeFps, cv::Size(m_recordW, m_recordH));
    if (!ok) return false;

    m_recordPath       = outputPath;
    m_recordFrameCount = 0;
    m_recording        = true;
    emit recordingStarted(outputPath);
    return true;
}

void VideoProcessor::stopRecording()
{
    QMutexLocker lock(&m_recMutex);
    if (!m_recording) return;
    m_writer.release();
    m_recording = false;
    emit recordingStopped(m_recordPath, m_recordFrameCount);
}

//===========================================================================//
//  Boucle principale
//===========================================================================//
void VideoProcessor::run()
{
    m_running = true;

    cv::VideoCapture cap;
    if (m_isCamera) {
#ifdef Q_OS_WIN
        // Windows : DirectShow ouvre la webcam bien plus vite que Media Foundation
        cap.open(m_source.toInt(), cv::CAP_DSHOW);
        if (!cap.isOpened()) cap.open(m_source.toInt());   // repli MSMF
#else
        cap.open(m_source.toInt());
#endif
    } else {
        cap.open(m_source.toStdString());
    }

    if (!cap.isOpened()) {
        emit errorOccurred("stOpenFail", QStringList{ m_source });
        return;
    }

    int W = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int H = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    m_nativeFps = cap.get(cv::CAP_PROP_FPS);
    if (m_nativeFps <= 0 || m_nativeFps > 120) m_nativeFps = 30.0;

    { QMutexLocker lk(&m_recMutex); m_recordW = W; m_recordH = H; }

    // Réinitialiser l'état temporel
    m_tdAccum.release();
    m_tdPrevSmall.release();
    m_lutSmooth.release();

    QString srcTypeKey = m_isCamera ? "stSrcCamera"
                       : (m_source.startsWith("rtsp://")  ||
                          m_source.startsWith("http://")  ||
                          m_source.startsWith("https://") ||
                          m_source.startsWith("rtmp://")) ? "stSrcUrl" : "stSrcFile";

    emit sourceInfo(srcTypeKey, m_source, W, H, m_nativeFps);

    cv::Mat frame;
    int     frameCount = 0;
    QElapsedTimer totalTimer;
    totalTimer.start();

    while (m_running)
    {
        if (m_paused) { msleep(30); continue; }

        if (m_restart) {
            if (!m_isCamera) cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            m_frameBuffer.clear();
            m_tdAccum.release();
            m_tdPrevSmall.release();
            m_lutSmooth.release();
            frameCount = 0;
            totalTimer.restart();
            m_restart = false;
        }

        cap >> frame;
        if (frame.empty()) {
            if (!m_isCamera) {
                emit errorOccurred("stVideoEnd", QStringList{});
                while (m_running && !m_restart) msleep(30);
                if (m_restart) {
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                    m_frameBuffer.clear();
                    m_tdAccum.release();
                    m_tdPrevSmall.release();
                    m_lutSmooth.release();
                    frameCount = 0;
                    totalTimer.restart();
                    m_restart = false;
                }
            } else {
                msleep(10);
            }
            continue;
        }

        QElapsedTimer procTimer;
        procTimer.start();
        cv::Mat processed = processFrame(frame);
        qint64 ms = procTimer.elapsed();

        frameCount++;
        int curFps = (totalTimer.elapsed() > 0)
                     ? (int)(frameCount * 1000.0 / totalTimer.elapsed()) : 0;

        {
            QMutexLocker lk(&m_recMutex);
            if (m_recording && m_writer.isOpened()) {
                m_writer.write(processed);
                ++m_recordFrameCount;
            }
        }

        emit frameReady(matToQImage(frame), matToQImage(processed), curFps, ms);

        int delay = std::max(1, (int)(1000.0 / m_nativeFps));
        msleep(std::max(1, delay - (int)ms));
    }

    { QMutexLocker lk(&m_recMutex); m_recordW = 0; m_recordH = 0; }
    cap.release();
}

//===========================================================================//
//  Conversion cv::Mat → QImage
//===========================================================================//
QImage VideoProcessor::matToQImage(const cv::Mat& mat)
{
    if (mat.empty()) return {};
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, (int)rgb.step,
                  QImage::Format_RGB888).copy();
}

//===========================================================================//
//  PIPELINE PRINCIPAL
//
//  Ordre optimal de nuit :
//    débruitage temporel → rehaussement (Auto-LUT | Fast-LIME)
//    → CLAHE → netteté
//
//  Le débruitage AVANT le rehaussement est capital : tout gain d'exposition
//  amplifie le bruit ; on nettoie donc d'abord ce qui peut l'être.
//===========================================================================//
cv::Mat VideoProcessor::processFrame(const cv::Mat& frame)
{
    Config cfg = getConfig();
    cv::Mat r  = frame;

    // ── 1. Débruitage ───────────────────────────────────────────────────
    if (cfg.use_tdenoise)         r = temporalDenoise(r, cfg.tdenoise_strength);
    else if (cfg.accumulation > 1) r = accumulateFrames(r);   // héritage

    // ── 2. Rehaussement d'exposition (en choisir UN) ────────────────────
    if (cfg.use_autolut)          r = autoLut(r, cfg.autolut_strength);
    else if (cfg.use_lime)        r = fastLime(r, cfg.lime_gamma);

    // Héritage v1 (réimplémentés en rapide, utilisables indépendamment)
    if (cfg.use_histstretch)      r = histogramStretch(r);
    if (cfg.use_agcwd)            r = agcwd(r, cfg.agcwd_alpha);
    if (cfg.use_mertens)          r = mertensFusion(r);

    // ── 3. Finitions ────────────────────────────────────────────────────
    if (cfg.use_clahe)            r = applyCLAHE(r, cfg.clahe_clip);
    if (cfg.use_sharpen)          r = sharpen(r);

    return (r.data == frame.data) ? frame.clone() : r;
}

//===========================================================================//
//  UTILITAIRES
//===========================================================================//
cv::Mat VideoProcessor::clamp01(const cv::Mat& m)
{
    cv::Mat r;
    cv::max(m, 0.0f, r);
    cv::min(r, 1.0f, r);
    return r;
}

//===========================================================================//
//  FAST GUIDED FILTER (He & Sun, 2015)
//
//  Toutes les moyennes locales (box filters) sont calculées au 1/s de la
//  résolution ; les coefficients linéaires a,b sont ré-agrandis puis
//  appliqués en pleine résolution : q = a·I + b.
//  Coût ≈ O(N/s²) — s=4 → ~16× moins de calcul que le guided filter naïf,
//  qualité visuellement identique (a et b sont des cartes lisses).
//===========================================================================//
cv::Mat VideoProcessor::fastGuidedFilter1C(const cv::Mat& I, const cv::Mat& p,
                                           int radius, double eps, int subsample)
{
    const int s = std::max(1, subsample);
    cv::Size small(std::max(4, I.cols / s), std::max(4, I.rows / s));

    cv::Mat Is, ps;
    cv::resize(I, Is, small, 0, 0, cv::INTER_AREA);
    cv::resize(p, ps, small, 0, 0, cv::INTER_AREA);

    int rs = std::max(1, radius / s);
    cv::Size ks(2 * rs + 1, 2 * rs + 1);

    cv::Mat mI, mp, mIp, mII;
    cv::boxFilter(Is,          mI,  CV_32F, ks);
    cv::boxFilter(ps,          mp,  CV_32F, ks);
    cv::boxFilter(Is.mul(ps),  mIp, CV_32F, ks);
    cv::boxFilter(Is.mul(Is),  mII, CV_32F, ks);

    cv::Mat a = (mIp - mI.mul(mp)) / (mII - mI.mul(mI) + (float)eps);
    cv::Mat b = mp - a.mul(mI);

    cv::Mat ma, mb;
    cv::boxFilter(a, ma, CV_32F, ks);
    cv::boxFilter(b, mb, CV_32F, ks);

    cv::Mat maF, mbF;
    cv::resize(ma, maF, I.size(), 0, 0, cv::INTER_LINEAR);
    cv::resize(mb, mbF, I.size(), 0, 0, cv::INTER_LINEAR);

    return maF.mul(I) + mbF;
}

//===========================================================================//
//  A. DÉBRUITAGE TEMPOREL ADAPTATIF AU MOUVEMENT
//
//  Remplace l'« accumulation » v1 (moyenne naïve → ghosting massif).
//  Moyenne exponentielle glissante par pixel :
//      accum ← accum + α(x) · (frame − accum)
//  où α(x) dépend du mouvement local :
//      zone statique  → α petit  (forte moyenne : SNR ×3..×4 la nuit)
//      zone en mouvement → α ≈ 1 (aucun ghosting)
//  Le mouvement est estimé au 1/2 de résolution (diff absolue lissée).
//  Coût 720p ≈ 4-6 ms.  C'est LE traitement le plus rentable de nuit :
//  gratuit en FPS et il rend tout gain d'exposition exploitable.
//===========================================================================//
cv::Mat VideoProcessor::temporalDenoise(const cv::Mat& frame, double strength)
{
    strength = std::max(0.0, std::min(strength, 0.95));
    const float alphaBase = 1.0f - (float)strength * 0.9f;   // 0.1 .. 1.0

    cv::Size half(frame.cols / 2, frame.rows / 2);
    cv::Mat gray, graySmall;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::resize(gray, graySmall, half, 0, 0, cv::INTER_AREA);

    if (m_tdAccum.empty() || m_tdAccum.size() != frame.size()) {
        frame.convertTo(m_tdAccum, CV_32FC3);
        m_tdPrevSmall = graySmall.clone();
        return frame.clone();
    }

    // Carte de mouvement (1/2 rés) : diff lissée → α(x) ∈ [alphaBase, 1]
    cv::Mat diff;
    cv::absdiff(graySmall, m_tdPrevSmall, diff);
    cv::boxFilter(diff, diff, CV_8U, cv::Size(5, 5));
    m_tdPrevSmall = graySmall.clone();

    cv::Mat alphaSmall;
    // diff 8 niveaux ≈ bruit ; au-delà = mouvement. Gain 1/24 → α=1 vers diff≈22
    diff.convertTo(alphaSmall, CV_32F, (1.0 - alphaBase) / 24.0, alphaBase);
    cv::min(alphaSmall, 1.0f, alphaSmall);

    cv::Mat alpha;
    cv::resize(alphaSmall, alpha, frame.size(), 0, 0, cv::INTER_LINEAR);

    // Boucle fusionnée : accum += α·(frame − accum) et sortie 8U en une passe
    // (évite merge α×3, convertTo float plein cadre et 4 temporaires 11 Mo)
    cv::Mat out(frame.size(), CV_8UC3);
    for (int r = 0; r < frame.rows; ++r) {
        const uchar* pf = frame.ptr<uchar>(r);
        const float* pa = alpha.ptr<float>(r);
        float*       pc = m_tdAccum.ptr<float>(r);
        uchar*       po = out.ptr<uchar>(r);
        for (int c = 0; c < frame.cols; ++c) {
            const float a = pa[c];
            const int   i = c * 3;
            pc[i]   += (pf[i]   - pc[i])   * a;
            pc[i+1] += (pf[i+1] - pc[i+1]) * a;
            pc[i+2] += (pf[i+2] - pc[i+2]) * a;
            po[i]   = (uchar)(pc[i]   + 0.5f);
            po[i+1] = (uchar)(pc[i+1] + 0.5f);
            po[i+2] = (uchar)(pc[i+2] + 0.5f);
        }
    }
    return out;
}

//===========================================================================//
//  B. AUTO-LUT ADAPTATIVE  — remplace HistStretch + AGCWD + Mertens
//
//  Une seule courbe tonale 256 entrées, recalculée chaque frame depuis un
//  histogramme sous-échantillonné, combinant :
//    1. stretch percentile (0.5% / 99.5%)
//    2. gamma adaptatif type AGCWD : γ = log(0.45)/log(médiane) — ne fait
//       QUE éclaircir (γ ≤ 1), dosé par `strength`
//    3. léger roll-off des hautes lumières (protège les sources lumineuses)
//  La LUT est lissée temporellement (EMA 15%) → zéro scintillement.
//  Application : 1 appel cv::LUT.   Coût 720p ≈ 1-2 ms.
//===========================================================================//
cv::Mat VideoProcessor::autoLut(const cv::Mat& frame, double strength)
{
    strength = std::max(0.0, std::min(strength, 1.0));

    // Histogramme luminance sur image réduite (~120px de large)
    cv::Mat gray, tiny;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    int tw = 120, th = std::max(8, 120 * frame.rows / std::max(1, frame.cols));
    cv::resize(gray, tiny, cv::Size(tw, th), 0, 0, cv::INTER_AREA);

    int histSize = 256;
    float range[] = {0, 256};
    const float* ranges[] = {range};
    cv::Mat hist;
    cv::calcHist(&tiny, 1, nullptr, cv::Mat(), hist, 1, &histSize, ranges);

    const float total = (float)tiny.total();
    float acc = 0.f;
    int lo = 0, hi = 255, med = 128;
    bool gotLo = false, gotMed = false;
    for (int i = 0; i < 256; ++i) {
        acc += hist.at<float>(i);
        if (!gotLo  && acc >= total * 0.005f) { lo  = i; gotLo  = true; }
        if (!gotMed && acc >= total * 0.5f)   { med = i; gotMed = true; }
        if (acc >= total * 0.995f)            { hi  = i; break; }
    }
    if (hi <= lo + 4) { lo = std::max(0, lo - 2); hi = lo + 8; }

    // Gamma adaptatif (AGCWD) : cible médiane 0.45, éclaircit uniquement
    double medN  = std::max(0.004, med / 255.0);
    double gamma = std::log(0.45) / std::log(medN);
    gamma = std::min(1.0, std::max(0.35, gamma));           // γ ∈ [0.35, 1]
    gamma = 1.0 + strength * (gamma - 1.0);                 // dosage

    // Construction de la LUT cible
    cv::Mat lutF(1, 256, CV_32F);
    float* lf = lutF.ptr<float>();
    const double sLo = lo * strength;                        // stretch dosé
    const double sHi = 255.0 - (255.0 - hi) * strength;
    for (int i = 0; i < 256; ++i) {
        double x = (i - sLo) / std::max(1.0, sHi - sLo);     // stretch
        x = std::min(1.0, std::max(0.0, x));
        x = std::pow(x, gamma);                              // gamma
        x = x * (1.0 - 0.15 * x * x * strength)              // roll-off doux
              + 0.15 * strength * x * x;                     // des hautes lum.
        lf[i] = (float)std::min(255.0, std::max(0.0, x * 255.0));
    }

    // Lissage temporel de la LUT elle-même → aucune pompe / flicker
    if (m_lutSmooth.empty())
        m_lutSmooth = lutF.clone();
    else
        cv::addWeighted(lutF, 0.15, m_lutSmooth, 0.85, 0.0, m_lutSmooth);

    cv::Mat lut8;
    m_lutSmooth.convertTo(lut8, CV_8U);

    cv::Mat out;
    cv::LUT(frame, lut8, out);
    return out;
}

//===========================================================================//
//  FAST-LIME — carte d'illumination au 1/4 de résolution
//
//  v1 : 3 passes de bilateralFilter plein cadre (~60-80 ms).
//  v2 : T = max(B,G,R) → raffinage par fast guided filter au 1/4 →
//       T^γ calculé en basse rés → upsampling → I / T^γ.
//  Coût 720p ≈ 6-9 ms (~10× plus rapide), halos mieux contenus.
//===========================================================================//
cv::Mat VideoProcessor::fastLime(const cv::Mat& frame, double gamma)
{
    gamma = std::max(0.4, std::min(gamma, 0.9));

    // Illumination initiale : max des 3 canaux (8U, très rapide)
    std::vector<cv::Mat> bgr;
    cv::split(frame, bgr);
    cv::Mat T8;
    cv::max(bgr[0], bgr[1], T8);
    cv::max(T8, bgr[2], T8);

    // Raffinage édge-aware en basse résolution
    cv::Size quarter(std::max(8, frame.cols / 4), std::max(8, frame.rows / 4));
    cv::Mat Ts, Tf;
    cv::resize(T8, Ts, quarter, 0, 0, cv::INTER_AREA);
    Ts.convertTo(Tf, CV_32F, 1.0 / 255.0);
    Tf = fastGuidedFilter1C(Tf, Tf, 16, 1e-3, 2);

    // Gain = 1 / T^γ, borné (évite l'explosion du bruit dans le noir total)
    cv::max(Tf, 0.05f, Tf);
    cv::Mat Tg;
    cv::pow(Tf, gamma, Tg);
    cv::Mat gainSmall = 1.0f / Tg;
    cv::min(gainSmall, 8.0f, gainSmall);

    cv::Mat gain;
    cv::resize(gainSmall, gain, frame.size(), 0, 0, cv::INTER_LINEAR);

    // Application du gain en une seule passe 8U (pas de conversion float 3ch)
    cv::Mat out(frame.size(), CV_8UC3);
    for (int r = 0; r < frame.rows; ++r) {
        const uchar* pf = frame.ptr<uchar>(r);
        const float* pg = gain.ptr<float>(r);
        uchar*       po = out.ptr<uchar>(r);
        for (int c = 0; c < frame.cols; ++c) {
            const float g = pg[c];
            const int   i = c * 3;
            po[i]   = cv::saturate_cast<uchar>(pf[i]   * g);
            po[i+1] = cv::saturate_cast<uchar>(pf[i+1] * g);
            po[i+2] = cv::saturate_cast<uchar>(pf[i+2] * g);
        }
    }
    return out;
}

//===========================================================================//
//  HISTOGRAM STRETCH — réécrit avec cv::calcHist (plus de boucles pixel)
//  v1 ≈ 15-25 ms (boucles .at<uchar>) → v2 ≈ 1-2 ms.
//===========================================================================//
cv::Mat VideoProcessor::histogramStretch(const cv::Mat& frame)
{
    std::vector<cv::Mat> ch;
    cv::split(frame, ch);

    int histSize = 256;
    float range[] = {0, 256};
    const float* ranges[] = {range};

    for (auto& c : ch) {
        cv::Mat hist;
        cv::calcHist(&c, 1, nullptr, cv::Mat(), hist, 1, &histSize, ranges);
        const float total = (float)c.total();
        float acc = 0.f;
        int lo = 0, hi = 255;
        for (int i = 0; i < 256; ++i) {
            acc += hist.at<float>(i);
            if (acc >= total * 0.01f) { lo = i; break; }
        }
        acc = 0.f;
        for (int i = 255; i >= 0; --i) {
            acc += hist.at<float>(i);
            if (acc >= total * 0.01f) { hi = i; break; }
        }
        if (hi <= lo) continue;

        cv::Mat lut(1, 256, CV_8U);
        uchar* p = lut.ptr();
        for (int i = 0; i < 256; ++i)
            p[i] = cv::saturate_cast<uchar>((i - lo) * 255.0 / (hi - lo));
        cv::LUT(c, lut, c);
    }
    cv::Mat result;
    cv::merge(ch, result);
    return result;
}

//===========================================================================//
//  AGCWD — réécrit : médiane extraite de l'histogramme (plus de vecteur de
//  pixels ni de nth_element plein cadre), conversion YCrCb (moins chère que
//  Lab).  v1 ≈ 20-30 ms → v2 ≈ 2-3 ms.
//===========================================================================//
cv::Mat VideoProcessor::agcwd(const cv::Mat& frame, double alpha)
{
    cv::Mat ycc;
    cv::cvtColor(frame, ycc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> planes;
    cv::split(ycc, planes);
    cv::Mat& Y = planes[0];

    int histSize = 256;
    float range[] = {0, 256};
    const float* ranges[] = {range};
    cv::Mat hist;
    cv::calcHist(&Y, 1, nullptr, cv::Mat(), hist, 1, &histSize, ranges);

    const float total = (float)Y.total();
    float acc = 0.f;
    int med = 128;
    for (int i = 0; i < 256; ++i) {
        acc += hist.at<float>(i);
        if (acc >= total * 0.5f) { med = i; break; }
    }

    double medN  = std::max(1e-3, med / 255.0);
    double gamma = std::log(0.45) / std::log(medN);
    gamma = std::max(0.6, std::min(1.0 + alpha * (gamma - 1.0), 2.2));

    cv::Mat lut(1, 256, CV_8U);
    uchar* p = lut.ptr();
    for (int i = 0; i < 256; ++i)
        p[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, gamma) * 255.0);
    cv::LUT(Y, lut, Y);

    cv::merge(planes, ycc);
    cv::Mat result;
    cv::cvtColor(ycc, result, cv::COLOR_YCrCb2BGR);
    return result;
}

//===========================================================================//
//  MERTENS — conservé pour compatibilité mais ⚠ intrinsèquement lourd
//  (pyramides laplaciennes × 3 expositions). Préférer Auto-LUT.
//===========================================================================//
cv::Mat VideoProcessor::mertensFusion(const cv::Mat& frame)
{
    cv::Mat dark, bright;
    cv::Mat ld(1, 256, CV_8U), lb(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i) {
        ld.ptr()[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, 2.2) * 255.0);
        lb.ptr()[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, 0.4) * 255.0);
    }
    cv::LUT(frame, ld, dark);
    cv::LUT(frame, lb, bright);
    std::vector<cv::Mat> exposures = {dark, frame, bright};
    cv::Ptr<cv::MergeMertens> mg = cv::createMergeMertens();
    cv::Mat f;
    mg->process(exposures, f);
    f = clamp01(f);
    cv::Mat result;
    f.convertTo(result, CV_8UC3, 255.0);
    return result;
}

//===========================================================================//
//  CLAHE — canal Y de YCrCb (conversion ~2× moins chère que Lab)
//===========================================================================//
cv::Mat VideoProcessor::applyCLAHE(const cv::Mat& frame, float clip)
{
    cv::Mat ycc;
    cv::cvtColor(frame, ycc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> planes;
    cv::split(ycc, planes);
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clip, cv::Size(8, 8));
    clahe->apply(planes[0], planes[0]);
    cv::merge(planes, ycc);
    cv::Mat result;
    cv::cvtColor(ycc, result, cv::COLOR_YCrCb2BGR);
    return result;
}

//===========================================================================//
//  NETTETÉ — unsharp mask sigma réduit (1.5) : moins de halos autour des
//  sources lumineuses nocturnes, ~2× moins cher que sigma 3.
//===========================================================================//
cv::Mat VideoProcessor::sharpen(const cv::Mat& frame, double strength)
{
    cv::Mat blurred;
    cv::GaussianBlur(frame, blurred, cv::Size(0, 0), 1.5);
    cv::Mat result;
    cv::addWeighted(frame, strength, blurred, -(strength - 1.0), 0, result);
    return result;
}

//===========================================================================//
//  ACCUMULATION (héritage) — conservée mais préférer temporalDenoise
//===========================================================================//
cv::Mat VideoProcessor::accumulateFrames(const cv::Mat& frame)
{
    m_frameBuffer.push_back(frame.clone());
    Config cfg = getConfig();
    while ((int)m_frameBuffer.size() > cfg.accumulation) m_frameBuffer.pop_front();
    if (m_frameBuffer.size() == 1) return frame.clone();
    cv::Mat acc = cv::Mat::zeros(frame.size(), CV_32FC3);
    for (const auto& f : m_frameBuffer) {
        cv::Mat ff;
        f.convertTo(ff, CV_32FC3);
        acc += ff;
    }
    acc /= (double)m_frameBuffer.size();
    cv::Mat result;
    acc.convertTo(result, CV_8UC3);
    return result;
}
