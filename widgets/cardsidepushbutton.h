#ifndef CARDSIDEPUSHBUTTON_H
#define CARDSIDEPUSHBUTTON_H

#include <QPushButton>
#include "card.h"

class CardSidePushButton : public QPushButton
{
    Q_OBJECT
public:
    CardSidePushButton(QWidget *parent = nullptr);
    CardSidePushButton(int side = 0, QWidget *parent = nullptr);
    void setSide(int side);
    int side() const;

    void setCard(Card *card) { m_card = card; }
    Card *card() { return m_card; }

private:
    Card *m_card;

    QMenu *menu;
    int m_side;
    QAction *act_edit;
    QAction *act_delete;
    void customContextMenu(QPoint pos);
    void ActionEdit(bool checked);
    void ActionDelete(bool checked);

signals:
    void menu_edit();
    void menu_delete();
};

#endif // CARDSIDEPUSHBUTTON_H
