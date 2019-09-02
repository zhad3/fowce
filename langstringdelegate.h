#ifndef LANGSTRINGDELEGATE_H
#define LANGSTRINGDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "dialogs/languagestringeditdialog.h"

class LangStringDelegate : public QStyledItemDelegate
{
public:
    LangStringDelegate(QObject *parent = nullptr, bool useTextEdit = false);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

    void setEditDialog(LanguageStringEditDialog *editDialog);
    LanguageStringEditDialog *editDialog();
private:
    LanguageStringEditDialog *m_editDialog;
    FLanguageString edited_langstring;
    bool isModified;
    QModelIndex modelindex;
    QAbstractItemModel *model;

    bool m_useTextEdit;

private slots:
    void editDialogAccepted(const FLanguageString &langstring);
    void editDialogRejected();

};

#endif // LANGSTRINGDELEGATE_H
