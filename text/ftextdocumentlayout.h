#ifndef FTEXTDOCUMENTLAYOUT_H
#define FTEXTDOCUMENTLAYOUT_H

#include <QAbstractTextDocumentLayout>
#include <QTextFrameLayoutData>
#include <QVector>
#include <QBasicTimer>
#include <limits>
#include "qfixed_p.h"

struct QCheckPoint
{
    QFixed y;
    QFixed frameY; // absolute y position of the current frame
    int positionInFrame;
    QFixed minimumWidth;
    QFixed maximumWidth;
    QFixed contentsWidth;
};
Q_DECLARE_TYPEINFO(QCheckPoint, Q_PRIMITIVE_TYPE);
struct QTextLayoutStruct;
class QTextFrameData;

class FTextDocumentLayout : public QAbstractTextDocumentLayout
{
public:
    explicit FTextDocumentLayout(QTextDocument *document, const QString &name = QString());

public:
    void draw(QPainter *painter, const PaintContext &context) override;
    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const override;
    int pageCount() const override;
    QSizeF documentSize() const override;
    QRectF frameBoundingRect(QTextFrame *frame) const override;
    QRectF blockBoundingRect(const QTextBlock &block) const override;

    qreal idealWidth() const;
    void setViewport(const QRectF &viewport) { viewportRect = viewport; }

protected:
    void documentChanged(int from, int charsRemoved, int charsAdded) override;
    void resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format) override;

private:
    QString m_name;
    qreal m_idealWidth;
    bool m_useClip;
    QBasicTimer layoutTimer;
    QBasicTimer sizeChangedTimer;
    QVector<QCheckPoint> checkPoints;
    bool contentHasAlignment;
    bool showLayoutProgress;
    bool insideDocumentChange;
    mutable int currentLazyLayoutPosition;
    mutable int lazyLayoutStepSize;
    int lastPageCount;

    QSizeF lastReportedSize;
    QRectF viewportRect;
    QRectF clipRect;

    int dynamicPageCount() const;
    QSizeF dynamicDocumentSize() const;
    QRectF frameBoundingRectInternal(QTextFrame *frame) const;
    QFixed blockIndent(const QTextBlockFormat &blockFormat) const;

    void drawFrame(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextFrame *frame) const;
    void drawFlow(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context, QTextFrame::Iterator it,
                  const QList<QTextFrame*> &floats, QTextBlock *cursorBlockNeedingRepaint) const;
    void drawBlock(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   const QTextBlock &bl, bool inRootFrame) const;
    void drawBorder(QPainter *painter, const QRectF &rect, qreal topMargin, qreal bottomMargin, qreal border,
                    const QBrush &brush, const QTextFrameFormat::BorderStyle style) const;
    void drawFrameDecoration(QPainter *painter, QTextFrame *frame, QTextFrameData *fd, const QRectF &clip, const QRectF &rect) const;

    void markFrames(QTextFrame *current, int from, int charsRemoved, int charsAdded);
    QRectF doLayout(int from, int charsRemoved, int charsAdded);
    QRectF layoutFrame(QTextFrame *frame, int from, int to, QFixed parentY = 0);
    QRectF layoutFrameEx(QTextFrame *frame, int from, int to, QFixed width, QFixed height, QFixed parentY);
    void layoutBlock(const QTextBlock &bl, int blockPosition, const QTextBlockFormat &blockFormat, QTextLayoutStruct *layoutStruct,
                     int from, int to, const QTextBlockFormat *previousBlockFormat);
    void layoutFlow(QTextFrame::Iterator it, QTextLayoutStruct *layoutStruct, int from, int to, QFixed width = 0);

    void layoutFinished();
    void layoutStep() const;
    void ensureLayouted(QFixed y) const;
    void ensureLayoutedByPosition(int position) const;
    inline void ensureLayoutFinished() const { ensureLayoutedByPosition(INT_MAX); }
    QTextFrame::Iterator frameIteratorForYPosition(QFixed y) const;
    QTextFrame::Iterator frameIteratorForTextPosition(int position) const;
    void positionFloat(QTextFrame *frame, QTextLine *currentLine = nullptr);
    void floatMargins(const QFixed &y, const QTextLayoutStruct *layoutStruct, QFixed *left, QFixed *right) const;
    QFixed findY(QFixed yFrom, const QTextLayoutStruct *layoutStruct, QFixed requiredWidth) const;

    void timerEvent(QTimerEvent *e) override;
};

#endif // FTEXTDOCUMENTLAYOUT_H
