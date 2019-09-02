#ifndef LANGSTRINGLISTMODEL_H
#define LANGSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include "models/flanguagemodel.h"
#include "models/flanguagestring.h"
#include "util.h"

class LangStringListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    LangStringListModel(const QString &title = QString(), const FLanguageModel *languageModel = nullptr);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);

    bool addLanguageString(const FLanguageString &langstring);
    void setTitle(const QString title);
    const QString getTitle() const;

private:
    QList<FLanguageString> stringList;
    const FLanguageModel *m_languageModel;
    QString title;

};

#endif // LANGSTRINGLISTMODEL_H
