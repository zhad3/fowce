#include <QMenu>
#include "cardsidepushbutton.h"
#include "util.h"

CardSidePushButton::CardSidePushButton(QWidget *parent)
{
    CardSidePushButton(0, parent);
}
CardSidePushButton::CardSidePushButton(int side, QWidget *parent) :
    QPushButton(parent), m_card(nullptr), m_side(side)
{
    setCheckable(true);

    act_edit = new QAction(tr("Edit"));
    act_delete = new QAction(tr("Delete"));

    menu = new QMenu(this);
    menu->addAction(act_edit);
    menu->addSeparator();
    menu->addAction(act_delete);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &CardSidePushButton::customContextMenuRequested, this, &CardSidePushButton::customContextMenu);
    connect(act_edit, &QAction::triggered, this, &CardSidePushButton::ActionEdit);
    connect(act_delete, &QAction::triggered, this, &CardSidePushButton::ActionDelete);

}

void CardSidePushButton::setSide(int side)
{
    if (side >= 0 && side < MAX_CARDSIDES) {
        m_side = side;
    }
}

int CardSidePushButton::side() const
{
    return m_side;
}

void CardSidePushButton::customContextMenu(QPoint pos)
{
    menu->exec(QWidget::mapToGlobal(pos));
}

void CardSidePushButton::ActionEdit(bool /*checked*/)
{
    emit menu_edit();
}

void CardSidePushButton::ActionDelete(bool /*checked*/)
{
    emit menu_delete();
}
