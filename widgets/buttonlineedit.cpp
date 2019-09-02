#include <QApplication>
#include <QPainter>
#include <QStylePainter>
#include <QPaintEvent>
#include <QStyleOptionFrame>
#include <QLineEdit>
#include <QPushButton>

#include "widgets/buttonlineedit.h"

//class ButtonLineEditPrivate{};

ButtonLineEdit::ButtonLineEdit(QWidget *parent) :
    QWidget(parent), m_readOnly(0), m_isHover(0),
    m_isDown(0), m_ascent(0), m_contentRect(rect()),
    m_topmargin(0), m_rightmargin(0), m_bottommargin(0),
    m_leftmargin(0)
    //d_ptr(new ButtonLineEditPrivate)
{
    m_textLayout.setCacheEnabled(true);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize ButtonLineEdit::sizeHint() const
{
    ensurePolished();
    int w = 0;
    int h = 0;
    QStyleOptionButton option;
    initStyleOptionButton(&option);

    QString s(text());
    bool empty = s.isEmpty();
    if (empty) {
        s = QStringLiteral("XXXX");
    }
    QFontMetrics fm = fontMetrics();
    QSize sz = fm.size(Qt::TextShowMnemonic | Qt::TextSingleLine/*| Qt::TextWordWrap*/, s);
    qDebug(qUtf8Printable(QString::number(sz.width())));
    qDebug(qUtf8Printable(QString::number(sz.height())));
    if (!empty || !w) {
        w += sz.width();
    }
    if (!empty || !h) {
        h = qMax(h, sz.height());
    }
    option.rect.setSize(QSize(w,h));
    return style()->sizeFromContents(QStyle::CT_PushButton, &option, QSize(w,h), this).
            expandedTo(QApplication::globalStrut());
}

QString ButtonLineEdit::text() const {
    return m_data;
}

void ButtonLineEdit::setText(const QString text) {
    m_data = text;
    m_textLayout.setText(text);
    QTextOption textopt = m_textLayout.textOption();
    textopt.setTextDirection(isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
    textopt.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    textopt.setWrapMode(/*QTextOption::WrapAtWordBoundaryOrAnywhere*/QTextOption::NoWrap);
    m_textLayout.setTextOption(textopt);
    m_textLayout.setFont(font());
    m_textLayout.setFlags(QTextOption::IncludeTrailingSpaces);
    m_ascent = redoTextLayout();
    updateGeometry();
    update();
}

void ButtonLineEdit::setReadOnly(bool enable) {
    if (m_readOnly == enable) {
        return;
    }
    m_readOnly = enable;
}

void ButtonLineEdit::paintEvent(QPaintEvent *event)
{
    //Q_D(ButtonLineEdit);
    QPainter p(this);
    QFontMetrics fm(font());
    int horizontalMargin = 4; // default 2 (taken from qlineedit_p.cpp)
    int verticalMargin = 1; // default 1 (taken from qlineedit_p.cpp)
    QRect r(rect());


    QLineEdit dummyLE;
    QStyleOptionFrame option;
    QStyleOptionButton optionBTN;

    option.initFrom(&dummyLE);
    initStyleOptionFrame(&option);
    r = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);

    optionBTN.initFrom(this);
    initStyleOptionButton(&optionBTN);

    if (!m_isHover) {
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &p, this);
    } else {
        style()->drawControl(QStyle::CE_PushButton, &optionBTN, &p, this);
    }
    p.setClipRect(r);

    // Vertical Align
    //QRect lineRect(r.x() + horizontalMargin, r.y() + (r.height() - fm.height() + 1) / 2, r.width() - 2 * horizontalMargin, fm.height());
    QRect lineRect(r.x() + horizontalMargin, r.y() + verticalMargin, r.width() - 2 * horizontalMargin, r.height());
    //QPoint topLeft = lineRect.topLeft() - QPoint(0, fm.ascent());
    lineRect.setTop(lineRect.top() - (m_ascent - fm.ascent()));

    //textLayout()->draw(&p, topLeft, QVector<QTextLayout::FormatRange>(), r);
    const Qt::LayoutDirection layoutDir = isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
    const Qt::Alignment alignPhText = QStyle::visualAlignment(layoutDir, Qt::AlignLeft | Qt::AlignTop);
    p.drawText(lineRect, alignPhText | Qt::TextWordWrap, m_data);
}

void ButtonLineEdit::enterEvent(QEvent *event)
{
    m_isHover = true;
    update();
    QWidget::enterEvent(event);
}


void ButtonLineEdit::leaveEvent(QEvent *event)
{
    m_isHover = false;
    update();
    QWidget::leaveEvent(event);
}


void ButtonLineEdit::mousePressEvent(QMouseEvent *event)
{
    m_isDown = true;
    update();
    QWidget::mousePressEvent(event);
}

void ButtonLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDown = false;
    update();
    QWidget::mouseReleaseEvent(event);
}

void ButtonLineEdit::initStyleOptionButton(QStyleOptionButton *option) const {
    option->features = QStyleOptionButton::None;
    if (isReadOnly()) {
        option->state |= QStyle::State_ReadOnly;
    }
    if (isDown()) {
        option->state |= QStyle::State_Sunken;
    } else {
        option->state |= QStyle::State_Raised;
    }
    //option->text = text(); //This will automatically draw text on the center
}

void ButtonLineEdit::initStyleOptionFrame(QStyleOptionFrame *option) const {
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (isReadOnly()) {
        option->state |= QStyle::State_ReadOnly;
    }
    option->features = QStyleOptionFrame::None;
}

int ButtonLineEdit::redoTextLayout() const
{
    QTextOption to;
    to.setWrapMode(QTextOption::NoWrap);
    m_textLayout.clearLayout();
    m_textLayout.beginLayout();
    m_textLayout.setTextOption(to);
    QTextLine l = m_textLayout.createLine();
    m_textLayout.endLayout();

    return qRound(l.ascent());
}

QSize ButtonLineEdit::sizeForWidth(int w) const
{
    if (minimumWidth() > 0) {
        w = qMax(w, minimumWidth());
    }
    QSize contentsMargin(m_leftmargin + m_rightmargin, m_topmargin + m_bottommargin);
    int hextra = contentsMargin.width();
    int vextra = contentsMargin.height();
    QRect br;
    QFontMetrics fm = fontMetrics();

    const Qt::LayoutDirection layoutDir = isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
    int align = QStyle::visualAlignment(layoutDir, Qt::AlignTop | Qt::AlignLeft);

    return QSize();
}


