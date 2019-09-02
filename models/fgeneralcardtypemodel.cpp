#include <yaml-cpp/yaml.h>

#include "yamlconvert.h"
#include "flanguagemodel.h"
#include "fgeneralcardtypemodel.h"

FGeneralCardType::~FGeneralCardType()
{
    QHash<QString, FGeneralCardTypeText*>::iterator it;
    for (it = m_text.begin(); it != m_text.end(); ++it) {
        delete (*it);
    }
}

void FGeneralCardType::addText(const QString &countryCode, FGeneralCardTypeText *text)
{
    if (m_text.contains(countryCode)) {
        delete m_text[countryCode];
    }
    m_text.insert(countryCode, text);
}

const QString FGeneralCardType::name() const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        return m_text[languageModel->selectedLanguage()->countryCode()]->name();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        return m_text[languageModel->defaultLanguage()->countryCode()]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2'.").arg(QObject::tr("General Card Type"), m_stringId)));
    return QString();
}

const QString FGeneralCardType::name(const QString &countryCode) const
{
    if (m_text.contains(countryCode)) {
        return m_text[countryCode]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("General Card Type"), m_stringId, countryCode)));
    return QString();
}

const FGeneralCardTypeModel* FGeneralCardTypeModel::m_instance = nullptr;

FGeneralCardTypeModel::FGeneralCardTypeModel(QObject *parent) : FAbstractYAMLModel(parent)
{
    beginLoading();
    load();
    loadText(FLanguageModel::Instance()->defaultLanguage()->countryCode());
    if (FLanguageModel::Instance()->defaultLanguage()->id() != FLanguageModel::Instance()->selectedLanguage()->id()) {
        loadText(FLanguageModel::Instance()->selectedLanguage()->countryCode());
    }
    endLoading();
}

void FGeneralCardTypeModel::load()
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
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("General Card Type"), m_yamlFile)));
        return;
    }

    m_vec.reserve(int(node.size()));

    int i = 0;

    YAML::const_iterator it;
    for (it = node.begin(); it != node.end(); ++it) {
        if (!it->IsMap()) {
            qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected type '%2'.").arg(m_yamlFile, "Map")));
            continue;
        }
        try {
            const YAML::Node& generalCardTypeNode = *it;
            if (!generalCardTypeNode["Id"] || !generalCardTypeNode["Id"].IsScalar()) {
                qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected '%2' with type '%3'.").arg(m_yamlFile, "Id", "Scalar")));
                continue;
            }

            QString generalCardTypeId = generalCardTypeNode["Id"].as<QString>();

            FGeneralCardType *attribute = new FGeneralCardType(i, generalCardTypeId);
            insert(generalCardTypeId, attribute);

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("General Card Type"), m_yamlFile)));
        return;
    }
}

void FGeneralCardTypeModel::loadText(const QString &countryCode)
{
    YAML::Node node;
    try {
        node = YAML::LoadFile(QString("data/text/" + countryCode + "/" + m_yamlFile).toStdString());
    } catch (YAML::BadFile e) {
        qWarning(qUtf8Printable(QObject::tr("Couldn't open text file 'text/%1/%2'. (%3)").arg(countryCode, m_yamlFile, QString(e.msg.data()))));
        return;
    } catch (YAML::ParserException e) {
        qFatal(e.msg.data());
    }

    if (!node.IsSequence()) {
        qFatal(qUtf8Printable((QObject::tr("Invalid node type when loading yaml file '%1'. Expected type '%2'.").arg(m_yamlFile, "Sequence"))));
        return;
    }
    if (node.size() == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("General Card Type"), m_yamlFile)));
        return;
    }

    int i = 0;

    YAML::const_iterator it;
    for (it = node.begin(); it != node.end(); ++it) {
        if (!it->IsMap()) {
            qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected type '%2'.").arg(m_yamlFile, "Map")));
            continue;
        }
        try {
            const YAML::Node& textNode = *it;
            if (!textNode["Id"] || !textNode["Id"].IsScalar()) {
                qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected '%2' with type '%3'.").arg(m_yamlFile, "Id", "Scalar")));
                continue;
            }

            QString textId = textNode["Id"].as<QString>();
            if (!m_data.contains(textId)) {
                qWarning(qUtf8Printable(QObject::tr("Found text whose parent %1 does not exist when loading '%2'. (%3)").arg(QObject::tr("General Card Type"), m_yamlFile, textId)));
                continue;
            }

            QString name = "";
            if (textNode["Name"] && textNode["Name"].IsScalar()) {
                name = textNode["Name"].as<QString>();
            }

            FGeneralCardTypeText *generalCardTypeText = new FGeneralCardTypeText(name);

            FGeneralCardType *generalCardType = value_p(textId);
            if (generalCardType) {
                generalCardType->addText(countryCode, generalCardTypeText);
            }

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("General Card Type"), m_yamlFile)));
        return;
    }
}

const FGeneralCardType *FGeneralCardTypeModel::get(const QString &stringId) const
{
    if (m_data.contains(stringId)) {
        const FGeneralCardType *generalCardType = static_cast<const FGeneralCardType*>(m_data[stringId]);
        return generalCardType;
    }
    return nullptr;
}

const FGeneralCardType *FGeneralCardTypeModel::get(int id) const
{
    if (id < dataCount() && id >= 0) {
        const FGeneralCardType *generalCardType = static_cast<const FGeneralCardType*>(m_vec[id]);
        return generalCardType;
    }
    return nullptr;
}

FGeneralCardType *FGeneralCardTypeModel::value_p(const QString &stringId)
{
    if (m_data.contains(stringId)) {
        FAbstractObject *generalCardType = m_data.value(stringId);
        return static_cast<FGeneralCardType*>(generalCardType);
    } else {
        return nullptr;
    }
}
