#include <yaml-cpp/yaml.h>

#include "yamlconvert.h"
#include "fattributemodel.h"
#include "flanguagemodel.h"
#include "util.h"

FAttribute::~FAttribute()
{
    QHash<QString, FAttributeText*>::iterator it;
    for (it = m_text.begin(); it != m_text.end(); ++it) {
        delete (*it);
    }
}

void FAttribute::addText(const QString &countryCode, FAttributeText *attributeText)
{
    if (m_text.contains(countryCode)) {
        delete m_text[countryCode];
    }
    m_text.insert(countryCode, attributeText);
}

void FAttribute::addCharacteristic(const QString &stringId)
{
    if (!FWillCharacteristicModel::Instance()) return;
    if (FWillCharacteristicModel::Instance()->data()->contains(stringId)) {
        const FWillCharacteristic *characteristic = static_cast<const FWillCharacteristic*>(FWillCharacteristicModel::Instance()->value(stringId));
        m_characteristics.push_back(characteristic);
    } else {
        qWarning(qUtf8Printable(QObject::tr("Characteristic '%1' not found while trying to add it to the attribute '%2'.").arg(stringId, name())));
    }
}

const QString FAttribute::shortName() const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        return m_text[languageModel->selectedLanguage()->countryCode()]->shortName();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        return m_text[languageModel->defaultLanguage()->countryCode()]->shortName();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2'.").arg(QObject::tr("Attribute"), m_stringId)));
    return QString();
}

const QString FAttribute::shortName(const QString &countryCode) const
{
    if (m_text.contains(countryCode)) {
        return m_text[countryCode]->shortName();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("Attribute"), m_stringId, countryCode)));
    return QString();
}

const QString FAttribute::name() const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        return m_text[languageModel->selectedLanguage()->countryCode()]->name();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        return m_text[languageModel->defaultLanguage()->countryCode()]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2'.").arg(QObject::tr("Attribute"), m_stringId)));
    return QString();
}

const QString FAttribute::name(const QString &countryCode) const
{
    if (m_text.contains(countryCode)) {
        return m_text[countryCode]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("Attribute"), m_stringId, countryCode)));
    return QString();
}

const QString FAttribute::altName() const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        return m_text[languageModel->selectedLanguage()->countryCode()]->altName();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        return m_text[languageModel->defaultLanguage()->countryCode()]->altName();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2'.").arg(QObject::tr("Attribute"), m_stringId)));
    return QString();
}

const QString FAttribute::altName(const QString &countryCode) const
{
    if (m_text.contains(countryCode)) {
        return m_text[countryCode]->altName();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("Attribute"), m_stringId, countryCode)));
    return QString();
}

const FAttributeModel* FAttributeModel::m_instance = nullptr;

FAttributeModel::FAttributeModel(QObject *parent) : FAbstractYAMLModel(parent)
{
    m_defaultAttribute = nullptr;
    beginLoading();
    load();
    loadText(FLanguageModel::Instance()->defaultLanguage()->countryCode());
    if (FLanguageModel::Instance()->defaultLanguage()->id() != FLanguageModel::Instance()->selectedLanguage()->id()) {
        loadText(FLanguageModel::Instance()->selectedLanguage()->countryCode());
    }
    endLoading();
}

void FAttributeModel::load()
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
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Attribute"), m_yamlFile)));
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
            const YAML::Node& attributeNode = *it;
            if (!attributeNode["Id"] || !attributeNode["Id"].IsScalar()) {
                qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected '%2' with type '%3'.").arg(m_yamlFile, "Id", "Scalar")));
                continue;
            }

            QString attributeId = attributeNode["Id"].as<QString>();

            QString iconPath = "";
            if (attributeNode["Icon"] && attributeNode["Icon"].IsScalar()) {
                iconPath = attributeNode["Icon"].as<QString>();
            }

            QColor color = QColor(0,0,0,0);
            if (attributeNode["Color"] && attributeNode["Color"].IsSequence() && attributeNode["Color"].size() == 4) {
                color.setRed(attributeNode["Color"][0].as<int>());
                color.setGreen(attributeNode["Color"][1].as<int>());
                color.setBlue(attributeNode["Color"][2].as<int>());
                color.setAlpha(attributeNode["Color"][3].as<int>());
            }

            QColor color2 = QColor(0,0,0,0);
            if (attributeNode["Color2"] && attributeNode["Color2"].IsSequence() && attributeNode["Color2"].size() == 4) {
                color2.setRed(attributeNode["Color2"][0].as<int>());
                color2.setGreen(attributeNode["Color2"][1].as<int>());
                color2.setBlue(attributeNode["Color2"][2].as<int>());
                color2.setAlpha(attributeNode["Color2"][3].as<int>());
            }

            bool defaultAttrib = false;
            if (attributeNode["IsDefault"] && attributeNode["IsDefault"].IsScalar()) {
                defaultAttrib = attributeNode["IsDefault"].as<bool>();
            }

            bool exclusiveAttrib = false;
            if (attributeNode["IsExclusive"] && attributeNode["IsExclusive"].IsScalar()) {
                exclusiveAttrib = attributeNode["IsExclusive"].as<bool>();
            }

            int maxCost = MAX_CARD_COST;
            bool genericAttrib = false;

            YAML::Node costNode;
            if (attributeNode["Cost"] && attributeNode["Cost"].IsMap()) {
                costNode = attributeNode["Cost"];

                if (costNode["IsGeneric"] && costNode["IsGeneric"].IsScalar()) {
                    genericAttrib = costNode["IsGeneric"].as<bool>();
                    if (genericAttrib) {
                        maxCost = MAX_CARD_COST_GENERIC;
                    }
                }

                if (costNode["Max"] && costNode["Max"].IsScalar()) {
                    maxCost = costNode["Max"].as<int>();
                }

            }

            FAttribute *attribute = new FAttribute(i, attributeId, iconPath, color, color2, exclusiveAttrib, genericAttrib, maxCost);
            insert(attributeId, attribute);

            if (!costNode.IsNull()) {
                if (costNode["Characteristics"] && costNode["Characteristics"].IsSequence()) {
                    const YAML::Node cnode = costNode["Characteristics"];
                    if (cnode.size() > 0) {
                        for (YAML::const_iterator cit = cnode.begin(); cit != cnode.end(); ++cit) {
                            if ((*cit).IsScalar()) {
                                QString characteristicId = (*cit).as<QString>();
                                attribute->addCharacteristic(characteristicId);
                            }
                        }
                    }
                }
            }

            if (defaultAttrib) {
                m_defaultAttribute = attribute;
            }

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Attribute"), m_yamlFile)));
        return;
    }

    if (!m_defaultAttribute) {
        // Because we throw an error if no entry exists, first() should always return a value
        FAttribute *attribute = static_cast<FAttribute*>(dataVec()->first());
        m_defaultAttribute = attribute;
    }
}

void FAttributeModel::loadText(const QString &countryCode)
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
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("Attribute"), m_yamlFile)));
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
                qWarning(qUtf8Printable(QObject::tr("Found text whose parent %1 does not exist when loading '%2'. (%3)").arg(QObject::tr("Attribute"), m_yamlFile, textId)));
                continue;
            }

            QString shortName = "";
            if (textNode["Short"] && textNode["Short"].IsScalar()) {
                shortName = textNode["Short"].as<QString>();
            }

            QString name = "";
            if (textNode["Name"] && textNode["Name"].IsScalar()) {
                name = textNode["Name"].as<QString>();
            }

            QString altName = "";
            if (textNode["Alt"] && textNode["Alt"].IsScalar()) {
                altName = textNode["Alt"].as<QString>();
            }

            FAttributeText *attributeText = new FAttributeText(shortName, name, altName);

            FAttribute *attribute = value_p(textId);
            if (attribute) {
                attribute->addText(countryCode, attributeText);
            }

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("Attribute"), m_yamlFile)));
        return;
    }
}

const FAttribute *FAttributeModel::get(const QString &stringId) const
{
    if (m_data.contains(stringId)) {
        const FAttribute *attribute = static_cast<const FAttribute*>(m_data[stringId]);
        return attribute;
    }
    return nullptr;
}

const FAttribute *FAttributeModel::get(int id) const
{
    if (id < dataCount() && id >= 0) {
        const FAttribute *attribute = static_cast<const FAttribute*>(m_vec[id]);
        return attribute;
    }
    return nullptr;
}

FAttribute *FAttributeModel::value_p(const QString &stringId)
{
    if (m_data.contains(stringId)) {
        FAbstractObject *attribute = m_data.value(stringId);
        return static_cast<FAttribute*>(attribute);
    } else {
        return nullptr;
    }
}
