#ifndef FGRAPHICSTEXTITEM_H
#define FGRAPHICSTEXTITEM_H

#include <QGraphicsTextItem>
#include <QPen>
#include <QPixmap>

#include "ftextdocumentlayout.h"
#include "ftextobject.h"
#include "util.h"

struct FTextObjectReplacement
{
    QString word;
    Util::TextObject::Format objectType;
    QString symbolName;
};

class FGraphicsTextItem : public QGraphicsTextItem
{
public:
    enum FitToRectOrder { SpacingStretchSize, SizeSpacingStretch, SizeStretchSpacing };

    FGraphicsTextItem(QGraphicsItem *parent = nullptr, const QString &name = QString());
    FGraphicsTextItem(const QString &text, QGraphicsItem *parent = nullptr);

    void setOutlinePen(QPen pen) { m_outlinePen = pen; }
    void showOutline(bool show) { m_showOutline = show; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addReplacement(const QString &word, Util::TextObject::Format objectType, const QString &symbolName = QString());
    void setTargetRect(const QRect &rect);

    void setText(const QString &text);
    const QString text() const { return m_text; }

    void insertTextBlock(const QString &text);

    void setMinimumTextSize(int textSize);
    int minimumTextSize() const { return m_minTextSize; }

    void setFitToRectOrder(const FitToRectOrder &order);

    int calculatedTextSize() const { return m_calcTextSize; }
    void fitToRect();
    void checkUpdate(bool allowFitting = true);
    void clear();

public slots:
    void updatePixmap();
    void setPlainText(const QString &text);
    void setFont(const QFont &font);
    void setFontFamily(const QString &family);
    void setVoidCostFont(const QFont &font);

private:
    bool m_showOutline;
    bool m_isDirty;
    int m_minTextSize;
    int m_calcTextSize;
    int m_defaultTextSize;
    QString m_text;
    QPen m_outlinePen;
    QRect m_targetRect;
    QFont m_defaultFont;
    QFont m_voidCostFont;
    FTextDocumentLayout *m_layout;
    FKeywordTextObject *m_keywordTextObject;
    FSymbolTextObject *m_symbolTextObject;
    QPixmap m_textPixmap;
    QVector<FTextObjectReplacement> m_replacements;
    QMap<int, QString> m_textBlocks;
    FitToRectOrder m_fitToRectOrder;

    void generatePixmap();
    void parseAndInsertText(const QString &text);
    QString wordJoin(QString &text);
};

#endif // FGRAPHICSTEXTITEM_H
