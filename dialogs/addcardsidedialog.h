#ifndef ADDCARDSIDEDIALOG_H
#define ADDCARDSIDEDIALOG_H

#include <QDialog>
#include "util.h"

namespace Ui {
class AddCardSideDialog;
}

class AddCardSideDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCardSideDialog(QWidget *parent = nullptr);
    void setText(QString text);
    QString text() const;
    void setRole(DialogRole::Role role);
    DialogRole::Role role() const;
    void setSide(int side);
    int side() const;

private:
    Ui::AddCardSideDialog *ui;
    DialogRole::Role m_role;
    int m_side;
    void updateTitle();

public slots:
    void accept() override;

signals:
    void accepted_add(QString text, int side);
    void accepted_edit(QString text, int side);

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // ADDCARDSIDEDIALOG_H
