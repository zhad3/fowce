#ifndef FTEXTCURSOR_H
#define FTEXTCURSOR_H

#include <QTextCursor>
#include <QTextDocument>
#include <QFontMetricsF>
#include <QSvgRenderer>
#include <QFile>
#include <QtMath>
#include <QPainter>
#include <QDebug>
#include "util.h"

class FTextCursor : public QTextCursor
{
public:
    FTextCursor() : QTextCursor() {}
    FTextCursor(QTextDocument *document) : QTextCursor(document) {}
    FTextCursor(QTextFrame *frame) : QTextCursor(frame) {}
    FTextCursor(const QTextBlock &block) : QTextCursor(block) {}
    FTextCursor(const QTextCursor &cursor) : QTextCursor(cursor) {}

    //void insertKeyword(const QString &text, bool showGradient = false);
    //void insertSymbol(const QString &filename, QPen outline = Qt::NoPen);
    static void insertKeyword(QTextCursor cursor, const QString &text, bool showGradient)
    {
        QTextCharFormat oldFormat = cursor.charFormat();
        QTextCharFormat format = cursor.charFormat();
        format.setObjectType(Util::TextObject::KeywordTextFormat);
        format.setProperty(Util::TextObject::KeywordData, text);
        //format.setFont(cursor.document()->defaultFont(), QTextCharFormat::FontPropertiesAll);
        format.setVerticalAlignment(QTextCharFormat::VerticalAlignment::AlignMiddle);
        format.setProperty(Util::TextObject::KeywordGradient, showGradient);
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);

        // revert char format
        cursor.setCharFormat(oldFormat);
    }

    static void insertSymbol(QTextCursor cursor, const QString &filename, QPen outline, const QString &text = QString(), const QFont &font = QFont())
    {
        QTextCharFormat oldFormat = cursor.charFormat();
        QTextCharFormat format = cursor.charFormat();
        format.setObjectType(Util::TextObject::SymbolTextFormat);
        if (!text.isEmpty()) {
            format.setProperty(Util::TextObject::SymbolText, text);
        } else {
            format.clearProperty(Util::TextObject::SymbolText);
        }
        format.setProperty(Util::TextObject::SymbolOutlineWidth, outline == Qt::NoPen ? -1 : outline.widthF());
        if (font != QFont()) {
            format.setProperty(Util::TextObject::SymbolFont, font);
        } else {
            format.clearProperty(Util::TextObject::SymbolFont);
        }
        //format.setFont(cursor.document()->defaultFont(), QTextCharFormat::FontPropertiesAll);
        format.setVerticalAlignment(QTextCharFormat::VerticalAlignment::AlignMiddle);

          //QSvgRenderer renderer(text);
//        QFile svgFile(filename);
//        svgFile.open(QIODevice::ReadOnly);
//        QByteArray svgData(svgFile.readAll());

        QFontMetricsF fm(format.font());
//        qreal height = fm.height() * 2; // Set height bigger because we scale it down later

//        QDomDocument domDoc;
//        domDoc.setContent(svgData);

//        QDomElement svg = domDoc.firstChildElement();

//        int svgHeight = svg.attribute("height").toInt();
//        int svgWidth = svg.attribute("width").toInt();
//        qreal outlineWidth = 0;

//        //svg.setAttribute("height", QString::number(svgHeight));
//        //svg.setAttribute("width", QString::number(svgWidth));

//        if (outline.color().alpha() > 0 && outline != Qt::NoPen) {
//            outlineWidth = qFloor((svgHeight/height) * outline.widthF() *2 /* * 2 / 2*/); // Times two because we render at 2x size and
//                                                                                       // divide by two because stroke is aligned at the center
//        }

//        QRectF viewBox = QRectF(-outlineWidth, -outlineWidth, svgHeight + outlineWidth * 2, svgWidth + outlineWidth * 2);
//        QImage svgImage(QSize(qCeil(height), qCeil(height)), QImage::Format_ARGB32);
//        svgImage.fill(Qt::transparent);

//        QPainter p(&svgImage);
//        //p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

//        if (outline.color().alpha() > 0 && outline != Qt::NoPen) {
//            QDomElement backgroundElem = Util::XML::findElementById(domDoc, "background").toElement();
//            if (!backgroundElem.isNull()) {
//                QString oldStyles = backgroundElem.attribute("style");

//                QMap<QString, QString> styles = Util::XML::getStyleAttrib(backgroundElem);
//                styles["stroke"] = outline.color().name();
//                styles["stroke-width"] = QString::number(outlineWidth * 2);
//                QString styleString = Util::XML::styleAttribToString(styles);

//                backgroundElem.setAttribute("style", styleString);

//                QByteArray svgOutlineData = domDoc.toByteArray();

//                QSvgRenderer outlineRenderer(svgOutlineData);
//                outlineRenderer.setViewBox(viewBox);
//                outlineRenderer.render(&p, svgImage.rect());

//                backgroundElem.setAttribute("style", oldStyles);
//            }
//        }

//        QSvgRenderer renderer(domDoc.toByteArray());
//        renderer.setViewBox(viewBox);
//        renderer.render(&p, svgImage.rect());

        QPixmap svgImage = Util::XML::svgToPixmap(filename, QSize(-1, qCeil(fm.height())), outline);

        format.setProperty(Util::TextObject::SymbolData, svgImage);

        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);

        // Revert charFormat
        cursor.setCharFormat(oldFormat);
    }
};

#endif // FTEXTCURSOR_H
