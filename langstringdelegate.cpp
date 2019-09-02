#include "models/flanguagemodel.h"
#include "langstringdelegate.h"
#include "langstringlistmodel.h"
#include "util.h"

LangStringDelegate::LangStringDelegate(QObject *parent, bool useTextEdit) :
    QStyledItemDelegate (parent),
    isModified(false), m_useTextEdit(useTextEdit)
{

}

void LangStringDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    FLanguageString langstring = index.data().value<FLanguageString>();
    QPen defaultPen = painter->pen();

    QRect text_rect = option.rect;
    text_rect.setX(text_rect.x() + 5);
    text_rect.setWidth(text_rect.width() - 10);
    painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, langstring.text());

    QFont smallerFont = painter->font();
    smallerFont.setPointSize(smallerFont.pointSize() - 2);
    painter->setFont(smallerFont);

    QString filled_out_text = QString::number(langstring.getFilledOut()) + QString("/") + QString::number(FLanguageModel::Instance()->dataCount());
    QRect text_bounding = painter->boundingRect(text_rect, Qt::AlignRight | Qt::AlignVCenter, filled_out_text);
    text_bounding.setX(text_bounding.x() - 3);
    text_bounding.setWidth(text_bounding.width() + 3);
    text_bounding.setY(text_bounding.y() - 1);
    text_bounding.setHeight(text_bounding.height() + 1);

    QPainterPath roundedRectPath;
    roundedRectPath.addRoundedRect(text_bounding, 2, 2);

    QBrush background_brush;
    background_brush.setStyle(Qt::SolidPattern);
    if (langstring.text().isEmpty()) {
        background_brush.setColor(Util::Red);
    } else if (langstring.getFilledOut() < FLanguageModel::Instance()->dataCount()) {
        background_brush.setColor(Util::Blue);
    } else {
        background_brush.setColor(Util::Green);
    }

    painter->fillPath(roundedRectPath, background_brush);
    painter->drawText(text_rect, Qt::AlignRight | Qt::AlignVCenter, filled_out_text);

    painter->restore();
}

QWidget *LangStringDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    QWidget *dummyEditor = new QWidget(parent);

    //Util::LanguageString langstring = index.data(Qt::EditRole).value<Util::LanguageString>();
    FLanguageString langstring = index.data(Qt::EditRole).value<FLanguageString>();
    const LangStringListModel *model = static_cast<const LangStringListModel*>(index.model());
    LanguageStringEditDialog *editorDialog = new LanguageStringEditDialog(dummyEditor, nullptr, m_useTextEdit);
    editorDialog->setLanguageModel(FLanguageModel::Instance());
    editorDialog->setTitle(model->getTitle());
    editorDialog->setData(langstring);

    QObject::connect(editorDialog, &LanguageStringEditDialog::accepted_with_data, this, &LangStringDelegate::editDialogAccepted);
    QObject::connect(editorDialog, &LanguageStringEditDialog::rejected, this, &LangStringDelegate::editDialogRejected);
    editorDialog->show();
    return dummyEditor;
}

void LangStringDelegate::setEditorData(QWidget* /*editor*/, const QModelIndex &/*index*/) const
{
}

void LangStringDelegate::setModelData(QWidget* /*editor*/, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (isModified) {
        QVariant v;
        v.setValue(edited_langstring);
        model->setData(index, v);
    }
}

bool LangStringDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    isModified = false;
    modelindex = index;
    this->model = model;
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void LangStringDelegate::setEditDialog(LanguageStringEditDialog *editDialog)
{
    m_editDialog = editDialog;
}

LanguageStringEditDialog *LangStringDelegate::editDialog()
{
    return m_editDialog;
}

void LangStringDelegate::editDialogAccepted(const FLanguageString &langstring)
{
    QObject *obj = sender(); // MultiLangTextEditDialog
    QWidget *dummyEditor = static_cast<QWidget*>(obj->parent());
    if (langstring.getFilledOut() == 0) {
        isModified = false;
    } else {
        isModified = true;
        edited_langstring = langstring;
    }

    setModelData(dummyEditor, model, modelindex);
    emit closeEditor(dummyEditor);
}

void LangStringDelegate::editDialogRejected()
{
    QObject *obj = sender(); // MultiLangLineEditDialog
    QWidget *dummyEditor = static_cast<QWidget*>(obj->parent());
    emit closeEditor(dummyEditor);
}
