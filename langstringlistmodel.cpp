#include "langstringlistmodel.h"

LangStringListModel::LangStringListModel(const QString &title, const FLanguageModel *languageModel)
{
    this->title = title;
    if (!languageModel)
        m_languageModel = FLanguageModel::Instance();
}

int LangStringListModel::rowCount(const QModelIndex &/*parent*/) const
{
    return stringList.size();
}

QVariant LangStringListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() > stringList.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        FLanguageString langstring = stringList.at(index.row());
        //return langstring[Util::SelectedLanguage];
        QVariant v;
        v.setValue(langstring);
        return v;
    }

    return QVariant();
}

QVariant LangStringListModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    return QStringLiteral("Text").arg(section);
}

bool LangStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        stringList.replace(index.row(), value.value<FLanguageString>());
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags LangStringListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool LangStringListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i) {
        FLanguageString langstring;
        stringList.insert(row, langstring);
    }
    endInsertRows();
    return true;
}

bool LangStringListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row+count-1);
    for (int i = 0; i < count; ++i) {
        stringList.removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool LangStringListModel::addLanguageString(const FLanguageString &langstring)
{
    int lastrow = stringList.size();
    insertRow(lastrow);

    QModelIndex lastrowidx = index(lastrow);
    QVariant data;
    data.setValue(langstring);

    return setData(lastrowidx, data, Qt::EditRole);
}

const QString LangStringListModel::getTitle() const
{
    return title;
}


bool LangStringListModel::moveRows(const QModelIndex &/*sourceParent*/, int sourceRow, int count, const QModelIndex &/*destinationParent*/, int destinationChild)
{
    if (sourceRow >= stringList.size() || destinationChild >= stringList.size() || count != 1) {
        return false;
    }
    const FLanguageString temp = stringList.at(destinationChild);
    stringList.replace(destinationChild, stringList.at(sourceRow));
    stringList.replace(sourceRow, temp);
    emit dataChanged(index(sourceRow), index(destinationChild), {Qt::EditRole});
    return true;
}
