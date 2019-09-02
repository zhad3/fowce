#include "fgraphicstextitem.h"
#include "ftextcursor.h"
#include "util.h"
#include <QPainter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QSettings>
#include <QDebug>

FGraphicsTextItem::FGraphicsTextItem(QGraphicsItem *parent, const QString &name)
    : QGraphicsTextItem(parent), m_showOutline(false), m_isDirty(true), m_minTextSize(9),
      m_calcTextSize(32), m_defaultTextSize(32), m_outlinePen(Qt::NoPen), m_fitToRectOrder(SpacingStretchSize)
{
    m_textPixmap = QPixmap(QSize(1,1));
    m_layout = new FTextDocumentLayout(document(), name);
    m_keywordTextObject = new class FKeywordTextObject;
    m_symbolTextObject = new class FSymbolTextObject;
    m_layout->registerHandler(Util::TextObject::KeywordTextFormat, m_keywordTextObject);
    m_layout->registerHandler(Util::TextObject::SymbolTextFormat, m_symbolTextObject);
    QTextBlockFormat blockFmt;
    blockFmt.setTopMargin(20);
    blockFmt.setBottomMargin(20);
    blockFmt.setLineHeight(90, QTextBlockFormat::LineHeightTypes::ProportionalHeight);
    textCursor().mergeBlockFormat(blockFmt);

    QTextOption opt = document()->defaultTextOption();
    opt.setWrapMode(QTextOption::WrapMode::WordWrap);
    document()->setDefaultTextOption(opt);

    document()->setDocumentLayout(m_layout);
    QObject::connect(document(), &QTextDocument::documentLayoutChanged, this, &FGraphicsTextItem::updatePixmap);
    //QObject::connect(document(), &QTextDocument::contentsChanged, this, &FGraphicsTextItem::updatePixmap);

    QSettings settings;
    m_voidCostFont.fromString(settings.value("font/voidcost", "Georgia").value<QString>());
}

FGraphicsTextItem::FGraphicsTextItem(const QString &text, QGraphicsItem *parent) : FGraphicsTextItem(parent)
{
}

// Draw everything twice (once with outline and then without)
// Drawing text is expensive (mostly when outline is active), hence we write it to a pixmap when the text gets changed
// and then just draw the pixmap
void FGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (m_isDirty) {
        QPainter p(&m_textPixmap);
        p.setFont(painter->font());
        p.setBrush(painter->brush());
        p.setOpacity(painter->opacity());
        p.setRenderHints(p.renderHints(), false);
        p.setRenderHints(painter->renderHints(), true);
        if (m_outlinePen != Qt::NoPen && m_outlinePen.color().alpha() > 0) {

            QTextCharFormat format;
            format.setTextOutline(m_outlinePen);

            QTextCursor cursor(this->document());
            cursor.select(QTextCursor::Document);

            cursor.mergeCharFormat(format);
            QGraphicsTextItem::paint (&p, option, widget);
            format.setTextOutline(Qt::NoPen);
            cursor.mergeCharFormat(format);
            QGraphicsTextItem::paint(&p, option, widget);

        } else {
            QTextCursor cursor(this->document());
            cursor.select(QTextCursor::Document);
            QTextCharFormat format = cursor.charFormat();
            if (format.penProperty(QTextFormat::TextOutline).style() != Qt::NoPen) {
                format.setTextOutline(Qt::NoPen);
                cursor.mergeCharFormat(format);
            }
            if (format.hasProperty(QTextCharFormat::OutlinePen)) {
                format.clearProperty(QTextCharFormat::OutlinePen);
                cursor.mergeCharFormat(format);
            }
            QGraphicsTextItem::paint(&p, option, widget);
        }
        p.end();
        m_isDirty = false;
        painter->drawPixmap(boundingRect(), m_textPixmap, m_textPixmap.rect());
    } else {
        painter->drawPixmap(boundingRect(), m_textPixmap, m_textPixmap.rect());
    }

    if (Util::DrawDebugInfo) {
        painter->setPen(QColor(255, 0, 255));
        painter->drawRect(boundingRect());
    }
}

void FGraphicsTextItem::setFont(const QFont &font)
{
    m_defaultTextSize = font.pointSize();
    m_defaultFont = font;
    QTextBlockFormat blockFmt = textCursor().blockFormat();
    blockFmt.setTopMargin(font.pointSize()/2);
    blockFmt.setBottomMargin(font.pointSize()/2);
    textCursor().mergeBlockFormat(blockFmt);
    QGraphicsTextItem::setFont(font);
    checkUpdate();
}

void FGraphicsTextItem::setFontFamily(const QString &family)
{
    m_defaultFont.setFamily(family);
    QGraphicsTextItem::setFont(m_defaultFont);
    checkUpdate();
}

void FGraphicsTextItem::setVoidCostFont(const QFont &font)
{
    m_voidCostFont = font;
    textCursor().movePosition(QTextCursor::MoveOperation::Start, QTextCursor::MoveMode::MoveAnchor);
    QTextCursor c = document()->find(QString(QChar::ObjectReplacementCharacter));
    while (!c.isNull()) {
        if (c.charFormat().objectType() == Util::TextObject::SymbolTextFormat) {
            QTextCharFormat fmt;
            fmt.setProperty(Util::TextObject::SymbolFont, font);
            c.mergeCharFormat(fmt);
        }
        c = document()->find(QString(QChar::ObjectReplacementCharacter), c.position());
    }
    m_textPixmap = QPixmap(boundingRect().size().toSize());
    m_textPixmap.fill(Qt::transparent);
    m_isDirty = true;
}

void FGraphicsTextItem::addReplacement(const QString &word, Util::TextObject::Format objectType, const QString &symbolName)
{
    FTextObjectReplacement replacement;
    replacement.word = word;
    replacement.objectType = objectType;
    replacement.symbolName = symbolName;

    m_replacements.push_back(replacement);
}

void FGraphicsTextItem::setPlainText(const QString &text)
{
    setText(text);
}

void FGraphicsTextItem::setTargetRect(const QRect &rect)
{

    setTextWidth(rect.width());
    setPos(rect.topLeft());
    // Reset font
//    QFont f = font();
//    f.setPointSize(m_defaultTextSize);
//    f.setStretch(QFont::Unstretched);
//    f.setLetterSpacing(QFont::PercentageSpacing, 100);
    if (rect.size() != m_targetRect.size()) {
        m_targetRect = rect;
        QGraphicsTextItem::setFont(m_defaultFont);
        fitToRect();
    } else {
        m_targetRect = rect;
        setY((m_targetRect.y() + m_targetRect.height()/2) - boundingRect().height()/2);
    }
}

void FGraphicsTextItem::setText(const QString &text)
{
    clear();
    m_text = text;
    parseAndInsertText(text);
    checkUpdate();
}

void FGraphicsTextItem::insertTextBlock(const QString &text)
{
    int lastKey = m_textBlocks.isEmpty() ? -1 : m_textBlocks.lastKey();
    m_textBlocks.insert(lastKey + 1, text);
    textCursor().movePosition(QTextCursor::MoveOperation::NextBlock, QTextCursor::MoveMode::MoveAnchor, 1);
    if (m_textBlocks.size() > 1) {
        textCursor().insertBlock();
    }
    parseAndInsertText(text);
    //qDebug() << document()->blockCount();
    //checkUpdate();
}

void FGraphicsTextItem::setMinimumTextSize(int textSize)
{
    if (textSize > m_calcTextSize) {
        m_minTextSize = textSize;
        checkUpdate();
    } else {
        m_minTextSize = textSize;
    }
}

void FGraphicsTextItem::setFitToRectOrder(const FGraphicsTextItem::FitToRectOrder &order)
{
    if (m_fitToRectOrder != order) {
        m_fitToRectOrder = order;
        fitToRect();
    }
}

void FGraphicsTextItem::fitToRect()
{
//    if (m_text.isNull() || m_text.isEmpty()) {
//        return;
//    }
    //QFont f = font();
    QFont f = m_defaultFont;
    QGraphicsTextItem::setFont(m_defaultFont);
    QTextBlockFormat blockFmt = textCursor().blockFormat();
    if (!m_targetRect.isValid() && m_calcTextSize != m_defaultTextSize) {
        m_calcTextSize = m_defaultTextSize;
        f.setPointSize(m_defaultTextSize);
        QGraphicsTextItem::setFont(f); // automatically updates layout
        checkUpdate(false);
        return;
    }

    bool breakLoop = false;
    while ((boundingRect().width() > m_targetRect.width() || boundingRect().height() > m_targetRect.height()) && font().pointSize() > m_minTextSize && !breakLoop) {
        switch (m_fitToRectOrder) {
        case SpacingStretchSize:
            if (f.letterSpacing() > 90) {
                f.setLetterSpacing(f.letterSpacingType(), f.letterSpacing() - 5);
            } else if (f.stretch() > QFont::SemiCondensed || f.stretch() == QFont::AnyStretch) {
                if (f.stretch() == QFont::AnyStretch) {
                    f.setStretch(QFont::Unstretched - 3);
                } else {
                    f.setStretch(f.stretch() - 3);
                }
            } else {
                f.setPointSize(f.pointSize() - 2);
            }
            break;
        case SizeSpacingStretch:
            if (f.pointSize() - 2 > m_minTextSize) {
                f.setPointSize(f.pointSize() - 2);
            } else if (f.letterSpacing() > 90) {
                f.setLetterSpacing(f.letterSpacingType(), f.letterSpacing() - 5);
            } else if (f.stretch() > QFont::SemiCondensed || f.stretch() == QFont::AnyStretch) {
                if (f.stretch() == QFont::AnyStretch) {
                    f.setStretch(QFont::Unstretched - 3);
                } else {
                    f.setStretch(f.stretch() - 3);
                }
            } else {
                breakLoop = true;
            }
            break;
        case SizeStretchSpacing:
            if (f.pointSize() - 2 > m_minTextSize) {
                f.setPointSize(f.pointSize() - 2);
            } else if (f.stretch() > QFont::SemiCondensed || f.stretch() == QFont::AnyStretch) {
                if (f.stretch() == QFont::AnyStretch) {
                    f.setStretch(QFont::Unstretched - 3);
                } else {
                    f.setStretch(f.stretch() - 3);
                }
            } else if (f.letterSpacing() > 90) {
                f.setLetterSpacing(f.letterSpacingType(), f.letterSpacing() - 5);
            } else {
                breakLoop = true;
            }
            break;
        }

        QGraphicsTextItem::setFont(f); // automatically updates layout
    }
    if (m_calcTextSize != f.pointSize()) {
        blockFmt.setTopMargin(f.pointSize()/2);
        blockFmt.setBottomMargin(f.pointSize()/2);
        textCursor().mergeBlockFormat(blockFmt);
    }
    m_calcTextSize = f.pointSize();

    m_textPixmap = QPixmap(boundingRect().size().toSize());
    m_textPixmap.fill(Qt::transparent);
    m_isDirty = true;

    // Update Y position for vertical alignment
    setY((m_targetRect.y() + m_targetRect.height()/2) - boundingRect().height()/2);

    qDebug() << "FGraphicsTextItem::fitToRect => newSize:" << m_calcTextSize << "(Default:" << m_defaultTextSize << "), Spacing:" << f.letterSpacing() << ", Stretch:" << f.stretch() << ", BlockMargin:" << textCursor().blockFormat().topMargin();
}

void FGraphicsTextItem::updatePixmap()
{
        m_textPixmap = QPixmap(boundingRect().size().toSize());
        m_textPixmap.fill(Qt::transparent);
        m_isDirty = true;

        // Update Y position for vertical alignment
        setY((m_targetRect.y() + m_targetRect.height()/2) - boundingRect().height()/2);
}

void FGraphicsTextItem::parseAndInsertText(const QString &text)
{
    QRegularExpression re("\\[([^+/]+?)\\]", QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matchIter = re.globalMatch(text);
    int lastCaptureEnd = 0;
    bool hasMatch = false;
    QTextCharFormat fmt = textCursor().charFormat();
    if (!matchIter.hasNext()) {
        QString str = text;
        textCursor().insertText(wordJoin(str));
    }
    while(matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        if (match.hasMatch()) {
            hasMatch = true;
            bool intConverted = true;
            if (match.capturedStart(0) > lastCaptureEnd) {
                // Insert normal text
                QString str = text.mid(lastCaptureEnd, match.capturedStart(0) - lastCaptureEnd);
                textCursor().insertText(wordJoin(str));
            }
            // Insert symbol object if it exists
            if (Util::TextObject::Replacements.contains(match.captured(1))) {
                FTextCursor::insertSymbol(textCursor(), Util::TextObject::Replacements[match.captured(1)].symbolName, m_outlinePen);
            } else {
                if (match.captured(1).size() == 1 && match.captured(1).toInt(&intConverted) > -1 && intConverted) {
                    // Insert number
                    FTextCursor::insertSymbol(textCursor(), ":/svg/symbol-voidcost.svg", m_outlinePen, match.captured(1), m_voidCostFont);
                } else {
                    bool showGradient = match.captured(1).at(match.captured(1).size()-1) == '!';
                    FTextCursor::insertKeyword(textCursor(), showGradient ? match.captured(1).left(match.captured(1).size()-1) : match.captured(1), showGradient);
                }
            }
            lastCaptureEnd = match.capturedEnd(0);
            //qDebug() << "Matched! #0:" << match.captured(0) << "#1:" << match.captured(1) << "Start:" << match.capturedStart(0) << "End:" << match.capturedEnd(0);
        }
    }

    // Insert rest of text
    if (lastCaptureEnd < text.length() && hasMatch) {
        QString str = text.right(text.size() - lastCaptureEnd);
        textCursor().insertText(wordJoin(str));
    }
}
/*!
 * \brief Inserts the unicode character U+2060 (Word Joiner) after and before Slashes ('/') so that they do not get wrapped into a new line.
 * \param text
 * \return QString
 */
QString FGraphicsTextItem::wordJoin(QString &text)
{
    QRegularExpression re("/", QRegularExpression::MultilineOption);
    text.replace(re, QString(QChar(0x2060)) + "/" + QString(QChar(0x2060)));
    return text;
}

void FGraphicsTextItem::checkUpdate(bool allowFitting)
{
    //qDebug() << "FGraphicsTextItem::checkUpdate()";
    if (m_targetRect.isValid() && (boundingRect().width() > m_targetRect.width() || boundingRect().height() > m_targetRect.height()) && allowFitting) {
        //qDebug() << "FGraphicsTextItem::checkUpdate => Need to fit.";
        fitToRect();
    } else {
        m_textPixmap = QPixmap(boundingRect().size().toSize());
        m_textPixmap.fill(Qt::transparent);
        m_isDirty = true;

        // Update Y position for vertical alignment
        setY((m_targetRect.y() + m_targetRect.height()/2) - boundingRect().height()/2);
    }
}

void FGraphicsTextItem::clear()
{
    m_textBlocks.clear();
    QTextCursor c = QTextCursor(document()->firstBlock());
    //c.movePosition(QTextCursor::MoveOperation::Start, QTextCursor::MoveMode::MoveAnchor, 0);
    c.movePosition(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::KeepAnchor, 1);
    c.removeSelectedText();
}
