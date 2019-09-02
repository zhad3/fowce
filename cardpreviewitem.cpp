#include <QPainter>
#include <QIcon>
#include <QSettings>
#include "cardpreviewitem.h"
#include "models/fraritymodel.h"
#include "util.h"

CardPreviewItem::CardPreviewItem(const Card *card, QGraphicsItem *parent) : QGraphicsObject(parent)
{
    m_card = card;
    if (m_card) {
        QObject::connect(m_card, &Card::rarityChanged, this, &CardPreviewItem::changeRarity);
        QObject::connect(m_card, &Card::cardTypeChanged, this, &CardPreviewItem::changeCardType);
        QObject::connect(m_card, &Card::generalCardTypeChanged, this, &CardPreviewItem::changeGeneralCardType);
        QObject::connect(m_card, &Card::attributeChanged, this, &CardPreviewItem::changeAttribute);
        QObject::connect(m_card, &Card::cardNameChanged, this, &CardPreviewItem::changeCardName);
        QObject::connect(m_card, &Card::flavorTextChanged, this, &CardPreviewItem::changeFlavorText);
        QObject::connect(m_card, &Card::showCostChanged, this, &CardPreviewItem::showCost);
        QObject::connect(m_card, &Card::showStatsChanged, this, &CardPreviewItem::showStats);
        QObject::connect(m_card, &Card::showSmallTextBoxChanged, this, &CardPreviewItem::showSmallTextBox);
        QObject::connect(m_card, &Card::showBorderChanged, this, &CardPreviewItem::showBorder);
        QObject::connect(m_card, &Card::showTextBoxChanged, this, &CardPreviewItem::showTextBox);
        QObject::connect(m_card, &Card::showQuickcastChanged, this, &CardPreviewItem::showQuickcast);
        QObject::connect(m_card, &Card::redraw, this, &CardPreviewItem::redraw);

        QObject::connect(m_card->traitModel(), &LangStringListModel::dataChanged, this, &CardPreviewItem::changeTrait);
        QObject::connect(m_card->traitModel(), &LangStringListModel::rowsRemoved, this, &CardPreviewItem::changeTrait);
        QObject::connect(m_card->abilityTextModel(), &LangStringListModel::dataChanged, this, &CardPreviewItem::changeAbilityText);
        QObject::connect(m_card->abilityTextModel(), &LangStringListModel::rowsRemoved, this, &CardPreviewItem::removeAbilityText);
    }

    // Create white card background
    cardBackground = QPixmap(int(boundingRect().width()), int(boundingRect().height()));
    cardBackground.fill(Qt::transparent);
    cardPath.addRoundedRect(boundingRect(), 50, 50);
    QPainter painter(&cardBackground);
    painter.setPen(Qt::white);
    painter.drawPath(cardPath);
    painter.fillPath(cardPath, Qt::white);
    painter.end();

    loadPixmaps();

    QTextOption opt;
    opt.setAlignment(Qt::AlignCenter);

    textCardname = new FGraphicsTextItem(this);
    textAbilities = new FGraphicsTextItem(this);
    textCardtype = new FGraphicsTextItem(this);
    textFlavor = new FGraphicsTextItem(this);

    // Testing
    QSettings settings;

    textCardname->document()->setDefaultTextOption(opt);
    textCardname->setDefaultTextColor(Qt::white);
    QFont cardNameFont;
    cardNameFont.fromString(settings.value("font/cardname", "Times New Roman").value<QString>());
    cardNameFont.setWeight(QFont::Bold);
    cardNameFont.setPointSize(52);
    cardNameFont.setLetterSpacing(QFont::PercentageSpacing, 105);
    cardNameFont.setStyleStrategy(QFont::StyleStrategy::NoAntialias);
    textCardname->setFont(cardNameFont);
    textCardname->setTargetRect(nameTextBoxRect);
    textCardname->setOutlinePen(QPen(Qt::black, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    textCardname->showOutline(true);
    if (m_card && !m_card->cardName().text().isEmpty()) {
        textCardname->setText(m_card->cardName().text());
    }

    QFont textCardtypeFont;
    textCardtypeFont.fromString(settings.value("font/cardtype", "Times New Roman").value<QString>());
    //textCardtypeFont.setWeight(QFont::DemiBold);
    textCardtypeFont.setPointSize(36);
    textCardtypeFont.setLetterSpacing(QFont::PercentageSpacing, 105);
    textCardtypeFont.setStyleStrategy(QFont::StyleStrategy::NoAntialias);
    textCardtype->setFont(textCardtypeFont);
    textCardtype->setTargetRect(textBoxTopRectInner);
    textCardtype->setText(generateCardTypeText());

    QFont textAbilitiesFont;
    textAbilitiesFont.fromString(settings.value("font/abilities", "Ryo Text PlusN M").value<QString>());
    //textAbilitiesFont.setWeight(QFont::DemiBold);
    textAbilitiesFont.setPointSize(38);
    textAbilitiesFont.setLetterSpacing(QFont::PercentageSpacing, 105);
    textAbilitiesFont.setStyleStrategy(QFont::StyleStrategy::NoAntialias);
    textAbilities->setFitToRectOrder(FGraphicsTextItem::FitToRectOrder::SizeSpacingStretch);
    textAbilities->setFont(textAbilitiesFont);
    textAbilities->setTargetRect(textBoxRectInner);
    //textAbilities->setOutlinePen(QPen(Qt::white, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    textAbilities->insertTextBlock("[Enter] ⇨ Produce [w][w].");
//    textAbilities->insertTextBlock("Other light resonators you control gain [+200/+200]. As long as there are four or more runes revealed from your rune area, they gain [+400/+400] instead.");
//    textAbilities->insertTextBlock("[Judgement][u][u][1][2][3][4][5][6][7][8][9]");
//    textAbilities->insertTextBlock("[Energize][u]");
//    textAbilities->insertTextBlock("The weather is rain during your turn.");
//    textAbilities->insertTextBlock("[rest]: Search your deck for a card named \"Weather Change: Rain\", reveal it and put it into your hand. Then shuffle your deck.");
//    textAbilities->insertTextBlock("《Thunder Parasol》 [0]: Choose one: Play this ability only during your turn and only once per turn - Put an electricity counter on this card; or remove an electricity counter from this card. If you do, the weather is thunderstorm until end of turn.");
//    textAbilities->checkUpdate();

    QFont textFlavorFont;
    textFlavorFont.fromString(settings.value("font/flavor", "Times New Roman").value<QString>());
    textFlavorFont.setPointSize(32);
    textFlavorFont.setLetterSpacing(QFont::PercentageSpacing, 90);
    textFlavorFont.setStyleStrategy(QFont::StyleStrategy::NoAntialias);
    textFlavor->document()->setDefaultTextOption(opt);
    textFlavor->setFont(textFlavorFont);
    textFlavor->setTargetRect(textFlavorBox);
    // ===
}


QRectF CardPreviewItem::boundingRect() const
{
    //return QRectF(0, 0, 609, 850); // Low-res
    return QRectF(0, 0, 1466, 2048); // High-res
}

void CardPreviewItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QPen p = painter->pen();
    QBrush b = painter->brush();
    //QPainter::CompositionMode compMode = painter->compositionMode();

    painter->drawPixmap(0, 0, cardBackground);
    painter->setClipPath(cardPath);

    painter->drawPixmap(dummyScale, dummy); // TODO STRETCH

    // Border
    if (m_card && m_card->showBorder()) {
        painter->drawPixmap(BORDER_X, BORDER_Y, cornerTL);
        painter->drawPixmap(int(boundingRect().width()) - cornerTR.width() - BORDER_X , BORDER_Y, cornerTR);
        painter->drawTiledPixmap(borderTopRect, borderHorizontal);

        painter->drawTiledPixmap(borderRightRect, borderVertical);

        if (m_card->showStats()) {
            painter->drawTiledPixmap(borderLeftTopRect, borderVertical);
            painter->drawTiledPixmap(borderLeftBotRect, borderVertical, borderLeftBotOffset);
        } else {
            painter->drawTiledPixmap(borderLeftRect, borderVertical);
        }
    }
    if (m_card->showStats()) {
        painter->drawPixmap(0, STATS_BOX_Y, statsBox);
    }

    // Name
    if (m_card && m_card->showCost()) {

        //painter->setCompositionMode(QPainter::CompositionMode_Screen);
        painter->setPen(Qt::transparent);
        painter->setBrush(attributeNameGradient);

        painter->drawRect(NAME_BOX_X + 2, NAME_BOX_Y + 2, int(boundingRect().width()) - NAME_BOX_X - NAME_BOX_RIGHT_OFFSET - 4, nameBoxM.height() - 4);

        painter->setBrush(b);
        painter->setPen(p);
        //painter->setCompositionMode(compMode);

        painter->drawPixmap(NAME_BOX_X, NAME_BOX_Y, nameBoxL);
        painter->drawPixmap(int(boundingRect().width()) - nameBoxR.width() - NAME_BOX_RIGHT_OFFSET, NAME_BOX_Y, nameBoxR);
        painter->drawTiledPixmap(nameBoxRect, nameBoxM);

        painter->drawPixmap(COST_WHEEL_X, COST_WHEEL_Y, costWheel);
    } else {
        painter->setPen(Qt::transparent);
        painter->setBrush(attributeNameGradient);
        painter->drawRect(NAME_BOX_NO_COST_X + 2, NAME_BOX_NO_COST_Y + 2, int(boundingRect().width()) - (NAME_BOX_NO_COST_X * 2) - 4, nameBoxM.height() - 4);
        painter->setBrush(b);
        painter->setPen(p);

        painter->drawPixmap(NAME_BOX_NO_COST_X, NAME_BOX_NO_COST_Y, nameBoxL);
        painter->drawPixmap(int(boundingRect().width()) - nameBoxR.width() - NAME_BOX_NO_COST_X, NAME_BOX_NO_COST_Y, nameBoxR);
        painter->drawTiledPixmap(nameBoxRect, nameBoxM);
    }

    // Footer
    painter->setPen(Qt::transparent);
    painter->setBrush(QBrush(Util::BoxDefaultColor, Qt::SolidPattern));
    painter->drawRect(FOOTER_BOX_X + 2, footerBoxRect.y() + 2, int(boundingRect().width()) - (FOOTER_BOX_X * 2) - 4, footerBoxM.height());
    painter->setBrush(b);
    painter->setPen(p);
    painter->drawPixmap(FOOTER_BOX_X, footerBoxRect.y(), footerBoxL);
    painter->drawPixmap(int(boundingRect().width()) - footerBoxR.width() - FOOTER_BOX_X, footerBoxRect.y(), footerBoxR);
    painter->drawTiledPixmap(footerBoxRect, footerBoxM);

    // Textbox
    if (m_card && m_card->showTextBox()) {
        painter->setPen(Qt::transparent);
        painter->setBrush(attributeTextBoxGradient);
        painter->setClipPath(textBoxPath);
        // Top region
        painter->drawRect(textBoxRect.x(), textBoxRect.y(), textBoxRect.width(), TEXT_BOX_TOP_HEIGHT);
        // Bottom region
        painter->setBrush(QBrush(Util::TextBoxBottomRegionColor, Qt::SolidPattern));
        painter->drawRect(textBoxRect.x(), textBoxRect.y() + TEXT_BOX_TOP_HEIGHT, textBoxRect.width(), textBoxRect.height() - TEXT_BOX_TOP_HEIGHT);
        painter->setBrush(b);
    }

    // Draw attribute icons
    const QMap<int, const FAttribute*> data = m_card->attributes();
    QMap<int, const FAttribute*>::const_iterator it = data.constBegin();
    int step = 0;
    //painter->setBrush(Qt::white);
    //painter->setPen(QPen(Qt::white, 3.0));
    for (; it != data.constEnd(); ++it, step+=TEXT_BOX_ATTRIBUTE_STEP) {
        QPixmap attr = attributes[(*it)->id()];
        // Draw white outline
        //painter->drawEllipse(int(textBoxAttributeStartOffset.x()) + step, int(textBoxAttributeStartOffset.y()), 48, 48); // TODO UPDATE ICON SIZE
        painter->setOpacity(0.75);
        painter->drawPixmap(int(textBoxAttributeStartOffset.x()) + step, int(textBoxAttributeStartOffset.y()), attr);
        painter->setOpacity(1.0);
        if (Util::DrawDebugInfo) {
            painter->drawRect(QRect(int(textBoxAttributeStartOffset.x()) + step, int(textBoxAttributeStartOffset.y()), attr.size().width(), attr.size().height()));
        }
    }

    painter->setClipPath(cardPath);
    painter->setBrush(b);
    painter->setPen(p);

    if (Util::DrawDebugInfo) {
        // Debugging
        painter->drawRect(textBoxRectInner);
        painter->drawRect(textBoxTopRectInner);
    }
}

void CardPreviewItem::loadPixmaps()
{
    dummy = QPixmap(":/card_decoration/dummy.jpg");

    if (m_card && m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE")) {
        cornerTL = QPixmap(":/card_decoration/border-corner-superrare.png");
        borderHorizontal = QPixmap(":/card_decoration/border-horizontal-superrare.png");
        borderVertical = QPixmap(":/card_decoration/border-vertical-superrare.png");
        costWheel = QPixmap(":/card_decoration/cost-wheel-superrare.png");
        nameBoxL = QPixmap(":/card_decoration/name-box-superrare-left.png");
        nameBoxM = QPixmap(":/card_decoration/name-box-superrare-mid.png");
        statsBox = QPixmap(":/card_decoration/stats-box-superrare.png");
        footerBoxL = QPixmap(":/card_decoration/footer-box-superrare-left.png");
        footerBoxM = QPixmap(":/card_decoration/footer-box-superrare-mid.png");
    } else {
        if (m_card && m_card->rarity() == FRarityModel::Instance()->get("RARE")) {
            cornerTL = QPixmap(":/card_decoration/border-corner-rare.png");
        } else {
            cornerTL = QPixmap(":/card_decoration/border-corner-standard.png");
        }

        borderHorizontal = QPixmap(":/card_decoration/border-horizontal-standard.png");
        borderVertical = QPixmap(":/card_decoration/border-vertical-standard.png");
        costWheel = QPixmap(":/card_decoration/cost-wheel-standard.png");
        nameBoxL = QPixmap(":/card_decoration/name-box-standard-left.png");
        nameBoxM = QPixmap(":/card_decoration/name-box-standard-mid.png");
        statsBox = QPixmap(":/card_decoration/stats-box-standard.png");
        footerBoxL = QPixmap(":/card_decoration/footer-box-standard-left.png");
        footerBoxM = QPixmap(":/card_decoration/footer-box-standard-mid.png");
    }

    cornerTR = cornerTL.transformed(QTransform().scale(-1, 1));
    nameBoxR = nameBoxL.transformed(QTransform().scale(-1, 1));
    footerBoxR = footerBoxL.transformed(QTransform().scale(-1, 1));

    borderTopRect = QRect(
                cornerTL.width() + BORDER_X,
                BORDER_Y,
                int(boundingRect().width()) - cornerTR.width() - BORDER_X - (BORDER_X + cornerTL.width()),
                borderHorizontal.height());

    borderRightRect = QRect(
                int(boundingRect().width()) - borderVertical.width() - BORDER_X,
                cornerTR.height() + BORDER_Y,
                borderVertical.width(),
                int(boundingRect().height()) - cornerTR.height() - BORDER_Y);

    nameBoxRect = QRect(
                NAME_BOX_X + nameBoxL.width(),
                NAME_BOX_Y,
                int(boundingRect().width()) - nameBoxR.width() - NAME_BOX_RIGHT_OFFSET - (NAME_BOX_X + nameBoxL.width()),
                nameBoxM.height());

    nameTextBoxRect = QRect();
    nameTextBoxRect.setTopLeft(QPoint(nameBoxRect.left() + NAME_BOX_NO_COST_LEFTMARGIN, nameBoxRect.top()));
    nameTextBoxRect.setBottomRight(QPoint(nameBoxRect.right() - NAME_BOX_H_MARGIN, nameBoxRect.bottom()));

    borderLeftRect = QRect(
                BORDER_X,
                cornerTL.height() + BORDER_Y,
                borderVertical.width(),
                int(boundingRect().height()) - cornerTL.height() - BORDER_Y);

    borderLeftTopRect = QRect(
                BORDER_X,
                cornerTL.height() + BORDER_Y,
                borderVertical.width(),
                STATS_BOX_Y - (cornerTL.height() + BORDER_Y));

    borderLeftBotRect = QRect(
                BORDER_X,
                borderLeftTopRect.y() + borderLeftTopRect.height() + statsBox.height(),
                borderVertical.width(),
                int(boundingRect().height()) - (borderLeftTopRect.y() + borderLeftTopRect.height() + statsBox.height()));

    borderLeftBotOffset = QPointF(0, borderLeftBotRect.y() - (borderLeftRect.y() + borderVertical.height()));

    footerBoxRect = QRect(
                footerBoxL.width() + FOOTER_BOX_X,
                int(boundingRect().height()) - FOOTER_BOX_BOT_OFFSET,
                int(boundingRect().width()) - footerBoxR.width() - FOOTER_BOX_X - (footerBoxL.width() + FOOTER_BOX_X),
                FOOTER_BOX_BOT_OFFSET);

    textBoxRect = QRect(TEXT_BOX_X, TEXT_BOX_Y, int(boundingRect().width()) - (TEXT_BOX_X * 2), footerBoxRect.y() - TEXT_BOX_Y - 1);
    textBoxPath.addRoundedRect(textBoxRect, TEXT_BOX_TOP_HEIGHT / 4, TEXT_BOX_TOP_HEIGHT / 4);

    textBoxRectInner = QRect();
    textBoxRectInner.setTopLeft(QPoint(textBoxRect.left() + 40, textBoxRect.top() + 20 + TEXT_BOX_TOP_HEIGHT));
    textBoxRectInner.setBottomRight(QPoint(textBoxRect.right() - 40, textBoxRect.bottom() - TEXT_BOX_FLAVOR_HEIGHT));

    textBoxTopRectInner = QRect();
    textBoxTopRectInner.setTopLeft(QPoint(TEXT_BOX_X + 40, TEXT_BOX_Y));
    textBoxTopRectInner.setBottomRight(QPoint(textBoxRect.right() - 40, TEXT_BOX_Y + TEXT_BOX_TOP_HEIGHT));

    textFlavorBox = QRect();
    textFlavorBox.setTopLeft(QPoint(textBoxRect.left() + 40, textBoxRect.bottom() - TEXT_BOX_FLAVOR_HEIGHT));
    textFlavorBox.setBottomRight(QPoint(textBoxRect.right() - 40, textBoxRect.bottom()));

    textBoxAttributeStartOffset = QPointF(TEXT_BOX_X + textBoxRect.width(), TEXT_BOX_Y + TEXT_BOX_TOP_HEIGHT / 2 - TEXT_BOX_ATTRIBUTE_SIZE / 2);

    attributeNameGradient = QLinearGradient(nameBoxRect.topLeft(), nameBoxRect.topRight());
    attributeNameGradient.setColorAt(0, Util::BoxDefaultColor);
    attributeNameGradient.setColorAt(1, Util::BoxDefaultColor);

    attributeTextBoxGradient = QLinearGradient(textBoxRect.topLeft(), textBoxRect.topRight());
    attributeTextBoxGradient.setColorAt(0, Util::TextBoxTopRegionColor);
    attributeTextBoxGradient.setColorAt(1, Util::TextBoxTopRegionColor);

    if (boundingRect().width() / dummy.width() > boundingRect().height() / dummy.height()) {
        dummyScale = QRect(0, 0, int(boundingRect().width()), int(boundingRect().width() / dummy.width() * dummy.height()));
    } else {
        dummyScale = QRect(0, 0, int(boundingRect().height() / dummy.height() * dummy.width()), int(boundingRect().height()));
    }
}

void CardPreviewItem::setCard(const Card *card)
{
    if (!card || card == m_card) return;

    QObject::disconnect(m_card, &Card::rarityChanged, this, &CardPreviewItem::changeRarity);
    QObject::disconnect(m_card, &Card::cardTypeChanged, this, &CardPreviewItem::changeCardType);
    QObject::disconnect(m_card, &Card::generalCardTypeChanged, this, &CardPreviewItem::changeGeneralCardType);
    QObject::disconnect(m_card, &Card::attributeChanged, this, &CardPreviewItem::changeAttribute);
    QObject::disconnect(m_card, &Card::cardNameChanged, this, &CardPreviewItem::changeCardName);
    QObject::disconnect(m_card, &Card::showCostChanged, this, &CardPreviewItem::showCost);
    QObject::disconnect(m_card, &Card::showStatsChanged, this, &CardPreviewItem::showStats);
    QObject::disconnect(m_card, &Card::showSmallTextBoxChanged, this, &CardPreviewItem::showSmallTextBox);
    QObject::disconnect(m_card, &Card::showBorderChanged, this, &CardPreviewItem::showBorder);
    QObject::disconnect(m_card, &Card::showTextBoxChanged, this, &CardPreviewItem::showTextBox);
    QObject::disconnect(m_card, &Card::showQuickcastChanged, this, &CardPreviewItem::showQuickcast);
    QObject::disconnect(m_card, &Card::redraw, this, &CardPreviewItem::redraw);

    QObject::disconnect(m_card->traitModel(), &LangStringListModel::dataChanged, this, &CardPreviewItem::changeTrait);
    QObject::disconnect(m_card->traitModel(), &LangStringListModel::rowsRemoved, this, &CardPreviewItem::changeTrait);
    QObject::disconnect(m_card->abilityTextModel(), &LangStringListModel::dataChanged, this, &CardPreviewItem::changeAbilityText);
    QObject::disconnect(m_card->abilityTextModel(), &LangStringListModel::rowsRemoved, this, &CardPreviewItem::removeAbilityText);

    m_card = card;

    QObject::connect(m_card, &Card::rarityChanged, this, &CardPreviewItem::changeRarity);
    QObject::connect(m_card, &Card::cardTypeChanged, this, &CardPreviewItem::changeCardType);
    QObject::connect(m_card, &Card::generalCardTypeChanged, this, &CardPreviewItem::changeGeneralCardType);
    QObject::connect(m_card, &Card::attributeChanged, this, &CardPreviewItem::changeAttribute);
    QObject::connect(m_card, &Card::cardNameChanged, this, &CardPreviewItem::changeCardName);
    QObject::connect(m_card, &Card::showCostChanged, this, &CardPreviewItem::showCost);
    QObject::connect(m_card, &Card::showStatsChanged, this, &CardPreviewItem::showStats);
    QObject::connect(m_card, &Card::showSmallTextBoxChanged, this, &CardPreviewItem::showSmallTextBox);
    QObject::connect(m_card, &Card::showBorderChanged, this, &CardPreviewItem::showBorder);
    QObject::connect(m_card, &Card::showTextBoxChanged, this, &CardPreviewItem::showTextBox);
    QObject::connect(m_card, &Card::showQuickcastChanged, this, &CardPreviewItem::showQuickcast);
    QObject::connect(m_card, &Card::redraw, this, &CardPreviewItem::redraw);

    QObject::connect(m_card->traitModel(), &LangStringListModel::dataChanged, this, &CardPreviewItem::changeTrait);
    QObject::connect(m_card->traitModel(), &LangStringListModel::rowsRemoved, this, &CardPreviewItem::changeTrait);
    QObject::connect(m_card->abilityTextModel(), &LangStringListModel::dataChanged, this, &CardPreviewItem::changeAbilityText);
    QObject::connect(m_card->abilityTextModel(), &LangStringListModel::rowsRemoved, this, &CardPreviewItem::removeAbilityText);
}

void CardPreviewItem::changeRarity(const FRarity *rarity)
{
    if (!m_card) return;

    if (!m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) && !m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
        if (rarity == FRarityModel::Instance()->get("SUPERRARE")) {
            cornerTL = QPixmap(":/card_decoration/border-corner-superrare.png");
            borderHorizontal = QPixmap(":/card_decoration/border-horizontal-superrare.png");
            borderVertical = QPixmap(":/card_decoration/border-vertical-superrare-diamond.png");
            nameBoxL = QPixmap(":/card_decoration/name-box-superrare-left.png");
            nameBoxM = QPixmap(":/card_decoration/name-box-superrare-mid.png");
            footerBoxL = QPixmap(":/card_decoration/footer-box-superrare-left.png");
            footerBoxM = QPixmap(":/card_decoration/footer-box-superrare-mid.png");

            if (m_card->showQuickcast()) {
                costWheel = QPixmap(":/card_decoration/cost-wheel-superrare-quickcast.png");
            } else {
                costWheel = QPixmap(":/card_decoration/cost-wheel-superrare.png");
            }

            if (m_card->showStats() && !m_card->showBorder()) {
                statsBox = QPixmap(":/card_decoration/stats-box-superrare-edge-diamond.png");
            } else {
                statsBox = QPixmap(":/card_decoration/stats-box-superrare-diamond.png");
            }
        } else {
            if (rarity == FRarityModel::Instance()->get("RARE")) {
                cornerTL = QPixmap(":/card_decoration/border-corner-rare.png");
            } else {
                cornerTL = QPixmap(":/card_decoration/border-corner-standard.png");
            }

            borderHorizontal = QPixmap(":/card_decoration/border-horizontal-standard.png");
            borderVertical = QPixmap(":/card_decoration/border-vertical-standard.png");
            nameBoxL = QPixmap(":/card_decoration/name-box-standard-left.png");
            nameBoxM = QPixmap(":/card_decoration/name-box-standard-mid.png");
            footerBoxL = QPixmap(":/card_decoration/footer-box-standard-left.png");
            footerBoxM = QPixmap(":/card_decoration/footer-box-standard-mid.png");

            if (m_card->showQuickcast()) {
                costWheel = QPixmap(":/card_decoration/cost-wheel-standard-quickcast.png");
            } else {
                costWheel = QPixmap(":/card_decoration/cost-wheel-standard.png");
            }

            if (m_card->showStats() && !m_card->showBorder()) {
                statsBox = QPixmap(":/card_decoration/stats-box-standard-edge.png");
            } else {
                statsBox = QPixmap(":/card_decoration/stats-box-standard.png");
            }
        }
        cornerTR = cornerTL.transformed(QTransform().scale(-1, 1));
        nameBoxR = nameBoxL.transformed(QTransform().scale(-1, 1));
        footerBoxR = footerBoxL.transformed(QTransform().scale(-1, 1));

        update(boundingRect());
    }
}

void CardPreviewItem::changeCardType(const FCardType *cardType)
{
    qDebug() << generateCardTypeText();
    textCardtype->setText(generateCardTypeText());
    if (cardType == FCardTypeModel::Instance()->get("RULER") || cardType == FCardTypeModel::Instance()->get("JRULER")) {
        cornerTL = QPixmap(":/card_decoration/border-corner-ruler.png");
        borderHorizontal = QPixmap(":/card_decoration/border-horizontal-superrare.png");
        borderVertical = QPixmap(":/card_decoration/border-vertical-superrare.png");
        nameBoxL = QPixmap(":/card_decoration/name-box-superrare-left.png");
        nameBoxM = QPixmap(":/card_decoration/name-box-superrare-mid.png");
        footerBoxL = QPixmap(":/card_decoration/footer-box-superrare-left.png");
        footerBoxM = QPixmap(":/card_decoration/footer-box-superrare-mid.png");

        cornerTR = cornerTL.transformed(QTransform().scale(-1, 1));
        nameBoxR = nameBoxL.transformed(QTransform().scale(-1, 1));
        footerBoxR = footerBoxL.transformed(QTransform().scale(-1, 1));

        if (m_card->showQuickcast()) {
            costWheel = QPixmap(":/card_decoration/cost-wheel-superrare-quickcast.png");
        } else {
            costWheel = QPixmap(":/card_decoration/cost-wheel-superrare.png");
        }

        if (m_card->showStats() && !m_card->showBorder()) {
            statsBox = QPixmap(":/card_decoration/stats-box-superrare-edge.png");
        } else {
            statsBox = QPixmap(":/card_decoration/stats-box-superrare.png");
        }

        update(boundingRect());
    } else {
        if (m_card) changeRarity(m_card->rarity());
    }
}

void CardPreviewItem::changeGeneralCardType(const FGeneralCardType * /*generalCardType*/)
{
    textCardtype->setText(generateCardTypeText());
    //textCardtype->updatePixmap();
    //update(textBoxTopRectInner);
}

void CardPreviewItem::changeAttribute(const FAttribute * /*attribute*/)
{
    updateAttributeGradient();
    update(boundingRect());
}

void CardPreviewItem::changeCardName(const FLanguageString cardName)
{
    // Update QGraphicsTextItem
    textCardname->setText(cardName.text());
    update(textBoxRect);
}

void CardPreviewItem::changeFlavorText(const FLanguageString flavorText)
{
    textFlavor->setText(flavorText.text());
    update(textFlavorBox);
}

void CardPreviewItem::changeTrait()
{
    textCardtype->setText(generateCardTypeText());
}

void CardPreviewItem::changeAbilityText(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
    int rows = m_card->abilityTextModel()->rowCount(QModelIndex());
    if (topLeft.row() == rows - 1 && rows > 1) {
        // Newly inserted
        FLanguageString text = m_card->abilityTextModel()->data(topLeft, Qt::DisplayRole).value<FLanguageString>();
        textAbilities->insertTextBlock(text.text());
    } else {
        textAbilities->clear();

        for (int i = 0; i < rows; ++i) {
            QModelIndex idx = m_card->abilityTextModel()->index(i);
            FLanguageString text = m_card->abilityTextModel()->data(idx, Qt::DisplayRole).value<FLanguageString>();
            textAbilities->insertTextBlock(text.text());
        }
    }

    textAbilities->fitToRect();
    textAbilities->updatePixmap();
    update(textBoxRect);
}

void CardPreviewItem::removeAbilityText()
{
    textAbilities->clear();
    int rows = m_card->abilityTextModel()->rowCount(QModelIndex());
    for (int i = 0; i < rows; ++i) {
        QModelIndex idx = m_card->abilityTextModel()->index(i);
        FLanguageString text = m_card->abilityTextModel()->data(idx, Qt::DisplayRole).value<FLanguageString>();
        textAbilities->insertTextBlock(text.text());
    }
    textAbilities->fitToRect();
    textAbilities->updatePixmap();
    update();
}

void CardPreviewItem::showCost(bool showCost, bool doUpdate)
{
    QRect nameTextBoxRect_new = QRect();
    if (showCost) {
        nameBoxRect = QRect(
                    NAME_BOX_X + nameBoxL.width(),
                    NAME_BOX_Y,
                    int(boundingRect().width()) - nameBoxR.width() - NAME_BOX_RIGHT_OFFSET - (NAME_BOX_X + nameBoxL.width()),
                    nameBoxM.height());
        nameTextBoxRect_new.setTopLeft(QPoint(nameBoxRect.left() + NAME_BOX_NO_COST_LEFTMARGIN, nameBoxRect.top()));
    } else {
        nameBoxRect = QRect(
                    NAME_BOX_NO_COST_X + nameBoxL.width(),
                    NAME_BOX_NO_COST_Y,
                    int(boundingRect().width()) - nameBoxR.width() - NAME_BOX_NO_COST_X - (NAME_BOX_NO_COST_X + nameBoxL.width()),
                    nameBoxM.height());
        nameTextBoxRect_new.setTopLeft(QPoint(nameBoxRect.left() + NAME_BOX_H_MARGIN, nameBoxRect.top()));
    }
    nameTextBoxRect_new.setBottomRight(QPoint(nameBoxRect.right() - NAME_BOX_H_MARGIN, nameBoxRect.bottom()));
    if (nameTextBoxRect_new != nameTextBoxRect) {
        nameTextBoxRect = nameTextBoxRect_new;
        textCardname->setTargetRect(nameTextBoxRect);
    }

    attributeNameGradient.setStart(nameBoxRect.topLeft());
    attributeNameGradient.setFinalStop(nameBoxRect.topRight());
    if (doUpdate) {
        update(QRectF(0, 0, boundingRect().width(), 1200));
    }
}

void CardPreviewItem::showStats(bool showStats, bool doUpdate)
{
    if (showStats && m_card && !m_card->showBorder()) {
        if (m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) || m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
            statsBox = QPixmap(":/card_decoration/stats-box-superrare-edge.png");
        } else {
            if (m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE")) {
                statsBox = QPixmap(":/card_decoration/stats-box-superrare-edge-diamond.png");
            } else {
                statsBox = QPixmap(":/card_decoration/stats-box-standard-edge.png");
            }
        }
    } else if (showStats && m_card && m_card->showBorder()) {
        if (m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) || m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
            statsBox = QPixmap(":/card_decoration/stats-box-superrare.png");
        } else {
            if (m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE")) {
                statsBox = QPixmap(":/card_decoration/stats-box-superrare-diamond.png");
            } else {
                statsBox = QPixmap(":/card_decoration/stats-box-standard.png");
            }
        }
    }
    if (doUpdate) {
        update(QRectF(0, 160, statsBox.width(), boundingRect().height()));
    }
}

void CardPreviewItem::showSmallTextBox(bool showSmallTextBox, bool doUpdate)
{
    if (showSmallTextBox) {
        textBoxRect = QRect(TEXT_BOX_X, TEXT_BOX_SMALL_Y, int(boundingRect().width()) - (TEXT_BOX_X * 2), footerBoxRect.y() - TEXT_BOX_SMALL_Y - 1);
        textBoxPath = QPainterPath();
        textBoxPath.addRoundedRect(textBoxRect, 6, 6);
        textBoxAttributeStartOffset.setY(TEXT_BOX_SMALL_Y + TEXT_BOX_TOP_HEIGHT / 2 - TEXT_BOX_ATTRIBUTE_SIZE / 2);

        textBoxTopRectInner.setTopLeft(QPoint(TEXT_BOX_X + 40, TEXT_BOX_SMALL_Y));
        textBoxTopRectInner.setBottomRight(QPoint(textBoxRect.right() - 40, TEXT_BOX_SMALL_Y + TEXT_BOX_TOP_HEIGHT));
    } else {
        textBoxRect = QRect(TEXT_BOX_X, TEXT_BOX_Y, int(boundingRect().width()) - (TEXT_BOX_X * 2), footerBoxRect.y() - TEXT_BOX_Y - 1);
        textBoxPath = QPainterPath();
        textBoxPath.addRoundedRect(textBoxRect, 6, 6);
        textBoxAttributeStartOffset.setY(TEXT_BOX_Y + TEXT_BOX_TOP_HEIGHT / 2 - TEXT_BOX_ATTRIBUTE_SIZE / 2);

        textBoxTopRectInner.setTopLeft(QPoint(TEXT_BOX_X + 40, TEXT_BOX_Y));
        textBoxTopRectInner.setBottomRight(QPoint(textBoxRect.right() - 40, TEXT_BOX_Y + TEXT_BOX_TOP_HEIGHT));
    }
    attributeTextBoxGradient.setStart(textBoxRect.topLeft());
    attributeTextBoxGradient.setFinalStop(textBoxRect.topRight());

    textBoxRectInner.setTopLeft(QPoint(textBoxRect.left() + 40, textBoxRect.top() + 20 + TEXT_BOX_TOP_HEIGHT));
    textBoxRectInner.setBottomRight(QPoint(textBoxRect.right() - 40, textBoxRect.bottom() - TEXT_BOX_FLAVOR_HEIGHT));

    textCardtype->setTargetRect(textBoxTopRectInner);
    textAbilities->setTargetRect(textBoxRectInner);
    if (doUpdate) {

        //textAbilities->updatePixmap();
        //textAbilities->update();
        update(QRectF(TEXT_BOX_X, TEXT_BOX_Y, int(boundingRect().width()) - (TEXT_BOX_X * 2), int(boundingRect().height()) - footerBoxM.height() - TEXT_BOX_Y - 2));
    }
}

void CardPreviewItem::showBorder(bool showBorder, bool doUpdate)
{
    if (!showBorder && m_card && m_card->showStats()) {
        if (m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) || m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
            statsBox = QPixmap(":/card_decoration/stats-box-superrare-edge.png");
        } else {
            if (m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE")) {
                statsBox = QPixmap(":/card_decoration/stats-box-superrare-edge-diamond.png");
            } else {
                statsBox = QPixmap(":/card_decoration/stats-box-standard-edge.png");
            }
        }
    } else if (showBorder && m_card && m_card->showStats()) {
        if (m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) || m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
            statsBox = QPixmap(":/card_decoration/stats-box-superrare.png");
        } else {
            if (m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE")) {
                statsBox = QPixmap(":/card_decoration/stats-box-superrare-diamond.png");
            } else {
                statsBox = QPixmap(":/card_decoration/stats-box-standard.png");
            }
        }
    }
    if (doUpdate) {
        update(boundingRect());
    }
}

void CardPreviewItem::showTextBox(bool /*showTextBox*/, bool doUpdate)
{
    if (doUpdate) {
        update(QRectF(TEXT_BOX_X, TEXT_BOX_Y, int(boundingRect().width()) - (TEXT_BOX_X * 2), int(boundingRect().height()) - footerBoxM.height() - TEXT_BOX_Y - 2));
    }
}

void CardPreviewItem::showQuickcast(bool showQuickcast, bool doUpdate)
{
    if (showQuickcast) {
        if (m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE") || m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) || m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
            costWheel = QPixmap(":/card_decoration/cost-wheel-superrare-quickcast.png");
        } else {
            costWheel = QPixmap(":/card_decoration/cost-wheel-standard-quickcast.png");
        }
    } else {
        if (m_card->rarity() == FRarityModel::Instance()->get("SUPERRARE") || m_card->hasCardType(FCardTypeModel::Instance()->get("RULER")) || m_card->hasCardType(FCardTypeModel::Instance()->get("JRULER"))) {
            costWheel = QPixmap(":/card_decoration/cost-wheel-superrare.png");
        } else {
            costWheel = QPixmap(":/card_decoration/cost-wheel-standard.png");
        }
    }
    if (doUpdate) {
        update(QRectF(COST_WHEEL_X, COST_WHEEL_Y, costWheel.width(), costWheel.height()));
    }
}

void CardPreviewItem::setTextItemFont(OptionsWindow::FontUpdateType type, const QFont &font)
{
    switch (type) {
    case OptionsWindow::FontCardname:
        textCardname->setFontFamily(font.family());
        break;
    case OptionsWindow::FontCardtype:
        break;
    case OptionsWindow::FontAbilities:
        textAbilities->setFontFamily(font.family());
        break;
    case OptionsWindow::FontFlavor:
        break;
    case OptionsWindow::FontStats:
        break;
    case OptionsWindow::FontVoidCost:
        textAbilities->setVoidCostFont(font);
        break;
    }
}

void CardPreviewItem::redraw(QRectF rect)
{
    if (rect.isEmpty()) {
        textCardname->updatePixmap();
        textCardname->update();
        textCardtype->updatePixmap();
        textCardtype->update();
        textAbilities->updatePixmap();
        textAbilities->update();
        update(boundingRect());
    } else {
        update(rect);
    }
}

void CardPreviewItem::updateAttributeGradient()
{
    if (!m_card) return;
    attributeNameGradient = QLinearGradient(nameBoxRect.topLeft(), nameBoxRect.topRight());
    attributeTextBoxGradient = QLinearGradient(textBoxRect.topLeft(), textBoxRect.topRight());

    const QMap<int, const FAttribute*> data = m_card->attributes();
    if (data.size() == 0) {
        attributeNameGradient.setColorAt(0, Util::BoxDefaultColor);
        attributeNameGradient.setColorAt(1, Util::BoxDefaultColor);
        attributeTextBoxGradient.setColorAt(0, Util::TextBoxTopRegionColor);
        attributeTextBoxGradient.setColorAt(1, Util::TextBoxTopRegionColor);
    } else if (data.size() == 1) {
        QColor color = QColor(data.first()->color2());
        attributeNameGradient.setColorAt(0, color);
        attributeNameGradient.setColorAt(1, color);

        //QColor color2 = QColor(data.first()->color2());
        //color2.setHsl(color2.hue(), qMin(255, color2.saturation() + 20), qMax(0, color2.lightness() - 50));
        //attributeNameGradient.setColorAt(1, color2);

        QColor colorTextBox = data.first()->color();

        attributeTextBoxGradient.setColorAt(0, colorTextBox);
        attributeTextBoxGradient.setColorAt(1, colorTextBox);

        textBoxAttributeStartOffset.setX(TEXT_BOX_X + textBoxRect.width() - TEXT_BOX_ATTRIBUTE_STEP);

        if (!attributes.contains(data.first()->id())) {
            QPixmap pixAttribute = Util::XML::svgToPixmap(QString(":/svg/" + data.first()->iconPath() + ".svg"), QSize(TEXT_BOX_ATTRIBUTE_SIZE,-1), QPen(Qt::white, 8), false);
            attributes.insert(data.first()->id(), pixAttribute);
        }
    } else {
        qreal step = 1.0 / (data.size() - 1);
        qreal halfStep = step / 2.0;
        qreal currentStep = 0;

        textBoxAttributeStartOffset.setX(TEXT_BOX_X + textBoxRect.width() - TEXT_BOX_ATTRIBUTE_STEP * data.size());

        QMap<int, const FAttribute*>::const_iterator it = data.constBegin();
        for (; it != data.constEnd(); ++it) {
            if (!attributes.contains((*it)->id())) {
                QPixmap pixAttribute = Util::XML::svgToPixmap(QString(":/svg/" + (*it)->iconPath() + ".svg"), QSize(TEXT_BOX_ATTRIBUTE_SIZE,-1), QPen(Qt::white, 8), false);
                attributes.insert((*it)->id(), pixAttribute);
            }
            QColor color = (*it)->color2();
            QColor colorTextBox = (*it)->color();

            attributeNameGradient.setColorAt(currentStep, color);
            attributeTextBoxGradient.setColorAt(currentStep, colorTextBox);

            // Squish colors closer together between two points
            if (currentStep + step <= 1.0) {
                qreal inbetween = currentStep + halfStep;
                qreal leftPos = inbetween - halfStep * COLOR_GRADIENT_SQUISH_FACTOR;
                qreal rightPos = inbetween + halfStep * COLOR_GRADIENT_SQUISH_FACTOR;

                attributeNameGradient.setColorAt(leftPos, color);
                attributeTextBoxGradient.setColorAt(leftPos, colorTextBox);

                QColor nextColor = (*(it+1))->color2();
                QColor nextTextBoxColor = (*(it+1))->color();

                attributeNameGradient.setColorAt(rightPos, nextColor);
                attributeTextBoxGradient.setColorAt(rightPos, nextTextBoxColor);
            }

            currentStep += step;
        }
    }
}

const QString CardPreviewItem::generateCardTypeText()
{
    if (!m_card) {
        return QString();
    }
    QString cardTypeText = QString();
    bool containsResonatorType = false;
    bool containsRulerType = false;
    const QMap<int, const FCardType*> cardTypes = m_card->cardTypes();
    QMap<int, const FCardType*>::const_iterator ctIter = cardTypes.constBegin();
    for (; ctIter != cardTypes.end(); ++ctIter) {
        int key = ctIter.key();
        const FCardType *ct = ctIter.value();
        QString typeText;
        if (m_card->generalCardTypes().contains(key) && m_card->generalCardTypes()[key]->id() > 0) {
            typeText = ct->nameCombined(m_card->generalCardTypes()[key]->stringId());
        } else {
            typeText = ct->name();
        }
        if (ct->stringId() == "RULER" || ct->stringId() == "JRULER") {
            containsRulerType = true;
            typeText = QString("[") + typeText + QString("!]");
        }

        if (ct->stringId() == "RESONATOR") {
            containsResonatorType = true;
        }

        if (ctIter+1 != cardTypes.constEnd()) {
            cardTypeText += typeText + QString("/");
        } else {
            cardTypeText += typeText;
        }
    }

    const LangStringListModel *traits = m_card->traitModel();

    if (containsResonatorType && traits->rowCount(QModelIndex()) > 0) {
        cardTypeText += QString(": ");
    } else if (!containsRulerType && traits->rowCount(QModelIndex()) > 0) {
        cardTypeText += QString(" (");
    } else if (traits->rowCount(QModelIndex()) > 0) {
        cardTypeText += QString(" ");
    }

    for (int i = 0; i < traits->rowCount(QModelIndex()); ++i) {
        QModelIndex idx = traits->index(i);
        const FLanguageString trait = traits->data(idx, Qt::DisplayRole).value<FLanguageString>();
        if (i+1 < traits->rowCount(QModelIndex())) {
            cardTypeText += trait.text() + QString("/");
        } else {
            cardTypeText += trait.text();
        }
    }

    if (!containsResonatorType && !containsRulerType && traits->rowCount(QModelIndex()) > 0) {
        cardTypeText += QString(")");
    }
    return cardTypeText;
}
