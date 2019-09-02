#include <yaml-cpp/yaml.h>

#include "yamlconvert.h"
#include "fwillcharacteristicmodel.h"
#include "flanguagemodel.h"

FWillCharacteristic::~FWillCharacteristic()
{
    QHash<QString, FWillCharacteristicText*>::iterator it = m_text.begin();
    for (; it != m_text.end(); ++it) {
        delete(*it);
    }
}

void FWillCharacteristic::addText(const QString &countryCode, FWillCharacteristicText *characteristicText)
{
    if (m_text.contains(countryCode)) {
        delete m_text[countryCode];
    }
    m_text.insert(countryCode, characteristicText);
}

const QString FWillCharacteristic::name() const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        return m_text[languageModel->selectedLanguage()->countryCode()]->name();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        return m_text[languageModel->defaultLanguage()->countryCode()]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2'.").arg(QObject::tr("Will Characteristic"), m_stringId)));
    return QString();
}

const QString FWillCharacteristic::name(const QString &countryCode) const
{
    if (m_text.contains(countryCode)) {
        return m_text[countryCode]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("Will Characteristic"), m_stringId, countryCode)));
    return QString();
}

const FWillCharacteristicModel* FWillCharacteristicModel::m_instance = nullptr;

FWillCharacteristicModel::FWillCharacteristicModel(QObject *parent) : FAbstractYAMLModel(parent)
{
    beginLoading();
    load();
    loadText(FLanguageModel::Instance()->defaultLanguage()->countryCode());
    if (FLanguageModel::Instance()->defaultLanguage()->id() != FLanguageModel::Instance()->selectedLanguage()->id()) {
        loadText(FLanguageModel::Instance()->selectedLanguage()->countryCode());
    }
    endLoading();
}

void FWillCharacteristicModel::load()
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
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Will Characteristic"), m_yamlFile)));
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
            const YAML::Node& characteristicNode = *it;
            if (!characteristicNode["Id"] || !characteristicNode["Id"].IsScalar()) {
                qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected '%2' with type '%3'.").arg(m_yamlFile, "Id", "Scalar")));
                continue;
            }

            QString characteristicId = characteristicNode["Id"].as<QString>();

            QString iconPath = "";
            if (characteristicNode["Icon"] && characteristicNode["Icon"].IsScalar()) {
                iconPath = characteristicNode["Icon"].as<QString>();
            }

            FWillCharacteristic *characteristic = new FWillCharacteristic(i, characteristicId, iconPath);
            insert(characteristicId, characteristic);

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Will Characteristic"), m_yamlFile)));
        return;
    }
}

void FWillCharacteristicModel::loadText(const QString &countryCode)
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
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("Will Characteristic"), m_yamlFile)));
        return;
    }

    unsigned int i = 0;

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
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
                qWarning(qUtf8Printable(QObject::tr("Found text whose parent %1 does not exist when loading '%2'. (%3)").arg(QObject::tr("Will Characteristic"), m_yamlFile, textId)));
                continue;
            }

            QString name = "";
            if (textNode["Name"] && textNode["Name"].IsScalar()) {
                name = textNode["Name"].as<QString>();
            }

            FWillCharacteristicText *characteristicText = new FWillCharacteristicText(name);

            FWillCharacteristic *characteristic = value_p(textId);
            if (characteristic) {
                characteristic->addText(countryCode, characteristicText);
            }

            i++;

        }  catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("Will Characteristic"), m_yamlFile)));
        return;
    }
}

const FWillCharacteristic *FWillCharacteristicModel::get(const QString &stringId) const
{
    if (m_data.contains(stringId)) {
        const FWillCharacteristic *characteristic = static_cast<const FWillCharacteristic*>(m_data[stringId]);
        return characteristic;
    }
    return nullptr;
}

const FWillCharacteristic *FWillCharacteristicModel::get(int id) const
{
    if (id < dataCount() && id >= 0) {
        const FWillCharacteristic *characteristic = static_cast<const FWillCharacteristic*>(m_vec[int(id)]);
        return characteristic;
    }
    return nullptr;
}

FWillCharacteristic *FWillCharacteristicModel::value_p(const QString &stringId)
{
    if (m_data.contains(stringId)) {
        FAbstractObject *characteristic = m_data.value(stringId);
        return static_cast<FWillCharacteristic*>(characteristic);
    } else {
        return nullptr;
    }
}
