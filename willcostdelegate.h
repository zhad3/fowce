#ifndef WILLCOSTDELEGATE_H
#define WILLCOSTDELEGATE_H

#include <QStyledItemDelegate>
#include <QSpinBox>

class WillCostDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    WillCostDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif // WILLCOSTDELEGATE_H
