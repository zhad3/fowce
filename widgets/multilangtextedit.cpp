#include <QPainter>
#include <QStyleOptionFrame>
#include "multilangtextedit.h"

MultiLangTextEdit::MultiLangTextEdit(QWidget *parent, const FLanguageModel *languageModel, Qt::ItemDataRole role)
    : QPlainTextEdit(parent), m_role(role)
{
    if (role == Qt::DisplayRole)
        setReadOnly(true);
    if (!languageModel) {
        m_languageModel = FLanguageModel::Instance();
    }
}

void MultiLangTextEdit::setLanguageString(FLanguageString langstring)
{
    m_langstring = langstring;
    setPlainText(m_langstring.text());
}

void MultiLangTextEdit::setRole(Qt::ItemDataRole role)
{
    if (role == Qt::EditRole) {
        setReadOnly(false);
    } else if (role == Qt::DisplayRole) {
        setReadOnly(true);
    }
    m_role = role;
}

void MultiLangTextEdit::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);

    if (m_role == Qt::DisplayRole) {
        QPainter painter(viewport());

        QStyleOptionFrame panel;
        initStyleOption(&panel);

        QRect text_rect = this->rect();
        text_rect.setX(text_rect.x() + 5);
        text_rect.setWidth(text_rect.width() - 10);

        QFont smallerFont = painter.font();
        smallerFont.setPointSize(smallerFont.pointSize() - 2);
        painter.setFont(smallerFont);

        QString filled_out_text = QString::number(m_langstring.getFilledOut()) + QString("/") + QString::number(m_languageModel->dataCount());
        QRect text_bounding = painter.boundingRect(text_rect, Qt::AlignRight | Qt::AlignVCenter, filled_out_text);
        text_bounding.setX(text_bounding.x() - 3);
        text_bounding.setWidth(text_bounding.width() + 3);
        text_bounding.setY(text_bounding.y() - 1);
        text_bounding.setHeight(text_bounding.height() + 1);

        QPainterPath roundedRectPath;
        roundedRectPath.addRoundedRect(text_bounding, 2, 2);

        QBrush background_brush;
        background_brush.setStyle(Qt::SolidPattern);
        if (m_langstring.text().isEmpty()) {
            background_brush.setColor(Util::Red);
        } else if (m_langstring.getFilledOut() < m_languageModel->dataCount()) {
            background_brush.setColor(Util::Blue);
        } else {
            background_brush.setColor(Util::Green);
        }

        painter.fillPath(roundedRectPath, background_brush);
        painter.drawText(text_rect, Qt::AlignRight | Qt::AlignVCenter, filled_out_text);
    }
}
