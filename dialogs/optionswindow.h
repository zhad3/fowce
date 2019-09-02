#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QDialog>

namespace Ui {
class OptionsWindow;
}

class OptionsWindow : public QDialog
{
    Q_OBJECT

public:
    enum FontUpdateType { FontCardname, FontCardtype, FontAbilities, FontFlavor, FontStats, FontVoidCost };
    explicit OptionsWindow(QWidget *parent = nullptr);
    ~OptionsWindow();

    const QFont cardnameFont() { return m_curCardnameFont; }
    const QFont cardtypeFont() { return m_curCardtypeFont; }
    const QFont abilitiesFont() { return m_curAbilitiesFont; }
    const QFont flavorFont() { return m_curFlavorFont; }
    const QFont statsFont() { return m_curStatsFont; }
    const QFont voidCostFont() { return m_curVoidCostFont; }

public slots:
    void save();

signals:
    void fontChanged(FontUpdateType type, const QFont &font);

private:
    Ui::OptionsWindow *ui;
    void readSettings();

    QFont m_curCardnameFont;
    QFont m_curCardtypeFont;
    QFont m_curAbilitiesFont;
    QFont m_curFlavorFont;
    QFont m_curStatsFont;
    QFont m_curVoidCostFont;
};

#endif // OPTIONSWINDOW_H
