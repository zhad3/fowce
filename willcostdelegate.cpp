#include "willcostdelegate.h"
#include "willcostmodel.h"
#include "util.h"

WillCostDelegate::WillCostDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}


QWidget *WillCostDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    const WillCost *cost = static_cast<const WillCostModel*>(index.model())->getCost(index);

    if (index.column() == 2) {
        int maxCost = MAX_CARD_COST;
        if (cost->dynamic) {
            maxCost = MAX_CARD_COST_DYNAMIC;
        } else if (!cost->characteristic) {
            maxCost = cost->attribute->maxCost();
        }
        QSpinBox *editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setRange(0, maxCost);
        return editor;
    }
    return nullptr;
}

void WillCostDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 2) {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }
}

void WillCostDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == 2) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        model->setData(index, spinBox->value(), Qt::EditRole);
    }
}
