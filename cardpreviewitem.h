#ifndef CARDPREVIEWITEM_H
#define CARDPREVIEWITEM_H

#include <QGraphicsObject>
#include <QLinearGradient>
#include "models/fraritymodel.h"
#include "text/fgraphicstextitem.h"
#include "dialogs/optionswindow.h"
#include "card.h"

#define COLOR_GRADIENT_SQUISH_FACTOR 0.2 // Smaller means stronger squished

// Positions
#define BORDER_X  18
#define BORDER_Y   18

#define STATS_BOX_Y 363

#define NAME_BOX_NO_COST_X 116
#define NAME_BOX_NO_COST_Y 98
#define NAME_BOX_X 268
#define NAME_BOX_Y 72
#define NAME_BOX_RIGHT_OFFSET 86
#define NAME_BOX_NO_COST_LEFTMARGIN 100
#define NAME_BOX_H_MARGIN 20

#define COST_WHEEL_X 21
#define COST_WHEEL_Y 25

#define FOOTER_BOX_X 111
#define FOOTER_BOX_BOT_OFFSET 88

#define TEXT_BOX_X FOOTER_BOX_X
#define TEXT_BOX_Y 1264
#define TEXT_BOX_TOP_HEIGHT 96
#define TEXT_BOX_SMALL_Y 1536
#define TEXT_BOX_FLAVOR_HEIGHT 100
#define TEXT_BOX_ATTRIBUTE_SIZE 72
#define TEXT_BOX_ATTRIBUTE_STEP (TEXT_BOX_ATTRIBUTE_SIZE + TEXT_BOX_ATTRIBUTE_SIZE/2)

class CardPreviewItem : public QGraphicsObject
{
    Q_OBJECT
public:
    CardPreviewItem(const Card *card = nullptr, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void loadPixmaps();
    void setCard(const Card *card);

    const FGraphicsTextItem *cardNameItem() const { return textCardname; }
    const FGraphicsTextItem *abilitiesItem() const { return textAbilities; }

public slots:
    void changeRarity(const FRarity *rarity);
    void changeCardType(const FCardType *cardType);
    void changeGeneralCardType(const FGeneralCardType *generalCardType);
    void changeAttribute(const FAttribute *attribute);
    void changeCardName(const FLanguageString cardName);
    void changeFlavorText(const FLanguageString flavorText);
    void changeTrait();
    void changeAbilityText(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void removeAbilityText();

    void showCost(bool showCost = true, bool doUpdate = true);
    void showStats(bool showStats = false, bool doUpdate = true);
    void showSmallTextBox(bool showSmallTextBox = false, bool doUpdate = true);
    void showBorder(bool showBorder = true, bool doUpdate = true);
    void showTextBox(bool showTextBox = true, bool doUpdate = true);
    void showQuickcast(bool showQuickcast = true, bool doUpdate = true);

    void setTextItemFont(OptionsWindow::FontUpdateType type, const QFont &font);

    void redraw(QRectF rect = QRectF());

private:
    // Pixmaps
    QPixmap cornerTL;
    QPixmap cornerTR;
    QPixmap costWheel;
    QPixmap nameBoxL;
    QPixmap nameBoxR;
    QPixmap nameBoxM;
    QPixmap borderVertical;
    QPixmap borderHorizontal;
    QPixmap footerBoxL;
    QPixmap footerBoxR;
    QPixmap footerBoxM;
    QPixmap statsBox;
    QMap<int, QPixmap> attributes;

    QPixmap dummy;
    QRect dummyScale;

    QRect borderTopRect;
    QRect borderRightRect;
    QRect nameBoxRect;
    QRect nameTextBoxRect; // adds padding
    QRect footerBoxRect;
    QRect borderLeftRect;
    QRect borderLeftTopRect; // Above stats box
    QRect borderLeftBotRect; // Below stats box
    QRect textBoxRect;
    QRect textBoxRectInner; // adds padding
    QRect textBoxTopRectInner;
    QRect textFlavorBox;

    QPainterPath cardPath;
    QPainterPath textBoxPath;
    QPixmap cardBackground;

    QPointF borderLeftBotOffset;
    QPointF textBoxAttributeStartOffset;

    QLinearGradient attributeNameGradient;
    QLinearGradient attributeTextBoxGradient;

    FGraphicsTextItem *textCardname;
    FGraphicsTextItem *textAbilities;
    FGraphicsTextItem *textCardtype;
    FGraphicsTextItem *textFlavor;
    //FGraphicsTextItem *textAttributes;

    const Card *m_card;

    void updateAttributeGradient();
    const QString generateCardTypeText();
};

#endif // CARDPREVIEWITEM_H
