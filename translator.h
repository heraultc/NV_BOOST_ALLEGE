#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QStringList>

//===========================================================================//
//  Translator — i18n runtime de NV-BOOST
//
//  • Une langue = un fichier JSON plat  (:/i18n/<code>.json)
//  • Exposé à QML sous le nom de contexte "i18n"
//        Text { text: i18n.s.srcTitle }            → binding réactif
//        Text { text: i18n.fmt(i18n.s.stConnecting, [src]) }
//  • Accessible depuis le C++ via Translator::instance()->t("clé")
//    ou la macro TR("clé")
//
//  Le changement de langue est instantané : le QVariantMap `s` est une
//  Q_PROPERTY notifiée, donc tous les bindings QML qui la lisent sont
//  ré-évalués automatiquement. Aucun redémarrage, aucun rechargement de QML.
//===========================================================================//
class Translator : public QObject
{
    Q_OBJECT

    // Dictionnaire de la langue courante (fusionné avec l'anglais en secours)
    Q_PROPERTY(QVariantMap  s          READ strings   NOTIFY languageChanged)
    // Code de la langue courante : "fr", "en", "es", "pt", "zh_CN"
    Q_PROPERTY(QString      language   READ language  WRITE setLanguage NOTIFY languageChanged)
    // Liste des langues disponibles : [{ code, name, flag }, …]
    Q_PROPERTY(QVariantList languages  READ languages CONSTANT)

public:
    explicit Translator(QObject* parent = nullptr);

    static Translator* instance();

    QVariantMap  strings()   const { return m_strings; }
    QString      language()  const { return m_language; }
    QVariantList languages() const { return m_languages; }

    // Traduction directe (C++ et QML)
    Q_INVOKABLE QString t(const QString& key) const;

    // Substitution de %1, %2, … dans un motif déjà traduit
    Q_INVOKABLE QString fmt(const QString& pattern, const QVariantList& args) const;

    // Raccourci : t(key) puis fmt(…)
    QString t(const QString& key, const QStringList& args) const;

public slots:
    void setLanguage(const QString& code);

signals:
    void languageChanged();

private:
    QVariantMap loadFile(const QString& code) const;
    QString     detectSystemLanguage() const;

    QVariantMap  m_fallback;    // anglais — jamais vide
    QVariantMap  m_strings;     // langue courante (fallback fusionné)
    QString      m_language;
    QVariantList m_languages;

    static Translator* s_instance;
};

// Sucre syntaxique côté C++
#define TR(key) Translator::instance()->t(QStringLiteral(key))
