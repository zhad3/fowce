#ifndef LANGUAGEMODEL_H
#define LANGUAGEMODEL_H

#include <QLocale>
#include <QMetaType>

#include "fabstractyamlmodel.h"
#include "fabstractobject.h"

class FLanguage : public FAbstractObject
{
public:
    FLanguage() : m_language(QLocale::Language::C), m_countryCode(""), m_name(""), m_nativeName(""), m_flagIconPath("") {}
    FLanguage(unsigned int id, QLocale::Language language, const QString &countryCode, const QString &name, const QString &nativeName, const QString &flagIconPath)
        : FAbstractObject(id), m_language(language), m_countryCode(countryCode), m_name(name), m_nativeName(nativeName), m_flagIconPath(flagIconPath) {}
    QLocale::Language language() const { return m_language; }
    const QString countryCode() const { return m_countryCode; }
    const QString stringId() const { return m_countryCode; }
    const QString name() const { return m_name; }
    const QString nativeName() const { return m_nativeName; }
    const QString flagIconPath() const { return m_flagIconPath; }

    bool isNull() { return m_countryCode.isNull(); }

private:
    QLocale::Language m_language;
    QString m_countryCode;
    QString m_name;
    QString m_nativeName;
    QString m_flagIconPath;
};

Q_DECLARE_METATYPE(FLanguage);

class FLanguageModel : public FAbstractYAMLModel
{
    Q_OBJECT
public:
    static const FLanguageModel* Instance() { return m_instance; }
    static void SetInstance(const FLanguageModel *instance) { m_instance = instance; }

    explicit FLanguageModel(QObject *parent = nullptr);

    //void addLanguage(const FLanguage language);
    //void addLanguage(const QString &countryCode, const FLanguage language);
    bool selectLanguage(const QString &countryCode);
    const FLanguage* selectedLanguage() const;
    const FLanguage* defaultLanguage() const { return m_defaultLanguage; }
    const FLanguage* get(const QString &stringId) const;
    const FLanguage* get(int id) const;

    void load() override;
    const QString YAMLFile() const override { return m_yamlFile; }

private:
    static const FLanguageModel *m_instance;
    const QString m_yamlFile = "Languages.yaml";
    FLanguage *m_selectedLanguage;
    FLanguage *m_defaultLanguage;
    //QMap<QString, FLanguage*> m_languages;

    FLanguage *value_p(const QString &stringId);

signals:
    void languageSelected(const FLanguage *language);
};

#endif // LANGUAGEMODEL_H
