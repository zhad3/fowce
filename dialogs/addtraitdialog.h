#ifndef ADDTRAITDIALOG_H
#define ADDTRAITDIALOG_H

#include <QDialog>

namespace Ui {
class AddTraitDialog;
}

class AddTraitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTraitDialog(QWidget *parent = nullptr);
    ~AddTraitDialog();

private:
    Ui::AddTraitDialog *ui;
};

#endif // ADDTRAITDIALOG_H
