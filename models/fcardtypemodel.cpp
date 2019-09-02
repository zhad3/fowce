#include <yaml-cpp/yaml.h>

#include "yamlconvert.h"
#include "fcardtypemodel.h"
#include "flanguagemodel.h"

const QString FCardTypeText::name(const QString &generalCardTypeId)
{
    if (m_combinedName.contains(generalCardTypeId)) {
        return m_combinedName[generalCardTypeId];
    }
    return QString();
}

void FCardTypeText::addCombinedName(const QString &typeId, const QString &name)
{
    m_combinedName.insert(typeId, name);
}

FCardType::~FCardType()
{
    QHash<QString, FCardTypeText*>::iterator it;
    for (it = m_text.begin(); it != m_text.end(); ++it) {
        delete(*it);
    }
}

void FCardType::addText(const QString &countryCode, FCardTypeText *cardTypeText)
{
    if (m_text.contains(countryCode)) {
        delete m_text[countryCode];
    }
    m_text.insert(countryCode, cardTypeText);
}

void FCardType::addGeneralCardType(const QString &stringId)
{
    if (!FGeneralCardTypeModel::Instance()) return;
    if (FGeneralCardTypeModel::Instance()->data()->contains(stringId)) {
        const FGeneralCardType *generalCardType = FGeneralCardTypeModel::Instance()->get(stringId);
        m_generalCardTypes.push_back(generalCardType);
    } else {
        qWarning(qUtf8Printable(QObject::tr("Characteristic '%1' not found while trying to add it to the attribute '%2'.").arg(stringId, name())));
    }
}

const QString FCardType::name() const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    QString countryCode = QString();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        countryCode = languageModel->selectedLanguage()->countryCode();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        countryCode = languageModel->defaultLanguage()->countryCode();
    }
    return name(countryCode);
}

const QString FCardType::name(const QString &countryCode) const
{
    if (!countryCode.isNull() && m_text.contains(countryCode)) {
        return m_text[countryCode]->name();
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("Card Type"), m_stringId, countryCode)));
    return QString();
}

const QString FCardType::nameCombined(const QString &generalCardTypeId) const
{
    const FLanguageModel *languageModel = FLanguageModel::Instance();
    QString countryCode = QString();
    if (m_text.contains(languageModel->selectedLanguage()->countryCode())) {
        countryCode = languageModel->selectedLanguage()->countryCode();
    } else if (m_text.contains(languageModel->defaultLanguage()->countryCode())) {
        countryCode = languageModel->defaultLanguage()->countryCode();
    }
    return nameCombined(countryCode, generalCardTypeId);
}

const QString FCardType::nameCombined(const QString &countryCode, const QString &generalCardTypeId) const
{
    if (!countryCode.isNull() && m_text.contains(countryCode)) {
        if (generalCardTypeId.isNull()) {
            return name(countryCode);
        } else {
            const QString combinedText = m_text[countryCode]->name(generalCardTypeId);
            if (combinedText.isNull()) {
                // No entry, we use the default concatenation of the two type strings
                const FGeneralCardType *gct = FGeneralCardTypeModel::Instance()->get(generalCardTypeId);
                if (!gct) {
                    // Invalid data
                    return m_text[countryCode]->name();
                }
                return gct->name(countryCode) + QString(" ") + m_text[countryCode]->name();
            } else {
                return combinedText;
            }
        }
    }
    qWarning(qUtf8Printable(QObject::tr("No text entry found for %1 '%2' with country code '%3'.").arg(QObject::tr("Card Type"), m_stringId, countryCode)));
    return QString();
}

const FCardTypeModel* FCardTypeModel::m_instance = nullptr;

FCardTypeModel::FCardTypeModel(QObject *parent) : FAbstractYAMLModel(parent)
{
    beginLoading();
    load();
    loadText(FLanguageModel::Instance()->defaultLanguage()->countryCode());
    if (FLanguageModel::Instance()->defaultLanguage()->id() != FLanguageModel::Instance()->selectedLanguage()->id()) {
        loadText(FLanguageModel::Instance()->selectedLanguage()->countryCode());
    }
    endLoading();
}

void FCardTypeModel::load()
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
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 in file '%2'.").arg(QObject::tr("Card Type"), m_yamlFile)));
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
            const YAML::Node& cardTypeNode = *it;
            if (!cardTypeNode["Id"] || !cardTypeNode["Id"].IsScalar()) {
                qWarning(qUtf8Printable(QObject::tr("Invalid node type when loading yaml file '%1'. Expected '%2' with type '%3'.").arg(m_yamlFile, "Id", "Scalar")));
                continue;
            }

            QString cardTypeId = cardTypeNode["Id"].as<QString>();

            bool canFight = false;
            if (cardTypeNode["CanFight"] && cardTypeNode["CanFight"].IsScalar()) {
                canFight = cardTypeNode["CanFight"].as<bool>();
            }

            bool hasDivinity = false;
            if (cardTypeNode["HasDivinity"] && cardTypeNode["HasDivinity"].IsScalar()) {
                hasDivinity = cardTypeNode["HasDivinity"].as<bool>();
            }

            bool hasCost = true;
            if (cardTypeNode["HasCost"] && cardTypeNode["HasCost"].IsScalar()) {
                hasCost = cardTypeNode["HasCost"].as<bool>();
            }

            FCardType *cardType = new FCardType(i, cardTypeId, canFight, hasDivinity, hasCost);
            insert(cardTypeId, cardType);

            if (cardTypeNode["GeneralTypes"] && cardTypeNode["GeneralTypes"].IsSequence()) {
                const YAML::Node gtNode = cardTypeNode["GeneralTypes"];
                if (gtNode.size() > 0) {
                    YAML::const_iterator gtit;
                    for (gtit = gtNode.begin(); gtit != gtNode.end(); ++gtit) {
                        if ((*gtit).IsScalar()) {
                            QString generalCardTypeId = (*gtit).as<QString>();
                            cardType->addGeneralCardType(generalCardTypeId);
                        }
                    }
                }
            }

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

void FCardTypeModel::loadText(const QString &countryCode)
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
                qWarning(qUtf8Printable(QObject::tr("Found text whose parent %1 does not exist when loading '%2'. (%3)").arg(QObject::tr("Card Type"), m_yamlFile, textId)));
                continue;
            }

            QString name = "";
            if (textNode["Name"] && textNode["Name"].IsScalar()) {
                name = textNode["Name"].as<QString>();
            }

            FCardTypeText *cardTypeText = new FCardTypeText(name);

            if (textNode["CombineType"] && textNode["CombineType"].IsSequence()) {
                const YAML::Node combineNode = textNode["CombineType"];
                if (combineNode.size() > 0) {
                    YAML::const_iterator cnit;
                    for (cnit = combineNode.begin(); cnit != combineNode.end(); ++cnit) {
                        if ((*cnit).IsMap()) {
                            if (!(*cnit)["TypeId"] || !(*cnit)["Name"]) {
                                continue;
                            }
                            QString typeId = (*cnit)["TypeId"].as<QString>();
                            QString combinedName = (*cnit)["Name"].as<QString>();
                            cardTypeText->addCombinedName(typeId, combinedName);
                        }
                    }
                }
            }

            FCardType *cardType = value_p(textId);
            if (cardType) {
                cardType->addText(countryCode, cardTypeText);
            }

            i++;

        } catch (YAML::Exception e) {
            qFatal(e.msg.data());
            return;
        }
    }

    // Entries exist, but none were valid
    if (i == 0) {
        qFatal(qUtf8Printable(QObject::tr("Need at least one %1 text in file '%2'.").arg(QObject::tr("Card Type"), m_yamlFile)));
        return;
    }
}

const FCardType *FCardTypeModel::get(const QString &stringId) const
{
    if (m_data.contains(stringId)) {
        const FCardType *cardType = static_cast<const FCardType*>(m_data[stringId]);
        return cardType;
    }
    return nullptr;
}

const FCardType *FCardTypeModel::get(int id) const
{
    if (id < dataCount() && id >= 0) {
        const FCardType *cardType = static_cast<const FCardType*>(m_vec[id]);
        return cardType;
    }
    return nullptr;
}

FCardType *FCardTypeModel::value_p(const QString &stringId)
{
    if (m_data.contains(stringId)) {
        FAbstractObject *cardType = m_data.value(stringId);
        return static_cast<FCardType*>(cardType);
    } else {
        return nullptr;
    }
}
