#ifndef MULTILANGLINEEDITDIALOG_H
#define MULTILANGLINEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "util.h"
#include "models/flanguagemodel.h"
#include "widgets/multilanglineedit.h"

namespace Ui {
class MultiLangLineEditDialog;
}

class LanguageStringEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LanguageStringEditDialog(QWidget *parent = nullptr, const FLanguageModel *languageModel = nullptr, bool useTextEdit = false);
    ~LanguageStringEditDialog();
    void setTitle(const QString &title);
    void setData(const FLanguageString langstring);
    void setLanguageModel(const FLanguageModel *languageModel);
    void init();
    void clearInputs();
    void refresh(const QString title);

protected:
    void showEvent(QShowEvent *event);

public slots:
    void accept();
    void rebuildUI(const FLanguage *language);

private:
    Ui::MultiLangLineEditDialog *ui;

    const FLanguageModel *m_languageModel;
    QVector<QWidget*> m_inputs;

    bool isInit;
    bool useTextEdit;

signals:
    void accepted_with_data(const FLanguageString &langstring);
};

#endif // MULTILANGLINEEDITDIALOG_H
