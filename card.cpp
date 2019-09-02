#include "card.h"

Card::Card(QObject *parent, const FAttributeModel *attributeModel, const FLanguageModel *languageModel)
    : QObject(parent), m_showStats(true), m_showCost(true), m_showSmallTextBox(false), m_showBorder(true), m_showTextBox(true), m_showQuickcast(false)
{
    m_willCostModel = new WillCostModel(this, attributeModel);
    m_traitModel = new LangStringListModel(QObject::tr("Race/Trait"), languageModel);
    m_abilityTextModel = new LangStringListModel(QObject::tr("Ability"), languageModel);
    m_rarity = FRarityModel::Instance()->get(0);
}

void Card::setShowStats(bool showStats)
{
    m_showStats = showStats;
    emit showStatsChanged(showStats);
}

void Card::setShowCost(bool showCost)
{
    m_showCost = showCost;
    emit showCostChanged(showCost);
}

void Card::setShowSmallTextBox(bool showSmallTextBox)
{
    m_showSmallTextBox = showSmallTextBox;
    emit showSmallTextBoxChanged(showSmallTextBox);
}

void Card::setShowBorder(bool showBorder)
{
    m_showBorder = showBorder;
    emit showBorderChanged(showBorder);
}

void Card::setShowTextBox(bool showTextBox)
{
    m_showTextBox = showTextBox;
    emit showTextBoxChanged(showTextBox);
}

void Card::setShowQuickcast(bool showQuickcast)
{
    m_showQuickcast = showQuickcast;
    emit showQuickcastChanged(showQuickcast);
}

void Card::updateCanFight()
{
    bool canFight = false;
    QMap<int, const FCardType*> data = m_cardTypes;
    QMap<int, const FCardType*>::const_iterator iter = data.constBegin();
    for (; iter != data.constEnd(); ++iter) {
        canFight |= (*iter)->canFight();
    }
    m_canFight = canFight;
}

void Card::updateHasDivinity()
{
    bool hasDivinity = false;
    //QVector<const FCardType*> data = m_cardTypes;
    QMap<int, const FCardType*> data = m_cardTypes;
    QMap<int, const FCardType*>::const_iterator iter = data.constBegin();
    for (; iter != data.constEnd(); ++iter) {
        hasDivinity |= (*iter)->hasDivinity();
    }
    m_hasDivinity = hasDivinity;
}

void Card::updateHasCost()
{
    bool hasCost = false;
    QMap<int, const FCardType*> data = m_cardTypes;
    QMap<int, const FCardType*>::const_iterator iter = data.constBegin();
    for (; iter != data.constEnd(); ++iter) {
        hasCost |= (*iter)->hasCost();
    }
    m_hasCost = hasCost;
}

void Card::updateCardOptions()
{
    bool hasCost = false;
    bool hasDivinity = false;
    bool canFight = false;
    QMap<int, const FCardType*> data = m_cardTypes;
    QMap<int, const FCardType*>::const_iterator iter = data.constBegin();
    for (; iter != data.constEnd(); ++iter) {
        hasCost |= (*iter)->hasCost();
        hasDivinity |= (*iter)->hasDivinity();
        canFight |= (*iter)->canFight();
    }
    m_hasCost = hasCost;
    m_hasDivinity = hasDivinity;
    m_canFight = canFight;
}

void Card::setRarity(const FRarity *rarity)
{
    if (rarity != m_rarity) {
        m_rarity = rarity;
        emit rarityChanged(rarity);
    }
}

void Card::addAttribute(const FAttribute *attribute)
{
    if (m_attributes.contains(attribute->id())) return;
    m_attributes.insert(attribute->id(), attribute);
    emit attributeChanged(attribute);
}

void Card::removeAttribute(const FAttribute *attribute)
{
    if (m_attributes.remove(attribute->id())) {
        emit attributeChanged(attribute);
    }
}

void Card::addCardType(int key, const FCardType *cardType)
{
    m_cardTypes.insert(key, cardType);
    updateCardOptions();
    emit cardTypeChanged(cardType);
}

void Card::replaceCardType(int key, const FCardType *cardType)
{
    if (key < m_cardTypes.size() && key >= 0) {
        m_cardTypes[key] = cardType;
        updateCardOptions();
        emit cardTypeChanged(cardType);
    }
}

void Card::removeCardType(int key)
{
    if (m_cardTypes.contains(key)) {
        const FCardType *ct = m_cardTypes[key];
        m_cardTypes.remove(key);
        updateCardOptions();
        emit cardTypeChanged(ct);
    }
}

void Card::addGeneralCardType(int key, const FGeneralCardType *generalCardType)
{
    m_generalCardTypes.insert(key, generalCardType);
    updateCardOptions();
    emit generalCardTypeChanged(generalCardType);
}

void Card::replaceGeneralCardType(int key, const FGeneralCardType *generalCardType)
{
    if (key < m_generalCardTypes.size() && key >= 0) {
        m_generalCardTypes[key] = generalCardType;
        updateCardOptions();
        emit generalCardTypeChanged(generalCardType);
    }
}

void Card::removeGeneralCardType(int key)
{
    if (m_generalCardTypes.contains(key)) {
        const FGeneralCardType *gct = m_generalCardTypes[key];
        m_generalCardTypes.remove(key);
        updateCardOptions();
        emit generalCardTypeChanged(gct);
    }
}
