#ifndef WILLCOSTMODEL_H
#define WILLCOSTMODEL_H

#include <QAbstractTableModel>
#include "models/fattributemodel.h"

class WillCost
{
public:
    WillCost() : attribute(nullptr), characteristic(nullptr), cost(0), generic(false), dynamic(false) {}
    WillCost(const FAttribute *attribute, const FWillCharacteristic *characteristic, int cost, bool generic, bool dynamic)
        : attribute(attribute), characteristic(characteristic), cost(cost), generic(generic), dynamic(dynamic) {}

    const FAttribute *attribute;
    const FWillCharacteristic *characteristic;
    int cost;
    bool generic;
    bool dynamic;
};

class WillCostModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    WillCostModel(QObject *parent = nullptr, const FAttributeModel *attributeModel = nullptr);

    const FAttribute *getAttribute(const QModelIndex &index) const;
    const WillCost *getCost(const QModelIndex &index) const;

    int rowCount(const QModelIndex &/*parent*/) const override { return m_rowCount; }
    int columnCount(const QModelIndex &/*parent*/) const override { return m_columnCount; }
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
    const FAttributeModel *m_attributeModel;
    QVector<WillCost> costs;
    int m_rowCount;
    int m_columnCount;
    int m_totalNonGenericCost;
    int m_totalGenericCost;
    bool m_costExceeded;

signals:
    void costExceeded(bool exceeded);
};

#endif // WILLCOSTMODEL_H
