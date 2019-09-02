#include <QSettings>

#include "optionswindow.h"
#include "ui_optionswindow.h"

OptionsWindow::OptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsWindow)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &OptionsWindow::save);

    readSettings();
}

OptionsWindow::~OptionsWindow()
{
    delete ui;
}

void OptionsWindow::save()
{
    QSettings settings;
    settings.setValue("font/cardname", ui->font_cardname->currentFont().toString());
    settings.setValue("font/abilities", ui->font_abilities->currentFont().toString());
    settings.setValue("font/cardtype", ui->font_cardtype->currentFont().toString());
    settings.setValue("font/flavor", ui->font_flavor->currentFont().toString());
    settings.setValue("font/stats", ui->font_stats->currentFont().toString());
    settings.setValue("font/voidcost", ui->font_voidcost->currentFont().toString());

    if (ui->font_cardname->currentFont() != m_curCardnameFont) {
        m_curCardnameFont = ui->font_cardname->currentFont();
        emit fontChanged(FontCardname, m_curCardnameFont);
    }

    if (ui->font_cardtype->currentFont() != m_curCardtypeFont) {
        m_curCardtypeFont = ui->font_cardtype->currentFont();
        emit fontChanged(FontCardtype, m_curCardtypeFont);
    }

    if (ui->font_abilities->currentFont() != m_curAbilitiesFont) {
        m_curAbilitiesFont = ui->font_abilities->currentFont();
        emit fontChanged(FontAbilities, m_curAbilitiesFont);
    }

    if (ui->font_flavor->currentFont() != m_curFlavorFont) {
        m_curFlavorFont = ui->font_flavor->currentFont();
        emit fontChanged(FontFlavor, m_curFlavorFont);
    }

    if (ui->font_stats->currentFont() != m_curStatsFont) {
        m_curStatsFont = ui->font_stats->currentFont();
        emit fontChanged(FontStats, m_curStatsFont);
    }

    if (ui->font_voidcost->currentFont() != m_curVoidCostFont) {
        m_curVoidCostFont = ui->font_voidcost->currentFont();
        emit fontChanged(FontVoidCost, m_curVoidCostFont);
    }
}

void OptionsWindow::readSettings()
{
    QSettings settings;
    m_curCardnameFont.fromString(settings.value("font/cardname", "Times New Roman").value<QString>());
    m_curCardtypeFont.fromString(settings.value("font/cardtype", "Times New Roman").value<QString>());
    m_curAbilitiesFont.fromString(settings.value("font/abilities", "Adobe Fangsong Std R").value<QString>());
    m_curFlavorFont.fromString(settings.value("font/flavor", "Times New Roman").value<QString>());
    m_curStatsFont.fromString(settings.value("font/stats", "Georgia").value<QString>());
    m_curVoidCostFont.fromString(settings.value("font/voidcost", "Georgia").value<QString>());

    ui->font_cardname->setCurrentFont(m_curCardnameFont);
    ui->font_cardtype->setCurrentFont(m_curCardtypeFont);
    ui->font_abilities->setCurrentFont(m_curAbilitiesFont);
    ui->font_flavor->setCurrentFont(m_curFlavorFont);
    ui->font_stats->setCurrentFont(m_curStatsFont);
    ui->font_voidcost->setCurrentFont(m_curVoidCostFont);

    emit fontChanged(FontCardname, m_curCardnameFont);
    emit fontChanged(FontCardtype, m_curCardtypeFont);
    emit fontChanged(FontAbilities, m_curAbilitiesFont);
    emit fontChanged(FontFlavor, m_curFlavorFont);
    emit fontChanged(FontStats, m_curStatsFont);
    emit fontChanged(FontVoidCost, m_curVoidCostFont);
}
