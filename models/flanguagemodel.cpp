#include <QDebug>

#include <yaml-cpp/yaml.h>

#include "yamlconvert.h"
#include "flanguagemodel.h"
#include "util.h"

const FLanguageModel* FLanguageModel::m_instance = nullptr;

FLanguageModel::FLanguageModel(QObject *parent) : FAbstractYAMLModel(parent)
{
    m_selectedLanguage = nullptr;
    m_defaultLanguage = nullptr;
    beginLoading();
    load();
    endLoading();
}

/*void FLanguageModel::addLanguage(const FLanguage language)
{
    QVariant v;
    v.setValue(language);
    insert(language.countryCode(), v);
}*/

/*void FLanguageModel::addLanguage(const QString &countryCode, const FLanguage language)
{
    QVariant v;
    v.setValue(language);
    insert(countryCode, v);
}*/

bool FLanguageModel::selectLanguage(const QString &countryCode)
{
    FLanguage *language = value_p(countryCode);
    if (language) {
        m_selectedLanguage = language;
        emit languageSelected(language);
        return true;
    }
    return false;
}

const FLanguage* FLanguageModel::selectedLanguage() const
{
    if (m_selectedLanguage) {
        return m_selectedLanguage;
    }
    return m_defaultLanguage;
}

void FLanguageModel::load()
{
    YAML::Node node;
    try {
        node = YAML::LoadFile(QString("data/" + m_yamlFile).toStdString());
    } catch (YAML::BadFile e) {
        qFatal(e.msg.data());
    } catch (YAML::ParserException e) {
        qFatal(e.msg.data());
    }

    if (!node.IsSequence()) {
        qFatal(qUtf8Printable((QObject::tr("Invalid node type when loading yaml file '%1'. Expected type '%2'.").arg(m_yamlFile, "Sequence"))));
        return;
    }
    if (node.size() == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Language"), m_yamlFile)));
        return;
    }

    unsigned int i = 0;

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
        if (!it->IsMap()) {
            qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected type '%2'.").arg(m_yamlFile, "Map")));
            continue;
        }
        try {
            const YAML::Node& languageNode = *it;

            if (!languageNode["Id"] || !languageNode["Id"].IsScalar()) {
                qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected '%2' with type '%3'.").arg(m_yamlFile, "Id", "Scalar")));
                continue;
            }

            QString languageId = languageNode["Id"].as<QString>();
            QLocale locale(languageId);

            if (locale.language() == QLocale::Language::C) {
                qWarning(qUtf8Printable(QObject::tr("Invalid language id '%2'.").arg(languageId)));
                continue;
            }

            QString name = "";
            if (languageNode["Name"] && languageNode["Name"].IsScalar()) {
                name = languageNode["Name"].as<QString>();
            }

            QString nativeName = "";
            if (languageNode["NativeName"] && languageNode["NativeName"].IsScalar()) {
                nativeName = languageNode["NativeName"].as<QString>();
            }

            QString flagIconPath = "";
            if (languageNode["FlagIcon"] && languageNode["FlagIcon"].IsScalar()) {
                flagIconPath = languageNode["FlagIcon"].as<QString>();
            }

            bool defaultLang = false;
            if (languageNode["Default"] && languageNode["Default"].IsScalar()) {
                defaultLang = languageNode["Default"].as<bool>();
            }

            /*FLanguage language(i,
                        locale.language(),
                        languageId,
                        name,
                        nativeName,
                        flagIconPath);*/

            FLanguage *language = new FLanguage(i, locale.language(), languageId, name, nativeName, flagIconPath);
            insert(languageId, language);

            //addLanguage(language);

            if (defaultLang) {
                m_defaultLanguage = language;
                m_selectedLanguage = language;
            }

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Language"), m_yamlFile)));
        return;
    }

    if (!m_defaultLanguage) {
        // Because we throw an error if no entry exists, first() should always return a value
        FLanguage *language = static_cast<FLanguage*>(dataVec()->first());
        m_defaultLanguage = language;
        m_selectedLanguage = language;
    }
}

const FLanguage *FLanguageModel::get(const QString &stringId) const
{
    if (m_data.contains(stringId)) {
        const FLanguage *language = static_cast<const FLanguage*>(m_data[stringId]);
        return language;
    }
    return nullptr;
}

const FLanguage *FLanguageModel::get(int id) const
{
    if (id < dataCount() && id >= 0) {
        const FLanguage *language = static_cast<const FLanguage*>(m_vec[int(id)]);
        return language;
    }
    return nullptr;
}

FLanguage *FLanguageModel::value_p(const QString &stringId)
{
    if (m_data.contains(stringId)) {
        FAbstractObject *language = m_data.value(stringId);
        return static_cast<FLanguage*>(language);
    } else {
        return nullptr;
    }
}
