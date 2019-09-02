#include "addtraitdialog.h"
#include "ui_addtraitdialog.h"

#include <QCompleter>

AddTraitDialog::AddTraitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTraitDialog)
{
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QStringList qsl;
    qsl << "Apfel" << "Birne" << "Erdbeere" << "Tomate" << "Erdmännchen";

    QCompleter * completer = new QCompleter(qsl, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    ui->in_racetrait->setCompleter(completer);
}

AddTraitDialog::~AddTraitDialog()
{
    delete ui;
}
