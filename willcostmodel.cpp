#include <QPixmap>
#include <QIcon>
#include "willcostmodel.h"
#include "util.h"

WillCostModel::WillCostModel(QObject *parent, const FAttributeModel *attributeModel)
    : QAbstractTableModel(parent), m_attributeModel(attributeModel), m_columnCount(4),
      m_totalNonGenericCost(0), m_totalGenericCost(0), m_costExceeded(false)
{
    if (m_attributeModel) {
        // Calculate rows
        int count = 0;
        QVector<FAbstractObject*>::const_iterator it;
        for (it = m_attributeModel->dataVec()->begin(); it != m_attributeModel->dataVec()->end(); ++it) {
            const FAttribute *attribute = static_cast<FAttribute*>(*it);
            costs.push_back(WillCost(attribute, nullptr, 0, attribute->isGeneric(), false));

            QVector<const FWillCharacteristic*>::const_iterator cit;
            for (cit = attribute->characteristics()->begin(); cit != attribute->characteristics()->end(); ++cit) {
                costs.push_back(WillCost(attribute, (*cit), 0, false, false));
            }

            count += 1 + attribute->characteristics()->size();
        }
        m_rowCount = count;
    } else {
        m_rowCount = 0;
    }
}

const FAttribute* WillCostModel::getAttribute(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() > m_rowCount) return nullptr;
    return costs[index.row()].attribute;
}

const WillCost* WillCostModel::getCost(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() > m_rowCount) return nullptr;
    return &costs[index.row()];
}

QVariant WillCostModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() > m_columnCount || index.row() > m_rowCount) return QVariant();
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return QString("");
        } else if (index.column() == 1) {
            QString name;
            if (costs[index.row()].characteristic) {
                name = QString("%1 (%2)").arg(costs[index.row()].attribute->name(), costs[index.row()].characteristic->name());
            } else {
                name = costs[index.row()].attribute->name();
            }
            return name;
        } else if (index.column() == 2) {
            return costs[index.row()].cost;
        } else if (index.column() == 3) {
            if (!costs[index.row()].attribute->isGeneric() || costs[index.row()].characteristic) {
                return QVariant();
            }
            if (costs[index.row()].dynamic) {
                return QObject::tr("Yes", "Is checkbox checked");
            } else {
                return QObject::tr("No", "Is checkbox checked");
            }
       } else return QVariant();
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            QPixmap test;
            if (costs[index.row()].characteristic) {
                if (costs[index.row()].characteristic->iconPath().isEmpty()) {
                    return QVariant(QIcon::fromTheme("missing"));
                } else {
                    return QVariant(QIcon::fromTheme(costs[index.row()].characteristic->iconPath()));
                }
            } else {
                if (costs[index.row()].attribute->iconPath().isEmpty()) {
                    return QVariant(QIcon::fromTheme("missing"));
                } else {
                    return QVariant(QIcon::fromTheme(costs[index.row()].attribute->iconPath()));
                }
            }
        }
    } else if (role == Qt::EditRole) {
        if (index.column() == 2) {
            return QVariant(costs[index.row()].cost);
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == 3) {
            if (!costs[index.row()].attribute->isGeneric() || costs[index.row()].characteristic) {
                return QVariant();
            }
            if (costs[index.row()].dynamic) {
                return QVariant(Qt::CheckState::Checked);
            } else {
                return QVariant(Qt::CheckState::Unchecked);
            }
        }
    }
    return QVariant();
}


QVariant WillCostModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role == Qt::DisplayRole) {
        QString name;
        switch (section) {
        default:
        case 0:
            name = "";
            break;
        case 1:
            name = QObject::tr("Attribute");
            break;
        case 2:
            name = QObject::tr("Cost");
            break;
        case 3:
            name = QObject::tr("Is Dynamic");
            break;
        }
        return name;
    }
    return QVariant();
}


Qt::ItemFlags WillCostModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.column() == 2) {
            return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
        } else if (index.column() == 3) {
            return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
        } else {
            return Qt::ItemIsEnabled;
        }
    }
    return QAbstractTableModel::flags(index);
}


bool WillCostModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (index.column() == 2 && role == Qt::EditRole) {
        int costDifference = value.toInt() - costs[index.row()].cost;
        if (costs[index.row()].generic) {
            m_totalGenericCost += costDifference;
        } else {
            m_totalNonGenericCost += costDifference;
        }

        costs[index.row()].cost = value.toInt();
        emit dataChanged(index, index);

        if (m_totalGenericCost > MAX_TOTAL_CARD_COST_GENERIC || m_totalNonGenericCost > MAX_TOTAL_CARD_COST) {
            m_costExceeded = true;
            emit costExceeded(true);
        } else if (m_costExceeded) {
            m_costExceeded = false;
            emit costExceeded(false);
        }
    } else if (index.column() == 3 && role == Qt::CheckStateRole) {
        costs[index.row()].dynamic = value.toInt() == Qt::CheckState::Checked ? true : false;
        emit dataChanged(index, index);
        // If current cost exceeds maximum dynamic cost -> update
        if (costs[index.row()].cost > MAX_CARD_COST_DYNAMIC) {
            QModelIndex idx = WillCostModel::index(index.row(), 2, index.parent());
            setData(idx, QVariant(MAX_CARD_COST_DYNAMIC), Qt::EditRole);
        }
    }
    return false;
}
