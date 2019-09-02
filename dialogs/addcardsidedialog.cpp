#include "addcardsidedialog.h"
#include "ui_addcardsidedialog.h"
#include "util.h"

AddCardSideDialog::AddCardSideDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCardSideDialog), m_role(DialogRole::Add),
    m_side(0)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void AddCardSideDialog::setText(QString text)
{
    ui->input_cardside->setText(text);
}

QString AddCardSideDialog::text() const
{
    return ui->input_cardside->text();
}

void AddCardSideDialog::setRole(DialogRole::Role role)
{
    m_role = role;
    updateTitle();
}

DialogRole::Role AddCardSideDialog::role() const
{
    return m_role;
}

void AddCardSideDialog::setSide(int side)
{
    if (side >= 0 && side < MAX_CARDSIDES) {
        m_side = side;
    }
}

int AddCardSideDialog::side() const
{
    return m_side;
}

void AddCardSideDialog::accept()
{
    switch(m_role) {
    case DialogRole::Add:
        emit accepted_add(ui->input_cardside->text(), m_side);
        break;
    case DialogRole::Edit:
        emit accepted_edit(ui->input_cardside->text(), m_side);
        break;
    }
    done(Accepted);
}

void AddCardSideDialog::updateTitle()
{
    switch (m_role) {
    case DialogRole::Add:
        setWindowTitle(tr("Add %1", "Dialog Title").arg(tr("Cardside")));
        break;
    case DialogRole::Edit:
        setWindowTitle(tr("Edit %1", "Dialog Title").arg(tr("Cardside")));
        break;
    }
}


void AddCardSideDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->input_cardside->setFocus(Qt::PopupFocusReason);
}
