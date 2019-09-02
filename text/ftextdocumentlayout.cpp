#include "ftextdocumentlayout.h"
#include <QDebug>

#include <QtMath>
#include <QFont>
#include <QTextFrame>
#include <QTextBlock>
#include <QVector>
#include <QPointer>
#include <QFontMetricsF>
#include <QPainter>

#include <algorithm>

#define DEBUG_PRINT 0

static bool operator<(const QCheckPoint &checkPoint, QFixed y)
{
    return checkPoint.y < y;
}

static bool operator<(const QCheckPoint &checkPoint, int pos)
{
    return checkPoint.positionInFrame < pos;
}

class QTextFrameData : public QTextFrameLayoutData
{
public:
    QTextFrameData();

    // Relative to parent frame
    QFixedPoint position;
    QFixedSize size;

    QFixed topMargin;
    QFixed bottomMargin;
    QFixed leftMargin;
    QFixed rightMargin;
    QFixed border;
    QFixed padding;

    // Includes padding
    QFixed contentsWidth;
    QFixed contentsHeight;
    QFixed oldContentsWidth;

    // Accumulated margins
    QFixed effectiveTopMargin;
    QFixed effectiveBottomMargin;

    QFixed minimumWidth;
    QFixed maximumWidth;

    QTextLayoutStruct *currentLayoutStruct;

    bool sizeDirty;
    bool layoutDirty;

    QVector<QPointer<QTextFrame> > floats;
};

struct QTextLayoutStruct
{
    QTextLayoutStruct() : maximumWidth(QFIXED_MAX), fullLayout(false) {}
    QTextFrame *frame;
    QFixed x_left;
    QFixed x_right;
    QFixed frameY; // Absolute y position of the current frame
    QFixed y; // Always relative to the current frame
    QFixed contentsWidth;
    QFixed minimumWidth;
    QFixed maximumWidth;
    bool fullLayout;
    QList<QTextFrame*> pendingFloats;
    QFixed pageHeight;
    QFixed pageBottom;
    QFixed pageTopMargin;
    QFixed pageBottomMargin;
    QRectF updateRect;
    QRectF updateRectForFloats;

    inline void addUpdateRectForFloat(const QRectF &rect)
    {
        if (updateRectForFloats.isValid()) {
            updateRectForFloats |= rect;
        } else {
            updateRectForFloats = rect;
        }
    }
    inline QFixed absoluteY() const
    {
        return frameY + y;
    }
    inline QFixed contentHeight() const
    {
        return pageHeight - pageBottomMargin - pageTopMargin;
    }
    inline int currentPage() const
    {
        return pageHeight == 0 ? 0 : (absoluteY() / pageHeight).truncate();
    }
    inline void newPage()
    {
        if (pageHeight == QFIXED_MAX) {
            return;
        }
        pageBottom += pageHeight;
        y = qMax(y, pageBottom - pageHeight + pageBottomMargin + pageTopMargin - frameY);
    }
};

static QTextFrameData *createFrameData(QTextFrame *f)
{
    QTextFrameData *data = new QTextFrameData;
    f->setLayoutData(data);
    return data;
}

static inline QTextFrameData *frameData(QTextFrame *f)
{
    QTextFrameData *data = static_cast<QTextFrameData*>(f->layoutData());
    if (!data) {
        data = createFrameData(f);
    }
    return data;
}
#if DEBUG_PRINT==1
static void printFrameData(QTextFrameData *fd)
{
    qDebug("=== Frame Data ===");
    qDebug("Position: %.2f, %.2f", fd->position.x.toReal(), fd->position.y.toReal());
    qDebug("Size: %f, %f", fd->size.width.toReal(), fd->size.height.toReal());
    qDebug("topMargin: %.2f, bottomMargin: %.2f, leftMargin: %.2f, rightMargin: %.2f", fd->topMargin.toReal(), fd->bottomMargin.toReal(), fd->leftMargin.toReal(), fd->rightMargin.toReal());
    qDebug("border: %.2f, padding: %.2f", fd->border.toReal(), fd->padding.toReal());

    qDebug("effectiveTopMargin: %.2f, effectiveBottomMargin: %.2f", fd->effectiveTopMargin.toReal(), fd->effectiveBottomMargin.toReal());
    qDebug("minimumWidth: %.2f, maximumWidth: %.2f", fd->minimumWidth.toReal(), fd->maximumWidth.toReal());
    qDebug("contentsWidth: %.2f, contentsHeight: %.2f", fd->contentsWidth.toReal(), fd->contentsHeight.toReal());
    qDebug("===");
}
#endif

static QFixed flowPosition(const QTextFrame::iterator &it)
{
    if (it.atEnd()) {
        return 0;
    }

    if (it.currentFrame()) {
        return frameData(it.currentFrame())->position.y;
    } else {
        QTextBlock block = it.currentBlock();
        QTextLayout *layout = block.layout();
        if (layout->lineCount() == 0) {
            return QFixed::fromReal(layout->position().y());
        } else {
            return QFixed::fromReal(layout->position().y() + layout->lineAt(0).y());
        }
    }
}

static QFixed firstChildPos(const QTextFrame *f)
{
    return flowPosition(f->begin());
}

static inline void getLineHeightParams(const QTextBlockFormat &blockFormat, const QTextLine &line, qreal scaling,
                                       QFixed *lineAdjustment, QFixed *lineBreakHeight, QFixed *lineHeight, QFixed *lineBottom)
{
    #if DEBUG_PRINT==1
    qDebug("getLineHeightParams: line.ascent = %.2f, line.descent = %.2f, line.leading = %.2f, line.height = %.2f", line.ascent(), line.descent(), line.leading(), line.height());
    #endif
    qreal rawHeight = qCeil(line.ascent() + line.descent() + line.leading());
    *lineHeight = QFixed::fromReal(blockFormat.lineHeight(rawHeight, scaling));
    *lineBottom = QFixed::fromReal(blockFormat.lineHeight(line.height(), scaling));

    if (blockFormat.lineHeightType() == QTextBlockFormat::FixedHeight || blockFormat.lineHeightType() == QTextBlockFormat::MinimumHeight) {
        *lineBreakHeight = *lineBottom;
        if (blockFormat.lineHeightType() == QTextBlockFormat::FixedHeight)
            *lineAdjustment = QFixed::fromReal(line.ascent() + qMax(line.leading(), qreal(0.0))) - ((*lineHeight * 4) / 5);
        else
            *lineAdjustment = QFixed::fromReal(line.height()) - *lineHeight;
    }
    else {
        *lineBreakHeight = QFixed::fromReal(line.height());
        *lineAdjustment = 0;
    }
}

static void fillBackground(QPainter *p, const QRectF &rect, QBrush brush, const QPointF &origin, const QRectF &gradientRect = QRectF())
{
    p->save();
    if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
        if (!gradientRect.isNull()) {
            QTransform m;
            m.translate(gradientRect.left(), gradientRect.top());
            m.scale(gradientRect.width(), gradientRect.height());
            brush.setTransform(m);
            const_cast<QGradient*>(brush.gradient())->setCoordinateMode(QGradient::LogicalMode);
        }
    } else {
        p->setBrushOrigin(origin);
    }
    p->fillRect(rect, brush);
    p->restore();
}

QTextFrameData::QTextFrameData() : maximumWidth(QFIXED_MAX),
    currentLayoutStruct(nullptr), sizeDirty(true), layoutDirty(true)
{
}

FTextDocumentLayout::FTextDocumentLayout(QTextDocument *document, const QString &name) : QAbstractTextDocumentLayout(document)
{
    m_name = name;
    m_idealWidth = 0;
    m_useClip = false;
    currentLazyLayoutPosition = -1;
    lazyLayoutStepSize = 1000;
    contentHasAlignment = false;
    lastPageCount = -1;
    showLayoutProgress = true;
    insideDocumentChange = false;
}

int FTextDocumentLayout::dynamicPageCount() const
{
    const QSizeF pgSize = document()->pageSize();
    if (pgSize.height() < 0) {
        return 1;
    }
    return qCeil(dynamicDocumentSize().height() / pgSize.height());
}

QSizeF FTextDocumentLayout::dynamicDocumentSize() const
{
    return frameData(document()->rootFrame())->size.toSizeF();
}

QRectF FTextDocumentLayout::frameBoundingRectInternal(QTextFrame *frame) const
{
    QPointF pos;
    QTextFrame *f = frame;
    while (f) {
        QTextFrameData *fd = frameData(f);
        pos += fd->position.toPointF();

        // removed table data

        f = f->parentFrame();
    }

    return QRectF(pos, frameData(frame)->size.toSizeF());
}

QFixed FTextDocumentLayout::blockIndent(const QTextBlockFormat &blockFormat) const
{
    qreal indent = blockFormat.indent();

    QTextObject *object = document()->objectForFormat(blockFormat);
    if (object) {
        indent += object->format().toListFormat().indent();
    }
    if (qIsNull(indent)) {
        return 0;
    }

    qreal scale = 1;
    if (paintDevice()) {
        scale = qreal(paintDevice()->logicalDpiY()) / 100;
    }

    return QFixed::fromReal(indent * scale * document()->indentWidth());
}

void FTextDocumentLayout::drawFrame(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextFrame *frame) const
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::drawFlow";
    #endif
    QTextFrameData *fd = frameData(frame);
    if (fd->layoutDirty) {
        return;
    }
    Q_ASSERT(!fd->sizeDirty);
    Q_ASSERT(!fd->layoutDirty);

    const QPointF off = offset + fd->position.toPointF();
    if (m_useClip && context.clip.isValid()
            && (off.y() > context.clip.bottom() || off.y() + fd->size.height.toReal() < context.clip.top()
            || off.x() > context.clip.right() || off.x() + fd->size.width.toReal() < context.clip.left()))
    {
        return;
    }

    const QRectF frameRect(off, fd->size.toSizeF());
    QTextBlock cursorBlockNeedingRepaint;

    // if the cursor is /on/ a table border we may need to repaint it
    // afterwards, as we usually draw the decoration first

    // removed ==> table data

    drawFrameDecoration(painter, frame, fd, context.clip, frameRect);

    QTextFrame::Iterator it = frame->begin();

    if (frame == document()->rootFrame()) {
        if (m_useClip) {
            it = frameIteratorForYPosition(QFixed::fromReal(context.clip.top()));
        }
    }

    QList<QTextFrame*> floats;
    const int numFloats = fd->floats.count();
    floats.reserve(numFloats);
    for (int i = 0; i < numFloats; ++i) {
        floats.append(fd->floats.at(i));
    }

    drawFlow(off, painter, context, it, floats, &cursorBlockNeedingRepaint);

    // removed ==> cursorBlockNeedingRepaint
}

void FTextDocumentLayout::drawFlow(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextFrame::Iterator it, const QList<QTextFrame *> &floats, QTextBlock *cursorBlockNeedingRepaint) const
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::drawFlow";
    #endif
    Q_UNUSED(cursorBlockNeedingRepaint);
    const bool inRootFrame = (!it.atEnd() && it.parentFrame() && it.parentFrame()->parentFrame() == nullptr);

    QVector<QCheckPoint>::ConstIterator lastVisibleCheckPoint = checkPoints.end();
    if (m_useClip && inRootFrame && context.clip.isValid()) {
        lastVisibleCheckPoint = std::lower_bound(checkPoints.begin(), checkPoints.end(), QFixed::fromReal(context.clip.bottom()));
    }

    QTextBlock previousBlock;
    QTextFrame *previousFrame = nullptr;

    for (; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();

        if (inRootFrame && !checkPoints.isEmpty()) {
            int currentPosInDoc;
            if (c) {
                currentPosInDoc = c->firstPosition();
            } else {
                currentPosInDoc = it.currentBlock().position();
            }

            // if we're past what is already laid out then we're better off
            // not trying to draw things that may not be positioned correctly yet
            if (currentPosInDoc >= checkPoints.constLast().positionInFrame) {
                break;
            }

            if (lastVisibleCheckPoint != checkPoints.end() && m_useClip && context.clip.isValid() && currentPosInDoc >= lastVisibleCheckPoint->positionInFrame) {
                break;
            }
        }

        if (c) {
            drawFrame(offset, painter, context, c);
        } else {
            //QAbstractTextDocumentLayout::PaintContext pc = context;
            // removed ==> table data

            drawBlock(offset, painter, context, it.currentBlock(), inRootFrame);
        }


        // when entering a table and the previous block is empty
        // then layoutFlow 'hides' the block that just causes a
        // new line by positioning it /on/ the table border. as we
        // draw that block before the table itself the decoration
        // 'overpaints' the cursor and we need to paint it afterwards
        // again

        // removed ==> table data

        previousBlock = it.currentBlock();
        previousFrame = c;
    }

    for (int i = 0; i < floats.count(); ++i) {
        QTextFrame *frame = floats.at(i);
        if (frame->firstPosition() <= frame->lastPosition() || frame->frameFormat().position() == QTextFrameFormat::InFlow) {
            continue;
        }

        const int pos = frame->firstPosition() - 1;
        QTextCharFormat format = const_cast<FTextDocumentLayout*>(this)->format(pos);
        QTextObjectInterface *handler = this->handlerForObject(format.objectType());
        if (handler) {
            QRectF rect = frameBoundingRectInternal(frame);
            handler->drawObject(painter, rect, document(), pos, format);
        }
    }
}

void FTextDocumentLayout::drawBlock(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, const QTextBlock &bl, bool inRootFrame) const
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::drawBlock" << bl.blockNumber();
    #endif
    const QTextLayout *tl = bl.layout();
    QRectF r = tl->boundingRect();
    if (!bl.isVisible() || (m_useClip && context.clip.isValid() && (r.bottom() < context.clip.y() || r.top() > context.clip.bottom()))) {
        #if DEBUG_PRINT==1
        qDebug() << "("<<m_name<<") FTExtDocumentLayout::drawBlock => bl is invisible" << bl.blockNumber();
        #endif
        return;
    }

    QTextBlockFormat blockFormat = bl.blockFormat();

    QBrush bg = blockFormat.background();
    if (bg != Qt::NoBrush) {
        QRectF rect = r;

        // extend the background rectangle if we're in the root frame with NoWrap,
        // as the rect of the text block will then be only the width of the text
        // instead of the full page width
        if (inRootFrame && document()->pageSize().width() <= 0) {
            const QTextFrameData *fd = frameData(document()->rootFrame());
            rect.setRight((fd->size.width - fd->rightMargin).toReal());
        }

        fillBackground(painter, rect, bg, r.topLeft());
    }
    //fillBackground(painter, r, QBrush(Qt::red), r.topLeft());

    // Draw selection?
    QVector<QTextLayout::FormatRange> selections;
    //...
    //const QTextCharFormat *selFormat = nullptr;
    // removed ==> selection drawing

    QTextObject *object = document()->objectForFormat(bl.blockFormat());
    if (object && object->format().toListFormat().style() != QTextListFormat::ListStyleUndefined) {
        // drawListItem(offset, painter, context, bl, selFormat);
    }

    QPen oldPen = painter->pen();
    painter->setPen(context.palette.color(QPalette::Text));
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTExtDocumentLayout::drawBlock => tl->draw() offset =" << offset;
    qDebug("(%s) FTExtDocumentLayout::drawBlock => clipRect = %.2f, %.2f, %.2f, %.2f", qUtf8Printable(m_name), clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height());
    qDebug("(%s) FTExtDocumentLayout::drawBlock => textLayoutRect = %.2f, %.2f, %.2f, %.2f", qUtf8Printable(m_name), r.x(), r.y(), r.width(), r.height());
    #endif
    //tl->draw(painter, offset, selections, context.clip.isValid() ? (context.clip & clipRect) : clipRect);
    tl->draw(painter, offset, selections, (m_useClip && context.clip.isValid()) ? (context.clip & clipRect) : clipRect);

    // Draw cursor?
    // removed ==> draw cursor

    if (blockFormat.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
        const qreal width = blockFormat.lengthProperty(QTextFormat::BlockTrailingHorizontalRulerWidth).value(r.width());
        painter->setPen(context.palette.color(QPalette::Dark));
        qreal y = r.bottom();
        if (bl.length() == 1) {
            y = r.top() + r.height() / 2;
        }
        const qreal middleX = r.left() + r.width() / 2;
        painter->drawLine(QLineF(middleX - width / 2, y, middleX + width / 2, y));
    }

    painter->setPen(oldPen);

    #if DEBUG_PRINT==1
    painter->drawRect(r);
    #endif
}

void FTextDocumentLayout::drawBorder(QPainter *painter, const QRectF &rect, qreal topMargin, qreal bottomMargin, qreal border, const QBrush &brush, const QTextFrameFormat::BorderStyle style) const
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::drawBorder";
    #endif
    const qreal pageHeight = document()->pageSize().height();
    const int topPage = pageHeight > 0 ? static_cast<int>(rect.top() / pageHeight) : 0;
    const int bottomPage = pageHeight > 0 ? static_cast<int>((rect.bottom() + border) / pageHeight) : 0;

    Q_UNUSED(style);

    bool turn_off_antialiasing = !(painter->renderHints() & QPainter::Antialiasing);
    painter->setRenderHint(QPainter::Antialiasing);

    for (int i = topPage; i <= bottomPage; ++i) {
        QRectF clipped = rect.toRect();

        if (topPage != bottomPage) {
            clipped.setTop(qMax(clipped.top(), i * pageHeight + topMargin - border));
            clipped.setBottom(qMin(clipped.bottom(), (i + 1) * pageHeight - bottomMargin));

            if (clipped.bottom() <= clipped.top()) {
                continue;
            }
        }

        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(brush);
        painter->drawRect(QRectF(clipped.left(), clipped.top(), clipped.left() + border, clipped.bottom() + border));
        painter->drawRect(QRectF(clipped.left() + border, clipped.top(), clipped.right() + border, clipped.top() + border));
        painter->drawRect(QRectF(clipped.right(), clipped.top() + border, clipped.right() + border, clipped.bottom()));
        painter->drawRect(QRectF(clipped.left() + border, clipped.bottom(), clipped.right() + border, clipped.bottom() + border));
        painter->restore();
    }

    if (turn_off_antialiasing) {
        painter->setRenderHint(QPainter::Antialiasing, false);
    }
}

void FTextDocumentLayout::drawFrameDecoration(QPainter *painter, QTextFrame *frame, QTextFrameData *fd, const QRectF &clip, const QRectF &rect) const
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::drawFrameDecoration";
    #endif
    const QBrush bg = frame->frameFormat().background();
    if (bg != Qt::NoBrush) {
        QRectF bgRect =rect;
        bgRect.adjust((fd->leftMargin + fd->border).toReal(),
                      (fd->topMargin + fd->border).toReal(),
                      -(fd->rightMargin + fd->border).toReal(),
                      -(fd->bottomMargin + fd->border).toReal());
        QRectF gradientRect; // invalid makes it default to bgRect
        QPointF origin = bgRect.topLeft();
        if (!frame->parentFrame()) {
            bgRect = clip;
            gradientRect.setWidth(painter->device()->width());
            gradientRect.setHeight(painter->device()->height());
        }
        fillBackground(painter, bgRect, bg, origin, gradientRect);
    }

    if (fd->border != 0) {
        painter->save();
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);

        const qreal leftEdge = rect.left() + fd->leftMargin.toReal();
        const qreal border = fd->border.toReal();
        const qreal topMargin = fd->topMargin.toReal();
        const qreal leftMargin = fd->leftMargin.toReal();
        const qreal bottomMargin = fd->bottomMargin.toReal();
        const qreal rightMargin = fd->rightMargin.toReal();
        const qreal w = rect.width() - 2 * border - leftMargin - rightMargin;
        const qreal h = rect.height() - 2 * border - topMargin - bottomMargin;

        drawBorder(painter, QRectF(leftEdge, rect.top() + topMargin, w + border, h + border),
                   fd->effectiveTopMargin.toReal(), fd->effectiveBottomMargin.toReal(),
                   border, frame->frameFormat().borderBrush(), frame->frameFormat().borderStyle());

        painter->restore();
    }
}


void FTextDocumentLayout::draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::draw";
    #endif
    QTextFrame *frame = document()->rootFrame();
    QTextFrameData *fd = frameData(frame);


    if (fd->sizeDirty) {
        return;
    }

    if (!m_useClip) {
        painter->setClipping(false);
    }

    // disable clip, we draw to a pixmap!
    if (m_useClip && context.clip.isValid()) {
        ensureLayouted(QFixed::fromReal(context.clip.bottom()));
    } else {
        ensureLayoutFinished();
    }

    QFixed width = fd->size.width;
    if (document()->pageSize().width() < 0.01 && viewportRect.isValid()) {
        // we're in NoWrap mode, meaning the frame should expand to the viewport
        // so that backgrounds are drawn correctly
        fd->size.width = qMax(width, QFixed::fromReal(viewportRect.right()));
    }
    // Make sure we conform to the root frames bounds when drawing.
    clipRect = QRectF(fd->position.toPointF(), fd->size.toSizeF()).adjusted(fd->leftMargin.toReal(), 0, -fd->rightMargin.toReal(), 0);
    drawFrame(QPointF(), painter, context, frame);
    fd->size.width = width;

#if DEBUG_PRINT==1
    QPen p(painter->pen());
    painter->setPen(Qt::red);
    qDebug() << "("<<m_name<<") Drawing clipRect:" << clipRect;
    painter->drawRect(clipRect);
    painter->setPen(p);
#endif
}

int FTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_UNUSED(point);
    Q_UNUSED(accuracy);
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::hitText(point =" << point << ", accuracy =" << accuracy << ")";
    // Return invalid cursor position. Selection not supported. Read-only.
    #endif
    return -1;
}

int FTextDocumentLayout::pageCount() const
{
    ensureLayoutFinished();
    int pageCount = dynamicPageCount();
    #if DEBUG_PRINT==1
    qDebug("(%s) FTextDocumentLayout::pageCount() => %d", qUtf8Printable(m_name), pageCount);
    #endif
    return pageCount;
}

QSizeF FTextDocumentLayout::documentSize() const
{
    ensureLayoutFinished();
    QSizeF size = dynamicDocumentSize();
    #if DEBUG_PRINT==1
    qDebug("(%s) FTextDocumentLayout::documentSize() => QSizeF(%.2f, %.2f)", qUtf8Printable(m_name), size.width(), size.height());
    #endif
    return size;
}

QRectF FTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    if (document()->pageSize().isNull()) {
        return QRectF();
    }
    ensureLayoutFinished();
    QRectF rect = frameBoundingRectInternal(frame);
    #if DEBUG_PRINT==1
    qDebug("(%s) FTextDocumentLayout::frameBoundingRect(frame) => QRectF(%.2f, %.2f, %.2f, %.2f)", qUtf8Printable(m_name), rect.x(), rect.y(), rect.width(), rect.height());
    #endif
    return rect;
}

QRectF FTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    if (document()->pageSize().isNull() || !block.isValid() || !block.isVisible()) {
        return QRectF();
    }
    ensureLayoutedByPosition(block.position() + block.length());
    QTextFrame *frame = document()->frameAt(block.position());
    QPointF offset;
    //const int blockPos = block.position();

    while (frame) {
        QTextFrameData *fd = frameData(frame);
        offset += fd->position.toPointF();

        // removed table data

        frame = frame->parentFrame();
    }

    const QTextLayout *layout = block.layout();
    QRectF rect = layout->boundingRect();
    rect.moveTopLeft(layout->position() + offset);
    #if DEBUG_PRINT==1
    qDebug("(%s) FTextDocumentLayout::blockBoundingRect(block = %s, blockNumber = %d, blockPosition = %d) => QRectF(%.2f, %.2f, %.2f, %.2f)",
           qUtf8Printable(m_name), qUtf8Printable(block.text()), block.blockNumber(), block.length(), rect.x(), rect.y(), rect.width(), rect.height());
    #endif
    return rect;
}

qreal FTextDocumentLayout::idealWidth() const
{
    ensureLayoutFinished();
    return m_idealWidth;
}

void FTextDocumentLayout::documentChanged(int from, int charsRemoved, int charsAdded)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::documentChanged(from = " << from << ", charsRemoved = " << charsRemoved << ", charsAdded = " << charsAdded;
    #endif

    QTextBlock blockIt = document()->findBlock(from);
    QTextBlock endIt = document()->findBlock(qMax(0, from + charsAdded - 1));
    if (endIt.isValid()) {
        endIt = endIt.next();
    }
    for (; blockIt.isValid() && blockIt != endIt; blockIt = blockIt.next()) {
        blockIt.clearLayout();
    }
    if (document()->pageSize().isNull()) {
        return;
    }

    QRectF updateRect;

    lazyLayoutStepSize = 1000;
    sizeChangedTimer.stop();
    insideDocumentChange = true;

    const int documentLength = document()->characterCount();
    const bool fullLayout = (charsRemoved == 0 && charsAdded == documentLength);
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::documentChanged ==> documentLength =" << documentLength << ", fullLayout =" << fullLayout;
    #endif
    const bool smallChange = documentLength > 0 && (qMax(charsAdded, charsRemoved) * 100 / documentLength) < 5;

    // don't show incremental layout progress (avoid scroll bar flicker)
    // if we see only a small change in the document and we're either starting
    // a layout run or we're already in progress for that and we haven't seen
    // any bigger change previously (showLayoutProgress already false)
    if (smallChange && (currentLazyLayoutPosition == -1 || showLayoutProgress == false)) {
        showLayoutProgress = false;
    } else {
        showLayoutProgress = true;
    }


    if (fullLayout) {
        contentHasAlignment = false;
        currentLazyLayoutPosition = 0;
        checkPoints.clear();
        layoutStep();
    } else {
        ensureLayoutedByPosition(from);
        updateRect = doLayout(from, charsRemoved, charsAdded);
    }

    if (!layoutTimer.isActive() && currentLazyLayoutPosition != -1) {
    #if DEBUG_PRINT==1
        qDebug() << "("<<m_name<<") FTextDocumentLayout::documentChanged ==> Start Timer";
    #endif
        layoutTimer.start(10, this);
    }

    insideDocumentChange = false;

    if (showLayoutProgress) {
        const QSizeF newSize = dynamicDocumentSize();
        if (newSize != lastReportedSize) {
            lastReportedSize = newSize;
    #if DEBUG_PRINT==1
            qDebug("(%s) FTextDocumentLayout::documentChanged ==> emit %.2f, %.2f", qUtf8Printable(m_name), lastReportedSize.width(), lastReportedSize.height());
    #endif
            emit documentSizeChanged(newSize);
        }
    }

    if (!updateRect.isValid()) {
    #if DEBUG_PRINT==1
        qDebug() << "("<<m_name<<") FTextDocumentLayout::documentChanged ==> Invalid updateRect";
    #endif
        updateRect = QRectF(QPointF(0,0), QSizeF(qreal(INT_MAX), qreal(INT_MAX)));
    }
    #if DEBUG_PRINT==1
    qDebug("(%s) FTextDocumentLayout::documentChanged ==> updateRect: %.2f, %.2f  %.2f x %.2f", qUtf8Printable(m_name), updateRect.x(), updateRect.y(), updateRect.width(), updateRect.height());
    #endif
    emit update(updateRect);
}

void FTextDocumentLayout::markFrames(QTextFrame *current, int from, int charsRemoved, int charsAdded)
{
    int end = qMax(charsRemoved, charsAdded) + from;
    if (current->firstPosition() >= end || current->lastPosition() < from) {
        return;
    }
    QTextFrameData *fd = frameData(current);
    QTextFrame *null = nullptr;

    // Remove nullptr from floats and then remove all elements between last removed and end item
    fd->floats.erase(std::remove(fd->floats.begin(), fd->floats.end(), null), fd->floats.end());

    fd->layoutDirty = true;
    fd->sizeDirty = true;
    QList<QTextFrame*> children = current->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        markFrames(children.at(i), from, charsRemoved, charsAdded);
    }
}

QRectF FTextDocumentLayout::doLayout(int from, int charsRemoved, int charsAdded)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::doLayout(from =" << from << ", charsRemoved =" << charsRemoved << ", charsAdded =" << charsAdded << ")";
    #endif
    markFrames(document()->rootFrame(), from, charsRemoved, charsAdded);

    QRectF updateRect;
    QTextFrame *root = document()->rootFrame();
    QTextFrameData *fd = frameData(root);
    if (fd->sizeDirty) {
        updateRect = layoutFrame(root, from, from + charsAdded);
    }
    fd->layoutDirty = false;

    if (currentLazyLayoutPosition == -1) {
        layoutFinished();
    } else if (showLayoutProgress) {
        sizeChangedTimer.start(0, this);
    }

    return updateRect;
}

QRectF FTextDocumentLayout::layoutFrame(QTextFrame *frame, int from, int to, QFixed parentY)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFrame(frame =" << frame << ", from =" << from << ", to =" << to << ", parentY =" << parentY << ")";
    #endif
    Q_ASSERT(frameData(frame)->sizeDirty);

    QTextFrameFormat fformat = frame->frameFormat();

    QTextFrame *parent = frame->parentFrame();
    const QTextFrameData *pfd = parent ? frameData(parent) : nullptr;

    const qreal maximumWidth = qMax(qreal(0), pfd ? pfd->contentsWidth.toReal() : document()->pageSize().width());
    QFixed width = QFixed::fromReal(fformat.width().value(maximumWidth));
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFrame ==> maximumWidth =" << maximumWidth << ", width =" << width.toReal();
    #endif

    if (fformat.width().type() == QTextLength::FixedLength && paintDevice()) {
        width = QFixed::fromReal(width.toReal() * (paintDevice()->logicalDpiY() / 100.)); // no qt_defaultDpi()
    #if DEBUG_PRINT==1
        qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFrame ==> fixed width =" << width;
    #endif
    }

    const QFixed maximumHeight = pfd ? pfd->contentsHeight : -1;
    const QFixed height = (maximumHeight != -1 || fformat.height().type() != QTextLength::PercentageLength)
                            ? QFixed::fromReal(fformat.height().value(maximumHeight.toReal()))
                            : -1;

    return layoutFrameEx(frame, from, to, width, height, parentY);
}

QRectF FTextDocumentLayout::layoutFrameEx(QTextFrame *frame, int from, int to, QFixed width, QFixed height, QFixed parentY)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFrameEx(frame =" << frame << ", from =" << from << ", to =" << to << ", width =" << width << ", height =" << height << ", parentY =" << parentY << ")";
    #endif
    Q_ASSERT(frameData(frame)->sizeDirty);

    QTextFrameData *fd = frameData(frame);
    QFixed newContentsWidth;

    bool fullLayout = false;
    {
        QTextFrameFormat fformat = frame->frameFormat();
        QFixed tm = QFixed::fromReal(fformat.topMargin());
        if (tm != fd->topMargin) {
            fd->topMargin = tm;
            fullLayout = true;
        }
        QFixed bm = QFixed::fromReal(fformat.bottomMargin());
        if (bm != fd->bottomMargin) {
            fd->bottomMargin = bm;
            fullLayout = true;
        }
        fd->leftMargin = QFixed::fromReal(fformat.leftMargin());
        fd->rightMargin = QFixed::fromReal(fformat.rightMargin());
        QFixed border = QFixed::fromReal(fformat.border());
        if (border != fd->border) {
            fd->border = border;
            fullLayout = true;
        }
        QFixed padding = QFixed::fromReal(fformat.padding());
        if (padding != fd->padding) {
            fd->padding = padding;
            fullLayout = true;
        }

        QTextFrame *parent = frame->parentFrame();
        const QTextFrameData *pfd = parent ? frameData(parent) : nullptr;

        // Accumulate top and bottom margins
        if (parent) {
            fd->effectiveTopMargin = pfd->effectiveTopMargin + fd->topMargin + fd->border + fd->padding;
            fd->effectiveBottomMargin = pfd->effectiveBottomMargin + fd->bottomMargin + fd->border + fd->padding;
        } else {
            fd->effectiveTopMargin = fd->topMargin + fd->border + fd->padding;
            fd->effectiveBottomMargin = fd->bottomMargin + fd->border + fd->padding;
        }

        newContentsWidth = width - 2*(fd->border + fd->padding) - fd->leftMargin - fd->rightMargin;
        if (height != -1) {
            fd->contentsHeight = height - 2*(fd->border + fd->padding) - fd->topMargin - fd->bottomMargin;
        } else {
            fd->contentsHeight = height;
        }
    }

    // set fd->contentsWidth temporarily, so that layoutFrame for the children
    // picks the right width. We'll initialize it properly at the end of this
    // function.
    fd->contentsWidth = newContentsWidth;

    QTextLayoutStruct layoutStruct;
    layoutStruct.frame = frame;
    layoutStruct.x_left = fd->leftMargin + fd->border + fd->padding;
    layoutStruct.x_right = layoutStruct.x_left + newContentsWidth;
    layoutStruct.y = fd->topMargin + fd->border + fd->padding;
    layoutStruct.frameY = parentY + fd->position.y;
    layoutStruct.contentsWidth = 0;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = QFIXED_MAX;
    layoutStruct.fullLayout = fullLayout || (fd->oldContentsWidth != newContentsWidth);
    layoutStruct.updateRect = QRectF(QPointF(0,0), QSizeF(qreal(INT_MAX), qreal(INT_MAX)));

    fd->oldContentsWidth = newContentsWidth;

    layoutStruct.pageHeight = QFixed::fromReal(document()->pageSize().height());
    if (layoutStruct.pageHeight < 0) {
        layoutStruct.pageHeight = QFIXED_MAX;
    }

    const int currentPage = layoutStruct.pageHeight == 0 ? 0 : (layoutStruct.frameY / layoutStruct.pageHeight).truncate();
    layoutStruct.pageTopMargin = fd->effectiveTopMargin;
    layoutStruct.pageBottomMargin = fd->effectiveBottomMargin;
    layoutStruct.pageBottom = (currentPage + 1) * layoutStruct.pageHeight - layoutStruct.pageBottomMargin;

    if (!frame->parentFrame()) {
        m_idealWidth = 0; // Reset
    }

    QTextFrame::Iterator it = frame->begin();
    layoutFlow(it, &layoutStruct, from, to);

    QFixed maxChildFrameWidth = 0;
    QList<QTextFrame*> children = frame->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextFrameData *cd = frameData(c);
        maxChildFrameWidth = qMax(maxChildFrameWidth, cd->size.width);
    }
    #if DEBUG_PRINT==1
    qDebug("(%s) FTextDocumentLayout::layoutFrame ==> maxChildFrameWidth = %.2f", qUtf8Printable(m_name), maxChildFrameWidth.toReal());
    #endif

    const QFixed marginWidth = 2*(fd->border + fd->padding) + fd->leftMargin + fd->rightMargin;
    if (!frame->parentFrame()) {
        m_idealWidth = qMax(maxChildFrameWidth, layoutStruct.contentsWidth).toReal();
        m_idealWidth += marginWidth.toReal();
    #if DEBUG_PRINT==1
        qDebug("(%s) FTextDocumentLayout::layoutFrame ==> idealWidth = %.2f, contentsWidth = %.2f", qUtf8Printable(m_name), m_idealWidth, layoutStruct.contentsWidth.toReal());
    #endif
    }

    QFixed actualWidth = qMax(newContentsWidth, qMax(maxChildFrameWidth, layoutStruct.contentsWidth));
    fd->contentsWidth = actualWidth;
    if (newContentsWidth <= 0) { // nowrap layout?
        fd->contentsWidth = newContentsWidth;
    }

    fd->minimumWidth = layoutStruct.minimumWidth;
    fd->maximumWidth = layoutStruct.maximumWidth;

    fd->size.height = fd->contentsHeight == -1
            ? layoutStruct.y + fd->border + fd->padding + fd->bottomMargin
            : fd->contentsHeight + 2*(fd->border + fd->padding) + fd->topMargin + fd->bottomMargin;
    fd->size.width = actualWidth + marginWidth;
    fd->sizeDirty = false;
    if (layoutStruct.updateRectForFloats.isValid()) {
        layoutStruct.updateRect |= layoutStruct.updateRectForFloats;
    }
    #if DEBUG_PRINT==1
    printFrameData(fd);
    #endif
    return layoutStruct.updateRect;
}

void FTextDocumentLayout::layoutBlock(const QTextBlock &bl, int blockPosition, const QTextBlockFormat &blockFormat, QTextLayoutStruct *layoutStruct, int from, int to, const QTextBlockFormat *previousBlockFormat)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutBlock(bl (blockNumber =" << bl.blockNumber() << "), blockPosition =" << blockPosition << ", blockFormat, layoutStruct, from =" << from << ", to =" << to << ", previousBlockFormat)";
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutBlock => blockFormat: lineHeight:" << blockFormat.lineHeight() << ", blockText:" << bl.text();
    #endif
    if (!bl.isVisible()) {
        return;
    }
    QTextLayout *tl = bl.layout();
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutBlock => QTextLayout. Font:" << tl->font() << ", blockLength:" << bl.length() << ", Text:" << tl->text() << ", TextOption flags:" << tl->textOption().flags() << ", boundingRect:" << tl->boundingRect();
    #endif
    const int blockLength = bl.length();

    if (previousBlockFormat) {
        qreal margin = qMax(blockFormat.topMargin(), previousBlockFormat->bottomMargin());
        if (margin > 0 && paintDevice()) {
            margin *= qreal(paintDevice()->logicalDpiY()) / 100.0;
        }
        layoutStruct->y += QFixed::fromReal(margin);
    }

    Qt::LayoutDirection dir = bl.textDirection();

    QFixed extraMargin;
    if (document()->defaultTextOption().flags() & QTextOption::AddSpaceForLineAndParagraphSeparators) {
        QFontMetricsF fm(bl.charFormat().font());
        extraMargin = QFixed::fromReal(fm.horizontalAdvance(QChar(0x21B5)));
    }

    const QFixed indent = blockIndent(blockFormat);
    const QFixed totalLeftMargin = QFixed::fromReal(blockFormat.leftMargin()) + (dir == Qt::RightToLeft ? extraMargin : indent);
    const QFixed totalRightMargin = QFixed::fromReal(blockFormat.rightMargin()) + (dir == Qt::RightToLeft ? indent : extraMargin);

    const QPointF oldPosition = tl->position();

    tl->setPosition(QPointF(layoutStruct->x_left.toReal(), layoutStruct->y.toReal()));

    if (layoutStruct->fullLayout
            || (blockPosition + blockLength > from && blockPosition <= to)
            || (layoutStruct->pageHeight != QFIXED_MAX && layoutStruct->absoluteY() + QFixed::fromReal(tl->boundingRect().height()) > layoutStruct->pageBottom))
    {
        QTextOption option = document()->defaultTextOption();
        option.setTextDirection(dir);
        option.setTabs(blockFormat.tabPositions());

        Qt::Alignment align = document()->defaultTextOption().alignment();
        if (blockFormat.hasProperty(QTextFormat::BlockAlignment)) {
            align = blockFormat.alignment();
        }

        //option.setAlignment(QGuiApplicationPrivate::visualAlignment(dir, align));
        Qt::Alignment rtlAlign = align;
        if (!(rtlAlign & Qt::AlignHorizontal_Mask)) {
            rtlAlign |= Qt::AlignLeft;
        }
        if (!(rtlAlign & Qt::AlignAbsolute) && (rtlAlign & (Qt::AlignLeft | Qt::AlignRight))) {
            if (dir == Qt::RightToLeft)
                rtlAlign ^= (Qt::AlignLeft | Qt::AlignRight);
            rtlAlign |= Qt::AlignAbsolute;
        }
        option.setAlignment(rtlAlign);

        if (blockFormat.nonBreakableLines() || document()->pageSize().width() < 0) {
            option.setWrapMode(QTextOption::ManualWrap);
        }

        tl->setTextOption(option);

        const bool haveWordOrAnyWrapMode = (option.wrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere);

        const QFixed cy = layoutStruct->y;
        const QFixed l = layoutStruct->x_left + totalLeftMargin;
        const QFixed r = layoutStruct->x_right - totalRightMargin;
        QFixed bottom;

        tl->beginLayout();
        bool firstLine = true;
        while (1) {
            QTextLine line = tl->createLine();
            if (!line.isValid()) {
                break;
            }
            line.setLeadingIncluded(true);

            QFixed left, right;
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            QFixed text_indent = 0;
            if (firstLine) {
                text_indent = QFixed::fromReal(blockFormat.textIndent());
                if (dir == Qt::LeftToRight) {
                    left += text_indent;
                } else {
                    right -= text_indent;
                }
                firstLine = false;
            }

            line.setLineWidth((right - left).toReal());

            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            if (dir == Qt::LeftToRight) {
                left += text_indent;
            } else {
                right -= text_indent;
            }

            if (QFixed::fromReal(line.naturalTextWidth()) > right - left) {
                // float has been added in the meantime, redo
                layoutStruct->pendingFloats.clear();

                line.setLineWidth((right - left).toReal());
                if (QFixed::fromReal(line.naturalTextWidth()) > right - left) {

                    if (haveWordOrAnyWrapMode) {
                        option.setWrapMode(QTextOption::WrapAnywhere);
                        tl->setTextOption(option);
                    }

                    layoutStruct->pendingFloats.clear();
                    // lines min width more than what we have
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, QFixed::fromReal(line.naturalTextWidth()));
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                    left = qMax(left, l);
                    right = qMin(right, r);
                    if (dir == Qt::LeftToRight) {
                        left += text_indent;
                    } else {
                        right -= text_indent;
                    }
                    line.setLineWidth(qMax<qreal>(line.naturalTextWidth(), (right - left).toReal()));

                    if (haveWordOrAnyWrapMode) {
                        option.setWrapMode(QTextOption::WordWrap);
                        tl->setTextOption(option);
                    }
                }
            }

            QFixed lineBreakHeight, lineHeight, lineAdjustment, lineBottom;
            qreal scaling = (paintDevice() && paintDevice()->logicalDpiY() != 100) ? qreal(paintDevice()->logicalDpiY()) / 100.0 : 1;

            getLineHeightParams(blockFormat, line, scaling, &lineAdjustment, &lineBreakHeight, &lineHeight, &lineBottom);

            #if DEBUG_PRINT==1
            qDebug("(%s) FTextDocumentLayout::layoutBlock => lineHeight: %f", qUtf8Printable(m_name), lineHeight.toReal());
            #endif

            while (layoutStruct->pageHeight > 0 && layoutStruct->absoluteY() + lineBreakHeight > layoutStruct->pageBottom && layoutStruct->contentHeight() >= lineBreakHeight) {
                layoutStruct->newPage();

                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, l);
                right = qMin(right, r);
                if (dir == Qt::LeftToRight) {
                    left += text_indent;
                } else {
                    right -= text_indent;
                }
            }

            line.setPosition(QPointF((left - layoutStruct->x_left).toReal(), (layoutStruct->y - cy - lineAdjustment).toReal()));
            bottom = layoutStruct->y + lineBottom;
            layoutStruct->y += lineHeight;
            layoutStruct->contentsWidth = qMax<QFixed>(layoutStruct->contentsWidth, QFixed::fromReal(line.x() + line.naturalTextWidth()) + totalRightMargin);

            // position floats
            for (int i = 0; i < layoutStruct->pendingFloats.size(); ++i) {
                QTextFrame *f = layoutStruct->pendingFloats.at(i);
                positionFloat(f);
            }
            layoutStruct->pendingFloats.clear();
        }

        layoutStruct->y = qMax(layoutStruct->y, bottom);
        tl->endLayout();
    } else { // Not fullLayout
        const int cnt = tl->lineCount();
        QFixed bottom;
        for (int i = 0; i < cnt; ++i) {
            QTextLine line = tl->lineAt(i);
            layoutStruct->contentsWidth = qMax(layoutStruct->contentsWidth, QFixed::fromReal(line.x() + line.naturalTextWidth()) + totalRightMargin);

            QFixed lineBreakHeight, lineHeight, lineAdjustment, lineBottom;
            qreal scaling = (paintDevice() && paintDevice()->logicalDpiY() != 100) ? qreal(paintDevice()->logicalDpiY()) / 100.0 : 1;
            getLineHeightParams(blockFormat, line, scaling, &lineAdjustment, &lineBreakHeight, &lineHeight, &lineBottom);

            if (layoutStruct->pageHeight != QFIXED_MAX) {
                if (layoutStruct->absoluteY() + lineBreakHeight > layoutStruct->pageBottom) {
                    layoutStruct->newPage();
                }
                line.setPosition(QPointF(line.position().x(), (layoutStruct->y - lineAdjustment).toReal() - tl->position().y()));
            }
            bottom = layoutStruct->y + lineBottom;
            layoutStruct->y += lineHeight;
        }
        layoutStruct->y = qMax(layoutStruct->y, bottom);
        if (layoutStruct->updateRect.isValid() && blockLength > 1) {
            if (from >= blockPosition + blockLength) {
                // if our height didn't change and the change in the document is
                // in one of the later paragraphs, then we don't need to repaint
                // this one
                layoutStruct->updateRect.setTop(qMax(layoutStruct->updateRect.top(), layoutStruct->y.toReal()));
            } else if (to < blockPosition) {
                if (oldPosition == tl->position()) {
                    // if the change in the document happened earlier in the document
                    // and our position did /not/ change because none of the earlier paragraphs
                    // or frames changed their height, then we don't need to repaint
                    // this one
                    layoutStruct->updateRect.setBottom(qMin(layoutStruct->updateRect.bottom(), tl->position().y()));
                } else {
                    layoutStruct->updateRect.setBottom(qreal(INT_MAX));
                }
            }
        }
    }

    // ### doesn't take floats into account. would need to do it per line. but how to retrieve then? (Simon)
    const QFixed margins = totalLeftMargin + totalRightMargin;
    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, QFixed::fromReal(tl->minimumWidth()) + margins);

    const QFixed maxW = QFixed::fromReal(tl->maximumWidth()) + margins;

    if (maxW > 0) {
        if (layoutStruct->maximumWidth == QFIXED_MAX) {
            layoutStruct->maximumWidth = maxW;
        } else {
            layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maxW);
        }
        #if DEBUG_PRINT==1
        qDebug("(%s) maxW > 0 => %f", qUtf8Printable(m_name), layoutStruct->maximumWidth.toReal());
        #endif
    }
}

void FTextDocumentLayout::layoutFlow(QTextFrame::Iterator it, QTextLayoutStruct *layoutStruct, int from, int to, QFixed width)
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow(it?, layoutStruct?, from =" << from << ", to =" << to << ", width =" << width << ")";
    #endif
    QTextFrameData *fd = frameData(layoutStruct->frame);

    fd->currentLayoutStruct = layoutStruct;

    QTextFrame::Iterator previousIt;
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow ==> Iterator Info. Frame =" << it.currentFrame() << ", BlockValid =" << it.currentBlock().isValid() << ", BlockPos =" << it.currentBlock().position();
    #endif

    const bool inRootFrame = (it.parentFrame() == document()->rootFrame());
    if (inRootFrame) {
        bool redoCheckPoints = layoutStruct->fullLayout || checkPoints.isEmpty();
        #if DEBUG_PRINT==1
        qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow ==> inRootFrame. redoCheckPoints?" << redoCheckPoints;
        #endif

        if (!redoCheckPoints) {
            QVector<QCheckPoint>::Iterator checkPointIt = std::lower_bound(checkPoints.begin(), checkPoints.end(), from);
            #if DEBUG_PRINT==1
            qDebug("(%s) FTextDocumentLayout::layoutFlow ==> checkPoint Info. minimumWidth = %.2f, maximumWidth = %.2f, contentsWidth = %.2f", qUtf8Printable(m_name), checkPointIt->minimumWidth.toReal(), checkPointIt->maximumWidth.toReal(), checkPointIt->contentsWidth.toReal());
            #endif
            if (checkPointIt != checkPoints.end()) {
                if (checkPointIt != checkPoints.begin()) {
                    --checkPointIt;
                }
                #if DEBUG_PRINT==1
                qDebug("(%s) FTextDocumentLayout::layoutFlow ==> checkPoint Info After. minimumWidth = %.2f, maximumWidth = %.2f, contentsWidth = %.2f", qUtf8Printable(m_name), checkPointIt->minimumWidth.toReal(), checkPointIt->maximumWidth.toReal(), checkPointIt->contentsWidth.toReal());
                #endif

                layoutStruct->y = checkPointIt->y;
                layoutStruct->frameY = checkPointIt->frameY;
                layoutStruct->minimumWidth = checkPointIt->minimumWidth;
                layoutStruct->maximumWidth = checkPointIt->maximumWidth;
                layoutStruct->contentsWidth = checkPointIt->contentsWidth;

                if (layoutStruct->pageHeight > 0) {
                    int page = layoutStruct->currentPage();
                    layoutStruct->pageBottom = (page + 1) * layoutStruct->pageHeight - layoutStruct->pageBottomMargin;
                }

                it = frameIteratorForTextPosition(checkPointIt->positionInFrame);
                checkPoints.resize(int(checkPointIt - checkPoints.begin() + 1));

                if (checkPointIt != checkPoints.begin()) {
                    previousIt = it;
                    --previousIt;
                }
            } else {
                redoCheckPoints = true;
                #if DEBUG_PRINT==1
                qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow ==> checkPoint is at the end -> redoCheckPoints = true";
                #endif
            }
        }

        if (redoCheckPoints) {
            checkPoints.clear();
            QCheckPoint cp;
            cp.y = layoutStruct->y;
            cp.frameY = layoutStruct->frameY;
            cp.positionInFrame = 0;
            cp.minimumWidth = layoutStruct->minimumWidth;
            cp.maximumWidth = layoutStruct->maximumWidth;
            cp.contentsWidth = layoutStruct->contentsWidth;
            checkPoints.append(cp);
        }
    }

    QTextBlockFormat previousBlockFormat = previousIt.currentBlock().blockFormat();

    QFixed maximumBlockWidth = 0;
    while (!it.atEnd()) {
        QTextFrame *c = it.currentFrame();

        int docPos;
        if (it.currentFrame()) {
            docPos = it.currentFrame()->firstPosition();
        } else {
            docPos = it.currentBlock().position();
        }

        if (inRootFrame) {
            if (qAbs(layoutStruct->y - checkPoints.constLast().y) > 2000) {
                QFixed left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                if (left == layoutStruct->x_left && right == layoutStruct->x_right) {
                    QCheckPoint p;
                    p.y = layoutStruct->y;
                    p.frameY = layoutStruct->frameY;
                    p.positionInFrame = docPos;
                    p.minimumWidth = layoutStruct->minimumWidth;
                    p.maximumWidth = layoutStruct->maximumWidth;
                    p.contentsWidth = layoutStruct->contentsWidth;
                    checkPoints.append(p);

                    if (currentLazyLayoutPosition != -1 && docPos > currentLazyLayoutPosition + lazyLayoutStepSize) {
                        break;
                    }
                }
            }
        }

        if (c) {
            // position child frame
            QTextFrameData *cd = frameData(c);
            QTextFrameFormat fformat = c->frameFormat();

            if (fformat.position() == QTextFrameFormat::InFlow) {
                if (fformat.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) {
                    layoutStruct->newPage();
                }

                QFixed left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, layoutStruct->x_left);
                right = qMin(right, layoutStruct->x_right);

                if (right - left < cd->size.width) {
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, cd->size.width);
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                }

                QFixedPoint pos(left, layoutStruct->y);

                Qt::Alignment align = Qt::AlignLeft;

//                QTextTable *table = qobject_cast<QTextTable*>(c);
//                if (table) {
//                    align = table->format().alignment() & Qt::AlignHorizontal_Mask;
//                }

                // detect whether we have any alignment in the document that disallows optimizations,
                // such as not laying out the document again in a textedit with wrapping disabled.
                if (inRootFrame && !(align & Qt::AlignLeft)) {
                    contentHasAlignment = true;
                }

                cd->position = pos;

                if (document()->pageSize().height() > 0) {
                    cd->sizeDirty = true;
                }

                if (cd->sizeDirty) {
                    if (width != 0) {
                        layoutFrameEx(c, from, to, width, -1, layoutStruct->frameY);
                    } else {
                        layoutFrame(c, from, to, layoutStruct->frameY);
                    }

                    //QFixed absoluteChildPos = table ? pos.y + static_cast<QTextTableData *>(data(table))->rowPositions.at(0) : pos.y + firstChildPos(c);
                    QFixed absoluteChildPos = pos.y + firstChildPos(c);
                    absoluteChildPos += layoutStruct->frameY;

                    // drop entire frame to next page if first child of frame is on the next page
                    if (absoluteChildPos > layoutStruct->pageBottom) {
                        #if DEBUG_PRINT==1
                        qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow => drop frame to next page. absoluteChildPos (" << absoluteChildPos << ") > layoutStruct->pageBottom (" << layoutStruct->pageBottom << ")";
                        #endif
                        layoutStruct->newPage();
                        pos.y = layoutStruct->y;

                        cd->position = pos;
                        cd->sizeDirty = true;

                        if (width != 0) {
                            layoutFrameEx(c, from, to, width, -1, layoutStruct->frameY);
                        } else {
                            layoutFrame(c, from, to, layoutStruct->frameY);
                        }
                    }
                }

                // align only if there is space for alignment
                if (right - left > cd->size.width) {
                    if (align & Qt::AlignRight) {
                        pos.x += layoutStruct->x_right - cd->size.width;
                    } else if (align & Qt::AlignHCenter) {
                        pos.x += (layoutStruct->x_right - cd->size.width) / 2;
                        #if DEBUG_PRINT==1
                        qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow => AlignHCenter. pos.x =" << pos.x;
                        #endif
                    }
                }

                cd->position = pos;

                layoutStruct->y += cd->size.height;
                const int page = layoutStruct->currentPage();
                layoutStruct->pageBottom = (page + 1) * layoutStruct->pageHeight - layoutStruct->pageBottomMargin;

                cd->layoutDirty = false;

                if (c->frameFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter) {
                    layoutStruct->newPage();
                }
            } else {
                // frameFormat position != Flow => Either FloatLeft or FloatRight
                QRectF oldFrameRect(cd->position.toPointF(), cd->size.toSizeF());
                QRectF updateRect;

                if (cd->sizeDirty) {
                    updateRect = layoutFrame(c, from, to);
                }

                positionFloat(c);

                // If the size was made dirty when the position was set, layout again
                if (cd->sizeDirty) {
                    updateRect = layoutFrame(c, from, to);
                }

                QRectF frameRect(cd->position.toPointF(), cd->size.toSizeF());

                if (frameRect == oldFrameRect && updateRect.isValid()) {
                    updateRect.translate(cd->position.toPointF());
                } else {
                    updateRect = frameRect;
                }

                layoutStruct->addUpdateRectForFloat(updateRect);
                if (oldFrameRect.isValid()) {
                    layoutStruct->addUpdateRectForFloat(oldFrameRect);
                }
            } // if frameFormat position end

            layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, cd->minimumWidth);
            layoutStruct->maximumWidth = qMin(layoutStruct->maximumWidth, cd->maximumWidth);

            previousIt = it;
            ++it;
        } else { // No valid textFrame
            QTextFrame::Iterator lastIt;
            if (!previousIt.atEnd() && previousIt != it) {
                lastIt = previousIt;
            }
            previousIt = it;
            QTextBlock block = it.currentBlock();
            ++it;

            const QTextBlockFormat blockFormat = block.blockFormat();
            if (blockFormat.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) {
                layoutStruct->newPage();
            }

//            const QFixed origY = layoutStruct->y;
//            const QFixed origPageBottom = layoutStruct->pageBottom;
            const QFixed origMaximumWidth = layoutStruct->maximumWidth;
            #if DEBUG_PRINT==1
            qDebug("(%s) origMaximumWidth: %f", qUtf8Printable(m_name), layoutStruct->maximumWidth.toReal());
            #endif
            layoutStruct->maximumWidth = 0;

            const QTextBlockFormat *previousBlockFormatPtr = nullptr;
            if (lastIt.currentBlock().isValid()) {
                previousBlockFormatPtr = &previousBlockFormat;
            }

            // layout and position child block
            #if DEBUG_PRINT==1
            qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutFlow => blockFormat Alignment:" << blockFormat.alignment() << ", lineHeight:" << blockFormat.lineHeight() << ", lineHeightType:" << blockFormat.lineHeightType();
            #endif
            layoutBlock(block, docPos, blockFormat, layoutStruct, from, to, previousBlockFormatPtr);

            // detect whether we have any alignment in the document that disallows optimizations,
            // such as not laying out the document again in a textedit with wrapping disabled
            if (inRootFrame && !(block.layout()->textOption().alignment() & Qt::AlignLeft)) {
                contentHasAlignment = true;
            }

            // if the block right before a table is empty 'hide' it by
            // positioning it into the table border

            // ==> removed (do not support tables)

            if (blockFormat.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter) {
                layoutStruct->newPage();
            }

            maximumBlockWidth = qMax(maximumBlockWidth, layoutStruct->maximumWidth);
            #if DEBUG_PRINT==1
            qDebug("(%s) maximumBlockWidth: %.2f", qUtf8Printable(m_name), maximumBlockWidth.toReal());
            #endif
            layoutStruct->maximumWidth = origMaximumWidth;
            previousBlockFormat = blockFormat;
        }
    }

    if (layoutStruct->maximumWidth == QFIXED_MAX && maximumBlockWidth > 0) {
        layoutStruct->maximumWidth = maximumBlockWidth;
    } else {
        layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maximumBlockWidth);
    }

    // a float at the bottom of a frame may make it taller, hence the qMax() for layoutStruct->y.
    // we don't need to do it for tables though because floats in tables are per table
    // and not per cell and layoutCell already takes care of doing the same as we do here

    // ==> removed (do not support tables)

    if (inRootFrame) {
        // we assume that any float is aligned in a way that disallows the optimizations that rely
        // on unaligned content.
        if (!fd->floats.isEmpty()) {
            contentHasAlignment = true;
        }

        if (it.atEnd()) {
            currentLazyLayoutPosition = -1;
            QCheckPoint cp;
            cp.y = layoutStruct->y;
            cp.positionInFrame = document()->characterCount();
            cp.minimumWidth = layoutStruct->minimumWidth;
            cp.maximumWidth = layoutStruct->maximumWidth;
            cp.contentsWidth = layoutStruct->contentsWidth;
            checkPoints.append(cp);
            checkPoints.reserve(checkPoints.size());
        } else {
            currentLazyLayoutPosition = checkPoints.constLast().positionInFrame;
        }
    }

    fd->currentLayoutStruct = nullptr;
}

void FTextDocumentLayout::layoutFinished()
{
    layoutTimer.stop();
    if (!insideDocumentChange) {
        sizeChangedTimer.start(0, this);
    }
    // reset
    showLayoutProgress = true;
}

void FTextDocumentLayout::layoutStep() const
{
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::layoutStep ==> " << currentLazyLayoutPosition << lazyLayoutStepSize;
    #endif
    ensureLayoutedByPosition(currentLazyLayoutPosition + lazyLayoutStepSize);
    lazyLayoutStepSize = qMin(200000, lazyLayoutStepSize * 2);
}

void FTextDocumentLayout::ensureLayouted(QFixed y) const
{
    if (currentLazyLayoutPosition == -1) {
        return;
    }
    if (checkPoints.isEmpty()) {
        layoutStep();
    }

    while (currentLazyLayoutPosition != -1 && checkPoints.last().y < y) {
        layoutStep();
    }
}

void FTextDocumentLayout::ensureLayoutedByPosition(int position) const
{
    if (currentLazyLayoutPosition == -1) {
        return;
    }
    if (position < currentLazyLayoutPosition) {
        return;
    }
    while (currentLazyLayoutPosition != -1 && currentLazyLayoutPosition < position) {
        const_cast<FTextDocumentLayout*>(this)->doLayout(currentLazyLayoutPosition, 0, INT_MAX - currentLazyLayoutPosition);
    }
}

QTextFrame::Iterator FTextDocumentLayout::frameIteratorForYPosition(QFixed y) const
{
    QTextFrame *rootFrame = document()->rootFrame();

    if (checkPoints.isEmpty() || y < 0 || y > frameData(rootFrame)->size.height) {
        return rootFrame->begin();
    }

    QVector<QCheckPoint>::ConstIterator checkPointIt = std::lower_bound(checkPoints.begin(), checkPoints.end(), y);
    if (checkPointIt == checkPoints.end()) {
        return rootFrame->begin();
    }

    if (checkPointIt != checkPoints.begin()) {
        --checkPointIt;
    }

    const int position = rootFrame->firstPosition() + checkPointIt->positionInFrame;
    return frameIteratorForTextPosition(position);
}

// Cannot fully implement this function like in the default QTextDocumentLayout
// Shouldn't be a problem as long as we only have one frame which is the root frame
QTextFrame::Iterator FTextDocumentLayout::frameIteratorForTextPosition(int position) const
{
    Q_UNUSED(position);
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::frameIteratorForTextPosition(position =" << position << ")";
    #endif
    //document()->characterCount() == docPrivate->length()
    //document()->blockCount() == blockMap()->numNodes()

    QTextFrame *rootFrame = document()->rootFrame();

//    const int begin = document()->findBlock(rootFrame->firstPosition()).position();
//    const int end = document()->findBlock(rootFrame->lastPosition()+1).position();

    #if DEBUG_PRINT==1
    QTextBlock block = document()->findBlock(position);
    const int blockPos = block.position();
    QTextFrame *containingFrame = document()->frameAt(blockPos);
    qDebug() << "("<<m_name<<") FTextDocumentLayout::frameIteratorForTextPosition ==> Before loop: Frame =" << containingFrame << ", BlockPos =" << blockPos << ", BlockValid =" << block.isValid() << ", rootFrame =" << rootFrame;
    #endif

    // Can't use this. Great.
    // QTextFrame::Iterator it(rootFrame, block.blockNumber(), begin, end);

    QTextFrame::Iterator it;
    it = rootFrame->begin();

//    if (containingFrame == rootFrame) {
//        qDebug() << "("<<m_name<<") FTextDocumentLayout::frameIteratorForTextPosition ==> containingFrame is rootFrame";
//        // Move iterator to start of block
//        while (!it.atEnd() && it.currentBlock().isValid() && it.currentBlock() != block) {
//            it++;
//        }
//        if (it.atEnd()) {
//            qDebug() << "("<<m_name<<") FTextDocumentLayout::frameIteratorForTextPosition ==> Couldn't find block";
//            it = rootFrame->begin();
//        }
//    }
    #if DEBUG_PRINT==1
    qDebug() << "("<<m_name<<") FTextDocumentLayout::frameIteratorForTextPosition ==> After loop: Frame =" << it.currentFrame() << ", BlockPos =" << it.currentBlock().position()  << ", BlockValid =" << it.currentBlock().isValid();
    #endif

    return it;
}

void FTextDocumentLayout::positionFloat(QTextFrame *frame, QTextLine *currentLine)
{
    QTextFrameData *fd = frameData(frame);
    QTextFrame *parent = frame->parentFrame();
    Q_ASSERT(parent);
    QTextFrameData *pd = frameData(parent);
    Q_ASSERT(pd && pd->currentLayoutStruct);

    QTextLayoutStruct *layoutStruct = pd->currentLayoutStruct;

    if (!pd->floats.contains(frame)) {
        pd->floats.append(frame);
    }
    fd->layoutDirty = true;
    Q_ASSERT(!fd->sizeDirty);

    QFixed y = layoutStruct->y;
    if (currentLine) {
        QFixed left, right;
        floatMargins(y, layoutStruct, &left, &right);
        if (right - left < QFixed::fromReal(currentLine->naturalTextWidth()) + fd->size.width) {
            layoutStruct->pendingFloats.append(frame);
            return;
        }
    }

    bool framesSpansIntoNextPage = (y + layoutStruct->frameY + fd->size.height > layoutStruct->pageBottom);
    if (framesSpansIntoNextPage && fd->size.height <= layoutStruct->pageHeight) {
        layoutStruct->newPage();
        y = layoutStruct->y;

        framesSpansIntoNextPage = false;
    }

    y = findY(y, layoutStruct, fd->size.width);

    QFixed left, right;
    floatMargins(y, layoutStruct, &left, &right);

    if (frame->frameFormat().position() == QTextFrameFormat::FloatLeft) {
        fd->position.x = left;
        fd->position.y = y;
    } else {
        fd->position.x = right - fd->size.width;
        fd->position.y = y;
    }

    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, fd->minimumWidth);
    layoutStruct->maximumWidth = qMin(layoutStruct->maximumWidth, fd->maximumWidth);

    fd->layoutDirty = false;
}

void FTextDocumentLayout::floatMargins(const QFixed &y, const QTextLayoutStruct *layoutStruct, QFixed *left, QFixed *right) const
{
    *left = layoutStruct->x_left;
    *right = layoutStruct->x_right;

    QTextFrameData *lfd = frameData(layoutStruct->frame);
    for (int i = 0; i < lfd->floats.size(); ++i) {
        QTextFrameData *fd = frameData(lfd->floats.at(i));
        if (fd->layoutDirty) {
            if (fd->position.y <= y && fd->position.y + fd->size.height > y) {
                if (lfd->floats.at(i)->frameFormat().position() == QTextFrameFormat::FloatLeft) {
                    *left = qMax(*left, fd->position.x + fd->size.width);
                } else {
                    *right = qMin(*right, fd->position.x);
                }
            }
        }
    }
}

QFixed FTextDocumentLayout::findY(QFixed yFrom, const QTextLayoutStruct *layoutStruct, QFixed requiredWidth) const
{
    QFixed left, right;
    requiredWidth = qMin(requiredWidth, layoutStruct->x_right - layoutStruct->x_left);

    while (1) {
        floatMargins(yFrom, layoutStruct, &left, &right);
        if (right - left >= requiredWidth) {
            break;
        }

        // move float down until we find enough space
        QFixed newY = QFIXED_MAX;
        QTextFrameData *lfd = frameData(layoutStruct->frame);
        for (int i = 0; i < lfd->floats.size(); ++i) {
            QTextFrameData *fd = frameData(lfd->floats.at(i));
            if (!fd->layoutDirty) {
                if (fd->position.y <= yFrom && fd->position.y + fd->size.height > yFrom) {
                    newY = qMin(newY, fd->position.y + fd->size.height);
                }
            }
        }
        if (newY == QFIXED_MAX) {
            break;
        }
        yFrom = newY;
    }
    return yFrom;
}

void FTextDocumentLayout::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == layoutTimer.timerId()) {
        if (currentLazyLayoutPosition != -1) {
            layoutStep();
        }
    } else if (e->timerId() == sizeChangedTimer.timerId()) {
        lastReportedSize = dynamicDocumentSize();
        #if DEBUG_PRINT==1
        qDebug("(%s) Last reported size: %.2f, %.2f", qUtf8Printable(m_name), lastReportedSize.width(), lastReportedSize.height());
        #endif
        emit documentSizeChanged(lastReportedSize);
        sizeChangedTimer.stop();

        if (currentLazyLayoutPosition == -1) {
            const int newCount = dynamicPageCount();
            if (newCount != lastPageCount) {
                lastPageCount = newCount;
                emit pageCountChanged(newCount);
            }
        }
    } else {
        QAbstractTextDocumentLayout::timerEvent(e);
    }
}


void FTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    QTextCharFormat fmt = format.toCharFormat();
    Q_ASSERT(fmt.isValid());

    QTextObjectInterface *iface = this->handlerForObject(item.format().objectType());
    if (!iface) {
        return;
    }

    QSizeF intrinsic = iface->intrinsicSize(document(), posInDocument, format);

    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qobject_cast<QTextFrame*>(document()->objectForFormat(fmt));
    if (frame) {
        pos = frame->frameFormat().position();
        QTextFrameData *fd = frameData(frame);
        fd->sizeDirty = false;
        fd->minimumWidth = fd->maximumWidth = fd->size.width;
    }

    QSizeF inlineSize = (pos == QTextFrameFormat::InFlow ? intrinsic : QSizeF(0, 0));
    item.setWidth(inlineSize.width());

    if (fmt.verticalAlignment() == QTextCharFormat::AlignMiddle) {
        QFontMetricsF fm(fmt.font());
        qreal textMiddle = fm.height()/2 - fm.descent();
        item.setAscent(inlineSize.height()/2 + textMiddle);
        item.setDescent(inlineSize.height()/2 - textMiddle);
    } else {
        item.setDescent(0);
        item.setAscent(inlineSize.height());
    }
}
