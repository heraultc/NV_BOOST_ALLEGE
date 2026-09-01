#include "translator.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLocale>
#include <QSettings>
#include <QDebug>

Translator* Translator::s_instance = nullptr;

//---------------------------------------------------------------------------//
//  Catalogue des langues supportées
//  Pour en ajouter une : une ligne ici + un fichier i18n/<code>.json
//  + une entrée dans resources.qrc. Rien d'autre.
//---------------------------------------------------------------------------//
static QVariantMap makeLang(const QString& code,
                            const QString& name,
                            const QString& flag)
{
    QVariantMap m;
    m["code"] = code;
    m["name"] = name;
    m["flag"] = flag;
    return m;
}

Translator::Translator(QObject* parent) : QObject(parent)
{
    if (!s_instance) s_instance = this;

    m_languages << makeLang("fr",    "Français",   "FR")
                << makeLang("en",    "English",    "EN")
                << makeLang("es",    "Español",    "ES")
                << makeLang("pt",    "Português",  "PT")
                << makeLang("zh_CN", "简体中文",    "中文");

    // L'anglais sert toujours de secours pour les clés manquantes
    m_fallback = loadFile("en");

    // Langue mémorisée > langue système > français
    QSettings st;
    QString code = st.value("ui/language").toString();
    if (code.isEmpty()) code = detectSystemLanguage();

    setLanguage(code);
}

Translator* Translator::instance()
{
    return s_instance;
}

//---------------------------------------------------------------------------//
//  Chargement d'un fichier de langue
//---------------------------------------------------------------------------//
QVariantMap Translator::loadFile(const QString& code) const
{
    QFile f(QStringLiteral(":/i18n/%1.json").arg(code));
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "[i18n] fichier de langue introuvable :" << code;
        return {};
    }

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[i18n]" << code << "JSON invalide :" << err.errorString();
        return {};
    }
    return doc.object().toVariantMap();
}

//---------------------------------------------------------------------------//
//  Détection de la langue système
//---------------------------------------------------------------------------//
QString Translator::detectSystemLanguage() const
{
    const QString sys = QLocale::system().name();      // ex. "pt_BR", "zh_CN"

    // Correspondance exacte (zh_CN)
    for (const QVariant& v : m_languages)
        if (v.toMap()["code"].toString().compare(sys, Qt::CaseInsensitive) == 0)
            return sys;

    // Correspondance sur le code langue seul (pt_BR → pt)
    const QString base = sys.section('_', 0, 0).toLower();
    for (const QVariant& v : m_languages)
        if (v.toMap()["code"].toString() == base)
            return base;

    // Toute variante chinoise simplifiée
    if (base == "zh") return QStringLiteral("zh_CN");

    return QStringLiteral("fr");
}

//---------------------------------------------------------------------------//
//  Changement de langue
//---------------------------------------------------------------------------//
void Translator::setLanguage(const QString& code)
{
    // Refuser un code inconnu
    bool known = false;
    for (const QVariant& v : m_languages)
        if (v.toMap()["code"].toString() == code) { known = true; break; }

    const QString target = known ? code : QStringLiteral("fr");
    if (target == m_language && !m_strings.isEmpty()) return;

    QVariantMap loaded = (target == "en") ? m_fallback : loadFile(target);

    // Fusion : l'anglais comble toute clé absente de la traduction
    QVariantMap merged = m_fallback;
    for (auto it = loaded.constBegin(); it != loaded.constEnd(); ++it)
        merged.insert(it.key(), it.value());

    m_strings  = merged;
    m_language = target;

    QSettings().setValue("ui/language", m_language);

    emit languageChanged();
}

//---------------------------------------------------------------------------//
//  Accès aux chaînes
//---------------------------------------------------------------------------//
QString Translator::t(const QString& key) const
{
    const auto it = m_strings.constFind(key);
    if (it != m_strings.constEnd()) return it.value().toString();

    qWarning() << "[i18n] clé manquante :" << key;
    return key;                                  // visible = facile à repérer
}

QString Translator::fmt(const QString& pattern, const QVariantList& args) const
{
    QString out = pattern;
    // Remplacement en ordre décroissant pour que %10 ne casse pas %1
    for (int i = args.size(); i >= 1; --i)
        out.replace(QLatin1Char('%') + QString::number(i),
                    args.at(i - 1).toString());
    return out;
}

QString Translator::t(const QString& key, const QStringList& args) const
{
    QVariantList vl;
    for (const QString& a : args) vl << a;
    return fmt(t(key), vl);
}
