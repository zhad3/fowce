#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QItemSelection>
#include "util.h"
#include "langstringlistmodel.h"
#include "willcostmodel.h"
#include "dialogs/languagestringeditdialog.h"
#include "dialogs/multilangtexteditdialog.h"
#include "dialogs/addcardsidedialog.h"
#include "dialogs/optionswindow.h"
#include "widgets/cardsidepushbutton.h"
#include "widgets/fcombobox.h"
#include "models/flanguagemodel.h"
#include "models/fwillcharacteristicmodel.h"
#include "models/fattributemodel.h"
#include "models/fcardtypemodel.h"
#include "models/fraritymodel.h"
#include "card.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void selectCardSide(int side);
    QString getCardSideName(int side);
    //~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void cardTypeChanged(int index);
    void generalCardTypeChanged(int index);
    void rarityChanged(int index);
    void switchCard(int index = 0);

private slots:
    void on_btn_trait_add_clicked();
    void on_btn_trait_edit_clicked();
    void on_btn_trait_remove_clicked();
    void on_btn_trait_moveup_clicked();
    void on_btn_trait_movedown_clicked();
    void dialog_trait_accepted(const FLanguageString langstring);
    void dialog_trait_edit_accepted(const FLanguageString langstring);
    void listView_trait_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_btn_cardname_edit_clicked();
    void dialog_cardname_accepted(const FLanguageString langstring);

    void on_btn_addside_clicked();
    void dialog_addcardside_accepted(QString text, int side);
    void dialog_editcardside_accepted(QString text, int side);
    void clickedCardSide(bool checked = false);

    void toggleCostError(bool exceeded);

    void on_btn_flavortext_edit_clicked();
    void dialog_flavortext_accepted(const FLanguageString langstring);

    void on_btn_add_cardtype_clicked();
    void deleteCardType();

    void on_actionOptions_triggered();

    void on_actionExport_as_PNG_triggered();

    void on_btn_ability_text_add_clicked();
    void on_btn_ability_text_edit_clicked();
    void on_btn_ability_text_remove_clicked();
    void dialog_ability_accepted(const FLanguageString langstring);
    void dialog_ability_edit_accepted(const FLanguageString langstring);
    void listView_ability_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Ui::MainWindow *ui;
    FLanguageModel *m_languageModel;
    FWillCharacteristicModel *m_characteristicModel;
    FAttributeModel *m_attributeModel;
    FGeneralCardTypeModel *m_generalCardTypeModel;
    FCardTypeModel *m_cardTypeModel;
    FRarityModel *m_rarityModel;


    AddCardSideDialog *addCardside_dialog;
    LanguageStringEditDialog *m_lineEditDialog;
    LanguageStringEditDialog *m_textEditDialog;
    OptionsWindow *m_optionsWindow;

    int m_selectedCardSide;
    QMap<int, CardSidePushButton*> m_cardSideButtons;

    int m_activeCardTypes;
    QVector<FComboBox*> m_cardTypeCBs;
    QVector<FComboBox*> m_generalCardTypeCBs;
    QVector<QPushButton*> m_deleteCardTypeBTNs;

    QVector<Card*> m_cards;
    Card* m_selectedCard;

    void readSettings();

    void editCardSideName();
    void deleteCardSide();
    void updateTraitButtonStates();
    void updateAbilityButtonStates();
    void updateTypeOptions();
    void updateGeneralCardTypeState(int index = 0, bool blockSignals = false);
    void enableCardTypes(int index, bool enabled = true, FComboBox *from = nullptr);
    void loadCardTypes(const Card *fromCard);
    void buildCardTypeComboBox(int indexCardType = 0, int indexGeneralCardType = 0, bool blockSignal = false);
    int getFirstUnusedCardType();
};

#endif // MAINWINDOW_H
