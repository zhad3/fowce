#include "ftextobject.h"
#include "util.h"

#include <QFontMetricsF>
#include <QTextBlock>
#include <QPainter>
#include <QDebug>

#define DEBUG_OUTLINE 0

QSizeF FKeywordTextObject::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc);
    Q_UNUSED(posInDocument);

    QSizeF size = QSizeF(0,0);
    QTextCharFormat fmt = format.toCharFormat();
    QString contents = qvariant_cast<QString>(format.property(Util::TextObject::KeywordData));
    bool showGradient = false;
    if (format.hasProperty(Util::TextObject::KeywordGradient)) {
        showGradient = qvariant_cast<bool>(format.property(Util::TextObject::KeywordGradient));
    }
    QFont f = fmt.font();
    if (!showGradient) {
        f.setPointSize(fmt.font().pointSize() - 4);
    }
    QFontMetricsF fm(f);
    //qreal marginx = fm.horizontalAdvance("x");
    qreal marginy = fm.xHeight()/2;
    qreal width = fm.width(contents) + fm.height();
    if (showGradient) {
        width = qMax(width, 300.0);
        //marginy *= 4;
    }
    size.setWidth(width);
    size.setHeight(fm.height() + marginy);

    return size;
}

void FKeywordTextObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc);
    Q_UNUSED(posInDocument);

    QPen p = painter->pen();
    QBrush b = painter->brush();

    QTextCharFormat fmt = format.toCharFormat();
//    qDebug("drawObject => Before: font size: %d (%d) (%d), family: %s", fmt.font().pointSize(), fmt.fontPointSize(), painter->font().pointSize(), qUtf8Printable(fmt.font().family()));
    if (fmt.foreground().color().alpha() == 0) return;
    QString contents = qvariant_cast<QString>(format.property(Util::TextObject::KeywordData));

    bool showGradient = false;
    if (fmt.hasProperty(Util::TextObject::KeywordGradient)) {
        showGradient = qvariant_cast<bool>(fmt.property(Util::TextObject::KeywordGradient));
    }

    QFont font = fmt.font();
    if (!showGradient) {
        font.setPointSize(fmt.font().pointSize() - 4);
    }
    QFontMetricsF fm(font);
    qreal marginx = fm.height()/2;
    qreal marginy = fm.xHeight()/2;
    QRectF textBounds = fm.boundingRect(contents);
    textBounds.moveLeft(rect.left() + (rect.width() - textBounds.width())/2);
    textBounds.moveTop(rect.top() + (rect.height() - textBounds.height())/2);

    // Fix font size
    QFont f = fmt.font();
    //f.setPixelSize(48);
    f.setPointSize(font.pointSize());
    f.setFamily(font.family());
    f.setLetterSpacing(font.letterSpacingType(), font.letterSpacing() - 5);
    f.setStretch(font.stretch());
    f.setStyleStrategy(font.styleStrategy());
    if (!showGradient) {
        f.setWeight(QFont::Bold);
    }
    painter->setFont(f);

//    qDebug("drawObject => After: font size: %d (%d) (%f), family: %s", fmt.font().pointSize(), fmt.fontPointSize(), painter->font().pointSizeF(), qUtf8Printable(painter->font().family()));
//    qDebug("drawObject => fontInfo: pointSize: %d", painter->fontInfo().pointSize());
//    qDebug() << "drawObject => fontInfo" << painter->fontInfo().family() << painter->fontInfo().pixelSize() << painter->fontInfo().pointSize();


    QPen p2 = QPen();



    if (showGradient) {
        QLinearGradient gradient;
        gradient.setStart(rect.left(), textBounds.top());
        gradient.setFinalStop(rect.left(), textBounds.bottom());
        gradient.setColorAt(0, QColor(212, 182, 50));
        gradient.setColorAt(0.4, QColor(254, 231, 151));
        gradient.setColorAt(0.6, QColor(254, 231, 151));
        gradient.setColorAt(1, QColor(212, 182, 50));

        p2.setColor(QColor(255, 255, 215));

        painter->setBrush(gradient);
    } else {
        painter->setBrush(Qt::white);
    }

    p2.setWidth(4);
    painter->setPen(p2);

    QPainterPath pp;
    if (showGradient && (fm.width(contents) + fm.height()) < 300) {
        pp.moveTo(rect.left() + marginx, textBounds.top());
        pp.lineTo(QPointF(rect.right() - marginx, textBounds.top()));
        //pp.cubicTo(rect.right() - marginx + (rect.height() - marginy /* * 2*/)/2,
        pp.cubicTo(rect.right(),
                   textBounds.top(),
                   rect.right(),
                   textBounds.bottom(),
                   rect.right() - marginx,
                   textBounds.bottom());
        pp.lineTo(QPointF(rect.left() + marginx, textBounds.bottom()));
        pp.cubicTo(rect.left(),
                   textBounds.bottom(),
                   rect.left(),
                   textBounds.top(),
                   rect.left() + marginx,
                   textBounds.top());
    } else {
        pp.moveTo(textBounds.left(), textBounds.top());
        pp.lineTo(QPointF(textBounds.right(), textBounds.top()));
        pp.cubicTo(textBounds.right() + marginx,
                   textBounds.top(),
                   textBounds.right() + marginx,
                   textBounds.bottom(),
                   textBounds.right(),
                   textBounds.bottom());
        pp.lineTo(QPointF(textBounds.left(), textBounds.bottom()));
        pp.cubicTo(textBounds.left() - marginx,
                   textBounds.bottom(),
                   textBounds.left() - marginx,
                   textBounds.top(),
                   textBounds.left(),
                   textBounds.top());
    }

    painter->drawPath(pp);

    if (fmt.textOutline().color().alpha() != 0 && fmt.textOutline() != Qt::NoPen) {
        QPen outlinePen = fmt.textOutline();
        QBrush prevBrush = painter->brush();
        outlinePen.setWidthF(outlinePen.widthF() + 2.);
        painter->setPen(outlinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(pp);
        painter->setBrush(prevBrush);
    }

    painter->setPen(p2);
    painter->drawPath(pp);

    painter->setBrush(b);
    painter->setPen(p);

    //QRectF textRect(rect.x() + marginx, rect.y() + marginy, rect.width() - marginx * 2, rect.height() - marginy * 2);
    painter->drawText(textBounds, Qt::AlignCenter, contents);

    if (Util::DrawDebugInfo) {
        // Debug
        QPen stroke(QBrush(Qt::red), 1);
        painter->setPen(stroke);
        painter->drawRect(rect);
        stroke.setColor(Qt::blue);
        painter->setPen(stroke);
        painter->drawRect(textBounds);
    }
}

QSizeF FSymbolTextObject::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc);
    Q_UNUSED(posInDocument);

    QSizeF size = QSizeF(0,0);
    QTextCharFormat fmt = format.toCharFormat();
    QFontMetricsF fm(doc->defaultFont());
    qreal marginy = fm.xHeight()/4;
    qreal marginx = marginy;

    QImage svgImage = qvariant_cast<QImage>(fmt.property(Util::TextObject::SymbolData));
    qreal outlineWidth = 0;
    if (fmt.hasProperty(Util::TextObject::SymbolOutlineWidth)) {
        outlineWidth = qvariant_cast<qreal>(fmt.property(Util::TextObject::SymbolOutlineWidth));
    }

    size.setWidth(fm.height() + marginx + outlineWidth);
    size.setHeight(fm.height() + marginy + outlineWidth);

    return size;
}

void FSymbolTextObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(doc);
    Q_UNUSED(posInDocument);
    QPen p(painter->pen());

    QTextCharFormat fmt = format.toCharFormat();
    if (fmt.foreground().color().alpha() == 0) return;
    QFontMetricsF fm(doc->defaultFont());
    qreal marginy = fm.xHeight()/4;
    qreal marginx = marginy;

    //QImage svgImage = qvariant_cast<QImage>(fmt.property(Util::TextObject::SymbolData));
    QPixmap svgImage = qvariant_cast<QPixmap>(fmt.property(Util::TextObject::SymbolData));
    qreal outlineWidth = qvariant_cast<qreal>(fmt.property(Util::TextObject::SymbolOutlineWidth));


    QRectF dest;
    dest = QRectF(rect.x() + 0*marginx/2, rect.y() + 0*marginy/2, rect.width() - 0*marginx, rect.height() - 0*marginy);

    if (fmt.textOutline().color().alpha() > 0 && fmt.textOutline() != Qt::NoPen && outlineWidth > -1) {
        //painter->drawImage(dest, svgImage);
        painter->drawPixmap(dest, svgImage, svgImage.rect());
    } else if (outlineWidth <= -1) {
        dest = QRectF(rect.x() + marginx/2, rect.y() + marginy/2, rect.width() - marginx, rect.height() - marginy);
        //painter->drawImage(dest, svgImage);
        painter->drawPixmap(dest, svgImage, svgImage.rect());
    }

    if (fmt.textOutline().color().alpha() <= 0 || fmt.textOutline() == Qt::NoPen || outlineWidth <= -1) {
        if (fmt.hasProperty(Util::TextObject::SymbolText)) {
            QString text = qvariant_cast<QString>(fmt.property(Util::TextObject::SymbolText));
            if (!text.isEmpty()) {
                // Fix font size
                QFont f = fmt.font();
                if (fmt.hasProperty(Util::TextObject::SymbolFont)) {
                    QFont fmtFont = qvariant_cast<QFont>(fmt.property(Util::TextObject::SymbolFont));
                    f.setFamily(fmtFont.family());
                } else {
                    f.setFamily(fmt.font().family());
                }
                f.setPointSize(fmt.font().pointSize() + 6);
                f.setItalic(true);
                f.setWeight(QFont::DemiBold);
                //f.setLetterSpacing(fmt.font().letterSpacingType(), fmt.font().letterSpacing());
                //f.setStretch(fmt.font().stretch());
                painter->setFont(f);


                // To stretch the font vertically we convert the text to a path and update
                // the elements positions of the path
                // The goal is to make the font fit the symbol height
                QPainterPath pp;
                pp.addText(QPointF(0,0), f, text);
                QSizeF textSize = pp.boundingRect().size();
                qreal scaleFactor = (dest.height()*0.75) / textSize.height();
                // The actual path points are not normalized, hence we keep track of
                // the min,max values to center align them properly
                qreal minx = 9999;
                qreal maxx = -9999;
                qreal miny = 9999;
                qreal maxy = -9999;
                for (int i = 0; i < pp.elementCount(); ++i) {
                    QPainterPath::Element el = pp.elementAt(i);
                    pp.setElementPositionAt(i, el.x, el.y * scaleFactor);
                    minx = qMin(minx, el.x);
                    maxx = qMax(maxx, el.x);
                    miny = qMin(miny, el.y);
                    maxy = qMax(maxy, el.y);
                }
                textSize = pp.controlPointRect().size();
                QPointF centerPos = QPointF((dest.x() + dest.width()/2) - (textSize.width()/2 + minx), (dest.y() + dest.height()/2) + (textSize.height()/2 - maxy));
                pp.translate(centerPos);
                painter->setBrush(QBrush(painter->pen().color()));
                painter->setPen(Qt::NoPen);
                painter->drawPath(pp);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(p);

                //painter->drawText(rect, Qt::AlignCenter, text);
            }

        }
    }

    if (Util::DrawDebugInfo) {
        // Debug
        QPen stroke(QBrush(Qt::red), 0);
        painter->setPen(stroke);
        painter->drawRect(rect);
        painter->drawRect(dest);
        painter->drawLine(QLineF(rect.x() - 50, rect.y(), rect.x() + rect.width() + 50, rect.y()));
        //painter->drawLine(QLineF(rect.x() - 50, rect.y() + rect.height()/2, rect.x() + rect.width() + 50, rect.y() + rect.height()/2));
        painter->drawLine(QLineF(rect.x() - 50, rect.y() + rect.height()/2, rect.x() + rect.width() + 50, rect.y() + rect.height()/2));
        painter->drawLine(QLineF(rect.x() - 50, rect.y() + rect.height(), rect.x() + rect.width() + 50, rect.y() + rect.height()));
    }
}
