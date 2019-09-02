#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QGraphicsObject>
#include "models/fcardtypemodel.h"
#include "models/fgeneralcardtypemodel.h"
#include "models/fattributemodel.h"
#include "models/flanguagestring.h"
#include "models/fraritymodel.h"
#include "willcostmodel.h"
#include "langstringlistmodel.h"
#include "util.h"

class Card : public QObject
{
    Q_OBJECT
public:
    Card(QObject *parent = nullptr, const FAttributeModel* attributeModel = nullptr, const FLanguageModel *languageModel = nullptr);

    const QMap<int, const FCardType*> cardTypes() const { return m_cardTypes; }
    const QMap<int, const FGeneralCardType*> generalCardTypes() const { return m_generalCardTypes; }
    const QMap<int, const FAttribute*> attributes() const { return m_attributes; }
    const FRarity *rarity() const { return m_rarity; }
    WillCostModel* willCostModel() { return m_willCostModel; }
    LangStringListModel *traitModel() const { return m_traitModel; }
    LangStringListModel *abilityTextModel() const { return m_abilityTextModel; }
    const FLanguageString cardName() const { return m_cardName; }
    const FLanguageString flavorText() const { return m_flavorText; }

    bool hasCardType(const FCardType *cardType) const { return m_cardTypes.key(cardType, -1) != -1; }
    bool hasGeneralCardType(const FGeneralCardType *generalCardType) const { return m_generalCardTypes.key(generalCardType, -1) != -1; }

    void setWillCostModel(WillCostModel *willCostModel) { m_willCostModel = willCostModel; }
    void setTraitModel(LangStringListModel *traitModel) { m_traitModel = traitModel; }
    void setAbilityTextModel(LangStringListModel *abilityTextModel) { m_abilityTextModel = abilityTextModel; }

    bool canFight() const { return m_canFight; }
    bool hasDivinity() const { return m_hasDivinity; }
    bool hasCost() const { return m_hasCost; }

    void updateCanFight();
    void updateHasDivinity();
    void updateHasCost();

    void updateCardOptions();

    // Art
    bool showStats() const { return m_showStats; }
    bool showCost() const { return m_showCost; }
    bool showSmallTextBox() const { return m_showSmallTextBox; }
    bool showBorder() const { return m_showBorder; }
    bool showTextBox() const { return m_showTextBox; }
    bool showQuickcast() const { return m_showQuickcast; }

public slots:
    void setRarity(const FRarity *rarity);
    void setCardName(FLanguageString cardName) { m_cardName = cardName; emit cardNameChanged(cardName); }
    void setFlavorText(FLanguageString flavorText) { m_flavorText = flavorText; emit flavorTextChanged(flavorText); }

    void setShowStats(bool showStats);
    void setShowCost(bool showCost);
    void setShowSmallTextBox(bool showSmallTextBox);
    void setShowBorder(bool showBorder);
    void setShowTextBox(bool showTextBox);
    void setShowQuickcast(bool showQuickcast);

    void addAttribute(const FAttribute *attribute);
    void removeAttribute(const FAttribute *attribute);

    void addCardType(int key, const FCardType *cardType);
    void replaceCardType(int key, const FCardType *cardType);
    void removeCardType(int key);

    void addGeneralCardType(int key, const FGeneralCardType *generalCardType);
    void replaceGeneralCardType(int key, const FGeneralCardType *generalCardType);
    void removeGeneralCardType(int key);

signals:
    void rarityChanged(const FRarity *rarity);
    void cardTypeChanged(const FCardType *cardType);
    void generalCardTypeChanged(const FGeneralCardType *generalCardType);
    void attributeChanged(const FAttribute *attribute);
    void cardNameChanged(const FLanguageString cardName);
    void flavorTextChanged(const FLanguageString flavorText);

    void showCostChanged(bool showCost = true, bool doUpdate = true);
    void showStatsChanged(bool showStats = true, bool doUpdate = true);
    void showSmallTextBoxChanged(bool showSmallTextBox = true, bool doUpdate = true);
    void showBorderChanged(bool showBorder = true, bool doUpdate = true);
    void showTextBoxChanged(bool showTextBox = false, bool doUpdate = true);
    void showQuickcastChanged(bool showQuickcast = false, bool doUpdate = true);

    void redraw(QRectF rect = QRectF());

private:
    QMap<int, const FCardType*> m_cardTypes;
    QMap<int, const FGeneralCardType*> m_generalCardTypes;
    QMap<int, const FAttribute*> m_attributes;
    const FRarity *m_rarity;
    WillCostModel *m_willCostModel;
    LangStringListModel *m_traitModel;
    LangStringListModel *m_abilityTextModel;
    FLanguageString m_cardName;
    FLanguageString m_flavorText;

    bool m_canFight;
    bool m_hasDivinity;
    bool m_hasCost;

    // Art
    bool m_showStats;
    bool m_showCost;
    bool m_showSmallTextBox;
    bool m_showBorder;
    bool m_showTextBox;
    bool m_showQuickcast;
};

#endif // CARD_H
