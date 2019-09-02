#include <QSettings>
#include <QStandardItemModel>
#include <QLocale>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "langstringdelegate.h"
#include "willcostdelegate.h"
#include "card.h"

#include "models/flanguagemodel.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_selectedCardSide(0), m_activeCardTypes(1)
{
    QIcon::setThemeName("FOWCE");

    m_languageModel = new FLanguageModel(this);
    FLanguageModel::SetInstance(m_languageModel);

    // Read language settings. Do this before initializing the rest of the UI
    QSettings settings;
    QLocale locale = QLocale(settings.value("main/locale", "en_US").toString());
    QLocale::setDefault(locale);

    QString language = settings.value("main/selected_language").toString();
    m_languageModel->selectLanguage(language);
    // ==

    m_characteristicModel = new FWillCharacteristicModel(this);
    FWillCharacteristicModel::SetInstance(m_characteristicModel);

    m_attributeModel = new FAttributeModel(this);
    FAttributeModel::SetInstance(m_attributeModel);

    m_generalCardTypeModel = new FGeneralCardTypeModel(this);
    FGeneralCardTypeModel::SetInstance(m_generalCardTypeModel);

    m_cardTypeModel = new FCardTypeModel(this);
    FCardTypeModel::SetInstance(m_cardTypeModel);

    m_rarityModel = new FRarityModel(this);
    FRarityModel::SetInstance(m_rarityModel);

    Card *mainCard = new Card(this, m_attributeModel, m_languageModel);
    mainCard->addCardType(0, m_cardTypeModel->get(0));
    mainCard->addGeneralCardType(0, m_generalCardTypeModel->get(0));
    m_cards.push_back(mainCard);
    m_selectedCard = mainCard;

    ui->setupUi(this);

    m_cardTypeCBs.push_back(ui->cb_cardtype);
    m_generalCardTypeCBs.push_back(ui->cb_general_cardtype);

    ui->widget_cost_error_group->hide();
    ui->label_cost_error->setText(QObject::tr("Exceeded maximum amount of cost.\n(Max for non-generic: %1, for generic: %2)").arg(QString::number(MAX_TOTAL_CARD_COST), QString::number(MAX_TOTAL_CARD_COST_GENERIC)));

    m_lineEditDialog = new LanguageStringEditDialog(this);
    m_textEditDialog = new LanguageStringEditDialog(this, nullptr, true);
    addCardside_dialog = new AddCardSideDialog(this);
    m_optionsWindow = new OptionsWindow(this);

    QObject::connect(m_optionsWindow, &OptionsWindow::fontChanged, ui->widget_cardpreview, &CardPreviewWidget::setFont);

    m_lineEditDialog->setLanguageModel(m_languageModel);
    m_textEditDialog->setLanguageModel(m_languageModel);
    QObject::connect(m_languageModel, &FLanguageModel::languageSelected, m_lineEditDialog, &LanguageStringEditDialog::rebuildUI);
    QObject::connect(m_languageModel, &FLanguageModel::languageSelected, m_textEditDialog, &LanguageStringEditDialog::rebuildUI);

    LangStringDelegate *langstringDelegate = new LangStringDelegate(ui->listView_trait);
    langstringDelegate->setEditDialog(m_lineEditDialog);
    ui->listView_trait->setItemDelegate(new LangStringDelegate);
    ui->listView_trait->setModel(m_selectedCard->traitModel());
    QObject::connect(ui->listView_trait->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::listView_trait_selectionChanged);

    LangStringDelegate *langstringDelegate_abilities = new LangStringDelegate(ui->listView_abilities, true);
    langstringDelegate_abilities->setEditDialog(m_textEditDialog);
    ui->listView_abilities->setItemDelegate(new LangStringDelegate(nullptr, true));
    ui->listView_abilities->setModel(m_selectedCard->abilityTextModel());
    QObject::connect(ui->listView_abilities->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::listView_ability_selectionChanged);

    WillCostDelegate *costDelegate = new WillCostDelegate(this);
    ui->table_willcost->verticalHeader()->setDefaultSectionSize(24);
    ui->table_willcost->setIconSize(QSize(16,16));
    ui->table_willcost->setItemDelegate(costDelegate);
    ui->table_willcost->setModel(m_selectedCard->willCostModel());
    ui->table_willcost->setColumnWidth(0, 20);
    QObject::connect(m_selectedCard->willCostModel(), &WillCostModel::costExceeded, this, &MainWindow::toggleCostError);

    ui->btn_frontside->setCard(mainCard);
    QObject::connect(ui->btn_frontside, &CardSidePushButton::clicked, this, &MainWindow::clickedCardSide);

    QObject::connect(ui->checkboxgroup_attributes, &AttributeCheckBoxGroup::attributeChecked, m_selectedCard, &Card::addAttribute);
    QObject::connect(ui->checkboxgroup_attributes, &AttributeCheckBoxGroup::attributeUnchecked, m_selectedCard, &Card::removeAttribute);

    readSettings();

    /* Card type combobox */
    const QVector<FAbstractObject*> *cardTypeData = m_cardTypeModel->dataVec();
    QVector<FAbstractObject*>::const_iterator cardTypeIter;
    for (cardTypeIter = cardTypeData->constBegin(); cardTypeIter != cardTypeData->constEnd(); ++cardTypeIter) {
        const FCardType *cardType = static_cast<const FCardType*>(*cardTypeIter);
        ui->cb_cardtype->addItem(cardType->name(), QVariant(cardType->id()));
    }

    /* General card type combobox */
    //ui->cb_general_cardtype->addItem("--", -1);

    const QVector<FAbstractObject*> *generalCardTypeData = m_generalCardTypeModel->dataVec();
    QVector<FAbstractObject*>::const_iterator gctIter;
    for (gctIter = generalCardTypeData->constBegin(); gctIter != generalCardTypeData->constEnd(); ++gctIter) {
        const FGeneralCardType *gct = static_cast<const FGeneralCardType*>(*gctIter);
        ui->cb_general_cardtype->addItem(gct->name(), QVariant(gct->id()));
    }

    QObject::connect(ui->cb_cardtype, QOverload<int>::of(&FComboBox::currentIndexChanged), this, &MainWindow::cardTypeChanged);
    QObject::connect(ui->cb_general_cardtype, QOverload<int>::of(&FComboBox::currentIndexChanged), this, &MainWindow::generalCardTypeChanged);

    // Update general card type selection
    cardTypeChanged(0);
    generalCardTypeChanged(0);

    /* Rarity combobox */
    const QVector<FAbstractObject*> *rarityData = m_rarityModel->dataVec();
    QVector<FAbstractObject*>::const_iterator rarityIter;
    for (rarityIter = rarityData->constBegin(); rarityIter < rarityData->constEnd(); ++rarityIter) {
        const FRarity *rarity = static_cast<const FRarity*>(*rarityIter);
        ui->cb_rarity->addItem(rarity->name(), QVariant(rarity->id()));
    }
    rarityChanged(0);
    QObject::connect(ui->cb_rarity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::rarityChanged);

    /* Card sides */
    m_cardSideButtons.insert(0, ui->btn_frontside);

    QObject::connect(addCardside_dialog, &AddCardSideDialog::accepted_add, this, &MainWindow::dialog_addcardside_accepted);
    QObject::connect(addCardside_dialog, &AddCardSideDialog::accepted_edit, this, &MainWindow::dialog_editcardside_accepted);

    // View Editors
    ui->menuView->addAction(ui->dockWidget_basic->toggleViewAction());
    ui->menuView->addAction(ui->dockWidget_text->toggleViewAction());
    ui->menuView->addAction(ui->dockWidget_art->toggleViewAction());
    ui->menuView->addAction(ui->dockWidget_sound->toggleViewAction());
    ui->menuView->addAction(ui->dockWidget_ability->toggleViewAction());

    // Art section
    QObject::connect(ui->check_show_cost, &QCheckBox::toggled, m_selectedCard, &Card::setShowCost);
    QObject::connect(ui->check_show_stats, &QCheckBox::toggled, m_selectedCard, &Card::setShowStats);
    QObject::connect(ui->check_small_text_box, &QCheckBox::toggled, m_selectedCard, &Card::setShowSmallTextBox);
    QObject::connect(ui->check_show_border, &QCheckBox::toggled, m_selectedCard, &Card::setShowBorder);
    QObject::connect(ui->check_show_text_box, &QCheckBox::toggled, m_selectedCard, &Card::setShowTextBox);
    QObject::connect(ui->check_show_quickcast, &QCheckBox::toggled, m_selectedCard, &Card::setShowQuickcast);

    ui->widget_cardpreview->addCard(mainCard);
}

void MainWindow::selectCardSide(int side)
{
    if (m_selectedCardSide == side) {
        QPushButton *last_selected = m_cardSideButtons.value(m_selectedCardSide);
        last_selected->setChecked(true);
        return;
    }
    if (side >= 0 && side < MAX_CARDSIDES) {
        QPushButton *last_selected = m_cardSideButtons.value(m_selectedCardSide);
        bool found = false;
        QMap<int, CardSidePushButton*>::iterator i;
        for (i = m_cardSideButtons.begin(); i != m_cardSideButtons.end(); ++i) {
            if (i.key() == side) {
                found = true;
                m_selectedCardSide = side;
                i.value()->setChecked(true);
                if (i.value()->card()) {
                    switchCard(m_cards.indexOf(i.value()->card()));
                }
            } else {
                i.value()->setChecked(false);
            }
        }
        if (!found) {
            last_selected->setChecked(true);
        }
    }
}

QString MainWindow::getCardSideName(int side)
{
    if (side >= 0 && side < MAX_CARDSIDES && m_cardSideButtons.contains(side)) {
        return m_cardSideButtons.value(side)->text();
    }
    return QString();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("main/selected_language", QVariant::fromValue(m_languageModel->selectedLanguage()->countryCode()));
    settings.setValue("ui/geometry", saveGeometry());
    settings.setValue("ui/state", saveState());
    settings.setValue("ui/geometry_options", m_optionsWindow->saveGeometry());
    QMainWindow::closeEvent(event);
}

void MainWindow::cardTypeChanged(int index)
{
    FComboBox *cb = static_cast<FComboBox*>(QObject::sender());
    const FCardType *cardType;
    int cb_index;
    int old_index;

    if (cb) {
        cardType = m_cardTypeModel->get(cb->itemData(index).value<int>());
        cb_index = m_cardTypeCBs.indexOf(cb);
        old_index = cb->previousIndex();
    } else {
        cardType = m_cardTypeModel->get(ui->cb_cardtype->itemData(index).value<int>());
        cb_index = 0;
        old_index = -1;
        cb = ui->cb_cardtype;
    }
    if (!cardType) return;

    //FComboBox *cb_gct = m_generalCardTypeCBs[cb_index];

    // Toggle General card type combobox
    updateGeneralCardTypeState(cb_index);
//    const QVector<const FGeneralCardType*> *gcTypes = cardType->generalCardTypes();
//    if (gcTypes && gcTypes->size() > 0) {
//        QStandardItemModel * cbModel = qobject_cast<QStandardItemModel*>(cb_gct->model());
//        if (cbModel) {
//            const QVector<const FGeneralCardType*> *gcTypes = cardType->generalCardTypes();
//            // Starting off 1, skipping the default first entry which should always be enabled
//            for (int i = 1; i < cbModel->rowCount(); ++i) {
//                QStandardItem *cbItem = cbModel->item(i);
//                if (!cbItem) continue;
//                int cbData = cbItem->data(Qt::UserRole).toInt();
//                if (m_generalCardTypeModel->get(cbData) && gcTypes->indexOf(m_generalCardTypeModel->get(cbData)) != -1) {
//                    cbItem->setFlags(cbItem->flags() | Qt::ItemFlag::ItemIsEnabled);
//                } else {
//                    cbItem->setFlags(cbItem->flags() & ~Qt::ItemFlag::ItemIsEnabled);
//                    if (cb_gct->currentIndex() == i) {
//                        cb_gct->setCurrentIndex(0);
//                    }
//                }
//            }
//            cb_gct->setEnabled(true);
//        }
//    } else {
//        cb_gct->setEnabled(false);
//        cb_gct->setCurrentIndex(0);
//    }

    // Disable/Enable option for other comboboxes
    QVector<FComboBox*> cb_iter_data = m_cardTypeCBs;
    QVector<FComboBox*>::iterator cb_iter = cb_iter_data.begin();
    for (; cb_iter < cb_iter_data.end(); ++cb_iter) {
        if ((*cb_iter) == cb) continue;
        QStandardItemModel *cb_model = qobject_cast<QStandardItemModel*>((*cb_iter)->model());
        for (int i = 0; i < cb_model->rowCount(); ++i) {
            QStandardItem *cb_item = cb_model->item(i);
            if (!cb_item) continue;
            if (i == index) {
                cb_item->setEnabled(false);
            } else if (i == old_index) {
                cb_item->setEnabled(true);
            }
        }
    }

    m_selectedCard->replaceCardType(cb_index, cardType);

    updateTypeOptions();
}

void MainWindow::generalCardTypeChanged(int index)
{
    FComboBox *cb = static_cast<FComboBox*>(QObject::sender());
    const FGeneralCardType *generalCardType;
    int cb_index;
    int old_index;

    if (cb) {
        generalCardType = m_generalCardTypeModel->get(cb->itemData(index).value<int>());
        cb_index = m_generalCardTypeCBs.indexOf(cb);
        old_index = cb->previousIndex();
    } else {
        generalCardType = m_generalCardTypeModel->get(ui->cb_cardtype->itemData(index).value<int>());
        cb_index = 0;
        old_index = -1;
        cb = ui->cb_general_cardtype;
    }
    if (!generalCardType) return;

    m_selectedCard->replaceGeneralCardType(cb_index, generalCardType);
}

void MainWindow::rarityChanged(int index)
{
    //QComboBox *cb = static_cast<QComboBox*>(QObject::sender());
    const FRarity *rarity = static_cast<const FRarity*>(m_rarityModel->get(index));
    if (rarity) {
        m_selectedCard->setRarity(rarity);
    }
}

void MainWindow::switchCard(int index)
{
    if (index < 0 || index >= m_cards.size()) return;
    Card *newCard = m_cards[index];
    if (newCard == m_selectedCard) return;

    ui->listView_trait->setModel(newCard->traitModel());
    ui->listView_abilities->setModel(newCard->abilityTextModel());
    ui->table_willcost->setModel(newCard->willCostModel());

    updateTraitButtonStates();
    updateAbilityButtonStates();

    QObject::disconnect(m_selectedCard->willCostModel(), &WillCostModel::costExceeded, this, &MainWindow::toggleCostError);
    QObject::connect(newCard->willCostModel(), &WillCostModel::costExceeded, this, &MainWindow::toggleCostError);

    QObject::disconnect(ui->checkboxgroup_attributes, &AttributeCheckBoxGroup::attributeChecked, m_selectedCard, &Card::addAttribute);
    QObject::disconnect(ui->checkboxgroup_attributes, &AttributeCheckBoxGroup::attributeUnchecked, m_selectedCard, &Card::removeAttribute);

    QObject::disconnect(ui->check_show_cost, &QCheckBox::toggled, m_selectedCard, &Card::setShowCost);
    QObject::disconnect(ui->check_show_stats, &QCheckBox::toggled, m_selectedCard, &Card::setShowStats);
    QObject::disconnect(ui->check_small_text_box, &QCheckBox::toggled, m_selectedCard, &Card::setShowSmallTextBox);
    QObject::disconnect(ui->check_show_border, &QCheckBox::toggled, m_selectedCard, &Card::setShowBorder);
    QObject::disconnect(ui->check_show_text_box, &QCheckBox::toggled, m_selectedCard, &Card::setShowTextBox);

    loadCardTypes(newCard);

    ui->check_show_cost->setChecked(newCard->showCost());
    ui->check_show_stats->setChecked(newCard->showStats());
    ui->check_show_border->setChecked(newCard->showBorder());
    ui->check_show_text_box->setChecked(newCard->showTextBox());
    ui->check_small_text_box->setChecked(newCard->showSmallTextBox());

    QObject::connect(ui->check_show_cost, &QCheckBox::toggled, newCard, &Card::setShowCost);
    QObject::connect(ui->check_show_stats, &QCheckBox::toggled, newCard, &Card::setShowStats);
    QObject::connect(ui->check_small_text_box, &QCheckBox::toggled, newCard, &Card::setShowSmallTextBox);
    QObject::connect(ui->check_show_border, &QCheckBox::toggled, newCard, &Card::setShowBorder);
    QObject::connect(ui->check_show_text_box, &QCheckBox::toggled, newCard, &Card::setShowTextBox);

    ui->input_cardname->setLanguageString(newCard->cardName());
    ui->input_flavortext->setLanguageString(newCard->flavorText());

    QSignalBlocker b2(ui->cb_rarity);
    ui->cb_rarity->setCurrentIndex(newCard->rarity()->id());

    QSignalBlocker b3(ui->checkboxgroup_attributes);
    ui->checkboxgroup_attributes->setAttributes(newCard->attributes());

    QObject::connect(ui->checkboxgroup_attributes, &AttributeCheckBoxGroup::attributeChecked, newCard, &Card::addAttribute);
    QObject::connect(ui->checkboxgroup_attributes, &AttributeCheckBoxGroup::attributeUnchecked, newCard, &Card::removeAttribute);

    m_selectedCard = newCard;
}

void MainWindow::readSettings()
{
    QSettings settings;

    /* Geometry */
    restoreGeometry(settings.value("ui/geometry").toByteArray());
    restoreState(settings.value("ui/state").toByteArray());
    m_optionsWindow->restoreGeometry(settings.value("ui/geometry_option").toByteArray());
}

void MainWindow::editCardSideName()
{
    CardSidePushButton *btn = static_cast<CardSidePushButton*>(QObject::sender());
    addCardside_dialog->setRole(DialogRole::Edit);
    addCardside_dialog->setText(btn->text());
    addCardside_dialog->setSide(btn->side());
    QObject::connect(addCardside_dialog, &AddCardSideDialog::accepted_edit, this, &MainWindow::dialog_editcardside_accepted);
    addCardside_dialog->show();
}

void MainWindow::deleteCardSide()
{
    CardSidePushButton *btn = static_cast<CardSidePushButton*>(QObject::sender());
    if (btn->card() == m_selectedCard) {
        // Select default front side. Frontside should not be deletable!
        selectCardSide(0);
    }
    m_cardSideButtons.remove(btn->side());
    m_cards.removeOne(btn->card());
    btn->deleteLater();
    if (m_cardSideButtons.size() < MAX_CARDSIDES) {
        ui->btn_addside->setEnabled(true);
    }
}

void MainWindow::updateTraitButtonStates()
{
    const QModelIndex selected = ui->listView_trait->currentIndex();
    if (!selected.isValid()) {
        // Deselected
        ui->btn_trait_edit->setEnabled(false);
        ui->btn_trait_remove->setEnabled(false);
        ui->btn_trait_moveup->setEnabled(false);
        ui->btn_trait_movedown->setEnabled(false);
    } else if (ui->listView_trait->model()->rowCount(QModelIndex()) > 1){
        // Assuming single selection
        if (selected == ui->listView_trait->model()->index(0, 0)) {
            // First item
            ui->btn_trait_moveup->setEnabled(false);
            ui->btn_trait_movedown->setEnabled(true);
        } else if (selected == ui->listView_trait->model()->index(ui->listView_trait->model()->rowCount(QModelIndex()) - 1, 0)) {
            // Last item
            ui->btn_trait_moveup->setEnabled(true);
            ui->btn_trait_movedown->setEnabled(false);
        } else {
            ui->btn_trait_moveup->setEnabled(true);
            ui->btn_trait_movedown->setEnabled(true);
        }
        ui->btn_trait_edit->setEnabled(true);
        ui->btn_trait_remove->setEnabled(true);
    } else {
        ui->btn_trait_edit->setEnabled(true);
        ui->btn_trait_remove->setEnabled(true);
        ui->btn_trait_moveup->setEnabled(false);
        ui->btn_trait_movedown->setEnabled(false);
    }
}

void MainWindow::updateAbilityButtonStates()
{
    const QModelIndex selected = ui->listView_abilities->currentIndex();
    if (!selected.isValid()) {
        // Deselected
        ui->btn_ability_text_edit->setEnabled(false);
        ui->btn_ability_text_remove->setEnabled(false);
        //ui->btn_ability_text_moveup->setEnabled(false);
        //ui->btn_ability_text_movedown->setEnabled(false);
    } else if (ui->listView_abilities->model()->rowCount(QModelIndex()) > 1){
        // Assuming single selection
        if (selected == ui->listView_abilities->model()->index(0, 0)) {
            // First item
            //ui->btn_ability_text_moveup->setEnabled(false);
            //ui->btn_ability_text_movedown->setEnabled(true);
        } else if (selected == ui->listView_abilities->model()->index(ui->listView_abilities->model()->rowCount(QModelIndex()) - 1, 0)) {
            // Last item
            //ui->btn_ability_text_moveup->setEnabled(true);
            //ui->btn_ability_text_movedown->setEnabled(false);
        } else {
            //ui->btn_ability_text_moveup->setEnabled(true);
            //ui->btn_ability_text_movedown->setEnabled(true);
        }
        ui->btn_ability_text_edit->setEnabled(true);
        ui->btn_ability_text_remove->setEnabled(true);
    } else {
        ui->btn_ability_text_edit->setEnabled(true);
        ui->btn_ability_text_remove->setEnabled(true);
        //ui->btn_trait_moveup->setEnabled(false);
        //ui->btn_trait_movedown->setEnabled(false);
    }
}

void MainWindow::updateTypeOptions()
{
    bool hasDivinity = false;
    bool canFight = false;
    bool hasCost = false;

    // See if any of our selected types can enable the settings
    for (int i = 0; i < m_activeCardTypes; ++i) {
        const FCardType *ct = m_cardTypeModel->get(m_cardTypeCBs[i]->currentIndex());
        if (!ct) continue;
        hasDivinity |= ct->hasDivinity();
        canFight |= ct->canFight();
        hasCost |= ct->hasCost();
    }

    /* Toggle divinity input */
    if (hasDivinity) {
        ui->label_divinity->setEnabled(true);
        ui->input_divinity->setEnabled(true);
    } else {
        ui->label_divinity->setEnabled(false);
        ui->input_divinity->setEnabled(false);
        ui->input_divinity->setValue(0);
    }

    /* Toggle atk/def input */
    if (canFight) {
        ui->label_atk->setEnabled(true);
        ui->input_atk->setEnabled(true);
        ui->label_def->setEnabled(true);
        ui->input_def->setEnabled(true);
        ui->check_cannotfight->setEnabled(true);
        ui->check_dynamic_atk->setEnabled(true);
        ui->check_dynamic_def->setEnabled(true);
    } else {
        ui->label_atk->setEnabled(false);
        ui->input_atk->setEnabled(false);
        ui->label_def->setEnabled(false);
        ui->input_def->setEnabled(false);
        ui->check_cannotfight->setEnabled(false);
        ui->check_cannotfight->setCheckState(Qt::CheckState::Unchecked);
        ui->check_dynamic_atk->setEnabled(false);
        ui->check_dynamic_def->setEnabled(false);
    }

    ui->check_show_cost->setChecked(hasCost);
    ui->check_show_stats->setChecked(canFight || hasDivinity);
}

// ZZZ TODO: Some undefined behavior when switching card sides. Does not switch correctly sometimes
//
void MainWindow::updateGeneralCardTypeState(int index, bool blockSignals)
{
    FComboBox *cb = m_cardTypeCBs[index];
    FComboBox *cb_gct = m_generalCardTypeCBs[index];

    if (blockSignals) {
        QSignalBlocker b0(cb);
        QSignalBlocker b1(cb_gct);
    }

    const FCardType *cardType = m_cardTypeModel->get(cb->currentIndex());
    if (!cardType) return;

    // Toggle General card type combobox
    const QVector<const FGeneralCardType*> *gcTypes = cardType->generalCardTypes();
    if (gcTypes && gcTypes->size() > 0) {
        QStandardItemModel * cbModel = qobject_cast<QStandardItemModel*>(cb_gct->model());
        if (cbModel) {
            const QVector<const FGeneralCardType*> *gcTypes = cardType->generalCardTypes();
            // Starting off 1, skipping the default first entry which should always be enabled
            for (int i = 1; i < cbModel->rowCount(); ++i) {
                QStandardItem *cbItem = cbModel->item(i);
                if (!cbItem) continue;
                int cbData = cbItem->data(Qt::UserRole).toInt();
                if (m_generalCardTypeModel->get(cbData) && gcTypes->indexOf(m_generalCardTypeModel->get(cbData)) != -1) {
                    cbItem->setFlags(cbItem->flags() | Qt::ItemFlag::ItemIsEnabled);
                } else {
                    cbItem->setFlags(cbItem->flags() & ~Qt::ItemFlag::ItemIsEnabled);
                    /*if (cb_gct->currentIndex() == i) {
                        cb_gct->setCurrentIndex(0);
                    }*/
                }
            }
            cb_gct->setEnabled(true);
        }
    } else {
        cb_gct->setEnabled(false);
        cb_gct->setCurrentIndex(0);
    }
}

void MainWindow::enableCardTypes(int index, bool enabled, FComboBox *from)
{
    QVector<FComboBox*> cb_iter_data = m_cardTypeCBs;
    QVector<FComboBox*>::iterator cb_iter = cb_iter_data.begin();
    for (; cb_iter < cb_iter_data.end(); ++cb_iter) {
        if (from && (*cb_iter) == from) continue;
        QStandardItemModel *cb_model = qobject_cast<QStandardItemModel*>((*cb_iter)->model());
        for (int i = 0; i < cb_model->rowCount(); ++i) {
            QStandardItem *cb_item = cb_model->item(i);
            if (!cb_item) continue;
            if (!from && i == index) {
                cb_item->setEnabled(enabled);
            }
        }
    }
}

int MainWindow::getFirstUnusedCardType()
{
    QVector<FComboBox*> data = m_cardTypeCBs;
    bool available = false;
    int index = -1;
    for (int i = 0; i < m_cardTypeModel->dataCount() && !available; ++i) {
        available = true;
        QVector<FComboBox*>::const_iterator iter = data.constBegin();
        for (; iter < data.constEnd() && available; ++iter) {
            if ((*iter)->currentIndex() == i) {
                available &= false;
            }
        }
        if (available) {
            index = i;
        }
    }
    return index;
}

void MainWindow::on_btn_trait_add_clicked()
{
    m_lineEditDialog->refresh(tr("Race/Trait"));
    QObject::connect(m_lineEditDialog, &LanguageStringEditDialog::accepted_with_data, this, &MainWindow::dialog_trait_accepted);
    m_lineEditDialog->show();
}

void MainWindow::on_btn_trait_edit_clicked()
{
    QModelIndex idx = ui->listView_trait->currentIndex();
    if (!idx.isValid()) return;
    m_lineEditDialog->refresh(tr("Race/Trait"));
    FLanguageString langstring = ui->listView_trait->model()->data(idx, Qt::EditRole).value<FLanguageString>();
    m_lineEditDialog->setData(langstring);

    QObject::connect(m_lineEditDialog, &LanguageStringEditDialog::accepted_with_data, this, &MainWindow::dialog_trait_edit_accepted);

    m_lineEditDialog->show();
}

void MainWindow::on_btn_trait_remove_clicked()
{
    QModelIndex idx = ui->listView_trait->currentIndex();
    if (!idx.isValid()) return;

    ui->listView_trait->model()->removeRows(idx.row(), 1, QModelIndex());
    updateTraitButtonStates();
}

void MainWindow::dialog_trait_accepted(const FLanguageString langstring)
{
    if (langstring.getFilledOut() == 0) return; // Do nothing if the user didn't input anything

    m_selectedCard->traitModel()->addLanguageString(langstring);
    updateTraitButtonStates();
}

void MainWindow::dialog_trait_edit_accepted(const FLanguageString langstring)
{
    if (langstring.getFilledOut() == 0) return;

    QModelIndex idx = ui->listView_trait->currentIndex();
    if (!idx.isValid()) return;

    QVariant v;
    v.setValue(langstring);
    ui->listView_trait->model()->setData(idx, v, Qt::EditRole);
}

void MainWindow::listView_trait_selectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    updateTraitButtonStates();
}

void MainWindow::on_btn_cardname_edit_clicked()
{
    m_lineEditDialog->refresh(QObject::tr("Cardname"));
    m_lineEditDialog->setData(ui->input_cardname->getLanguageString());

    QObject::connect(m_lineEditDialog, &LanguageStringEditDialog::accepted_with_data, this, &MainWindow::dialog_cardname_accepted);

    m_lineEditDialog->show();
}

void MainWindow::dialog_cardname_accepted(const FLanguageString langstring)
{
    ui->input_cardname->setLanguageString(langstring);
    m_selectedCard->setCardName(langstring);
}

void MainWindow::on_btn_addside_clicked()
{
    if (m_cardSideButtons.size() >= MAX_CARDSIDES) {
        QMessageBox msg;
        msg.setText(tr("Trying to add more card sides than allowed. (Maximum: %1)").arg(MAX_CARDSIDES));
        msg.exec();
        return;
    }
    addCardside_dialog->setRole(DialogRole::Add);
    addCardside_dialog->setSide(0);
    addCardside_dialog->show();
}

void MainWindow::dialog_addcardside_accepted(QString text, int /*side*/)
{
    if (text.isEmpty()) return;
    if (m_cardSideButtons.size() >= MAX_CARDSIDES) return;

    int new_side = m_cardSideButtons.lastKey() + 1;

    CardSidePushButton *btn = new CardSidePushButton(new_side, this);
    btn->setText(text);

    QObject::connect(btn, &CardSidePushButton::menu_edit, this, &MainWindow::editCardSideName);
    QObject::connect(btn, &CardSidePushButton::menu_delete, this, &MainWindow::deleteCardSide);
    QObject::connect(btn, &CardSidePushButton::clicked, this, &MainWindow::clickedCardSide);

    int count = ui->frame_bottom_layout->count();
    addCardside_dialog->setText("");
    ui->frame_bottom_layout->insertWidget(count-1, btn);
    m_cardSideButtons.insert(new_side, btn);

    Card *card = new Card(btn, m_attributeModel, m_languageModel);
    card->addCardType(0, m_cardTypeModel->get(0));
    card->addGeneralCardType(0, m_generalCardTypeModel->get(0));
    m_cards.push_back(card);

    ui->widget_cardpreview->addCard(card);

    btn->setCard(card);

    if (m_cardSideButtons.size() >= MAX_CARDSIDES) {
        ui->btn_addside->setEnabled(false);
    }
}

void MainWindow::dialog_editcardside_accepted(QString text, int side)
{
    if (text.isEmpty()) return;

    CardSidePushButton *btn = m_cardSideButtons.value(side);
    btn->setText(text);
    addCardside_dialog->setText("");

    QObject::disconnect(addCardside_dialog, &AddCardSideDialog::accepted_add, this, &MainWindow::dialog_editcardside_accepted);
}

void MainWindow::clickedCardSide(bool /*checked*/)
{
    selectCardSide(m_cardSideButtons.key(static_cast<CardSidePushButton*>(QObject::sender())));
}

void MainWindow::toggleCostError(bool exceeded)
{
    if (exceeded && ui->widget_cost_error_group->isHidden()) {
        ui->widget_cost_error_group->show();
    } else if (!exceeded && ui->widget_cost_error_group->isVisible()) {
        ui->widget_cost_error_group->hide();
    }
}

void MainWindow::on_btn_flavortext_edit_clicked()
{
    m_textEditDialog->refresh(tr("Flavortext"));
    m_textEditDialog->setData(ui->input_flavortext->getLanguageString());

    QObject::connect(m_textEditDialog, &LanguageStringEditDialog::accepted_with_data, this, &MainWindow::dialog_flavortext_accepted);

    m_textEditDialog->show();
}

void MainWindow::dialog_flavortext_accepted(const FLanguageString langstring)
{
    ui->input_flavortext->setLanguageString(langstring);
    m_selectedCard->setFlavorText(langstring);
}

void MainWindow::on_btn_trait_moveup_clicked()
{
    const QModelIndex selected = ui->listView_trait->currentIndex();
    if (!selected.isValid() || selected.row() == 0) {
        return;
    }
    ui->listView_trait->model()->moveRows(selected.parent(), selected.row(), 1, selected.parent(), selected.row() - 1);
    ui->listView_trait->setCurrentIndex(ui->listView_trait->model()->index(selected.row() - 1, 0));
}

void MainWindow::on_btn_trait_movedown_clicked()
{
    const QModelIndex selected = ui->listView_trait->currentIndex();
    if (!selected.isValid() || selected.row() == ui->listView_trait->model()->rowCount(selected.parent())) {
        return;
    }
    ui->listView_trait->model()->moveRows(selected.parent(), selected.row(), 1, selected.parent(), selected.row() + 1);
    ui->listView_trait->setCurrentIndex(ui->listView_trait->model()->index(selected.row() + 1, 0));
}

void MainWindow::on_btn_add_cardtype_clicked()
{
    if (m_cardTypeCBs.size() >= MAX_CARD_TYPES && m_activeCardTypes >= MAX_CARD_TYPES) {
        return;
    }

    //const FCardType *selectedCardType = m_cardTypeModel->get(ui->cb_cardtype->currentData().value<int>());

    int row = 0;
    int available_index = getFirstUnusedCardType();
    if (available_index == -1) {
        QMessageBox msg;
        msg.setText(tr("Not enough options available for another card type."));
        msg.exec();
        return;
    }

    if (m_cardTypeCBs.size() < MAX_CARD_TYPES) {
        // New Card Type
//        FComboBox *cb_new = new FComboBox(this);
//        cb_new->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//        const QVector<FAbstractObject*> *cardTypeData = m_cardTypeModel->dataVec();
//        QVector<FAbstractObject*>::const_iterator cardTypeIter;

//        QVector<FComboBox*> cb_data = m_cardTypeCBs;

//        for (cardTypeIter = cardTypeData->constBegin(); cardTypeIter != cardTypeData->constEnd(); ++cardTypeIter, ++row) {
//            const FCardType *cardType = static_cast<const FCardType*>(*cardTypeIter);
//            cb_new->addItem(cardType->name(), QVariant(cardType->id()));

//            QVector<FComboBox*>::const_iterator cb_iter = cb_data.constBegin();
//            for (; cb_iter < cb_data.constEnd(); ++cb_iter) {
//                if ((*cb_iter)->currentIndex() == row) {
//                    QStandardItemModel *cb_model = static_cast<QStandardItemModel*>(cb_new->model());
//                    if (cb_model) {
//                        cb_model->item(row)->setEnabled(false);
//                    }
//                    break;
//                }
//            }
//        }
//        m_cardTypeCBs.push_back(cb_new);
        buildCardTypeComboBox();


    } else {
        // Show hidden inputs
        m_cardTypeCBs[m_activeCardTypes]->show();
        m_generalCardTypeCBs[m_activeCardTypes]->show();
        m_deleteCardTypeBTNs[m_activeCardTypes-1]->show();

        m_cardTypeCBs[m_activeCardTypes]->setCurrentIndex(available_index);
    }

    const FCardType *ct = m_cardTypeModel->get(m_cardTypeCBs[m_activeCardTypes]->currentData().value<int>());
    if (ct) {
        m_selectedCard->addCardType(m_activeCardTypes, ct);
    }

    const FGeneralCardType *gct = m_generalCardTypeModel->get(m_generalCardTypeCBs[m_activeCardTypes]->currentData().value<int>());
    if (gct) {
        m_selectedCard->addGeneralCardType(m_activeCardTypes, gct);
    }

    m_activeCardTypes++;

    if (m_cardTypeCBs.size() >= MAX_CARD_TYPES && m_activeCardTypes >= MAX_CARD_TYPES) {
        ui->btn_add_cardtype->setEnabled(false);
    }

    updateTypeOptions();
}

void MainWindow::loadCardTypes(const Card *fromCard)
{
    qDebug("== loadCardTypes ==");
    qDebug("m_activeCardTypes: %d", m_activeCardTypes);
    const QMap<int, const FCardType*> ct_data = fromCard->cardTypes();
    const QMap<int, const FGeneralCardType*> gct_data = fromCard->generalCardTypes();
    int cb_size = m_cardTypeCBs.size();

    // Set first card type
    const FCardType *ct_first = ct_data.first();
    QSignalBlocker b0(m_cardTypeCBs[0]);
    m_cardTypeCBs[0]->setCurrentIndex(ct_first->id());

    updateGeneralCardTypeState(0, true);

    const FGeneralCardType *gct_first = gct_data.first();
    QSignalBlocker b1(m_generalCardTypeCBs[0]);
    m_generalCardTypeCBs[0]->setCurrentIndex(gct_first->id());

    // Load remaining card types
    int diff = m_activeCardTypes - ct_data.size();
    qDebug("diff: %d", diff);
    if (diff > 0) {
        // Hide additional combo boxes
        for (int i = m_activeCardTypes - 1; i > m_activeCardTypes - 1 - diff; --i) {
            qDebug("Hiding ComboBoxes with index: %d", i);
            m_cardTypeCBs[i]->hide();
            m_generalCardTypeCBs[i]->hide();
            m_deleteCardTypeBTNs[i-1]->hide();
            enableCardTypes(m_cardTypeCBs[i]->currentIndex(), true);
        }
    } else if (diff < 0) {
        // Add or show additional combo boxes
        for (int i = m_activeCardTypes; i < ct_data.size(); ++i) {
            const FCardType *ct = ct_data[i];
            const FGeneralCardType *gct = gct_data[i];
            if (i <= cb_size - 1) {
                // Show combo boxes
                qDebug("Showing ComboBoxes with index: %d", i);
                m_cardTypeCBs[i]->show();
                m_generalCardTypeCBs[i]->show();
                m_deleteCardTypeBTNs[i-1]->show();
                QSignalBlocker b2(m_cardTypeCBs[i]);
                m_cardTypeCBs[i]->setCurrentIndex(ct->id());
            } else {
                // Add combo boxes
                qDebug("Adding new ComboBox");
                buildCardTypeComboBox(ct->id(), gct->id(), true);
            }
            updateGeneralCardTypeState(i, true);

            QSignalBlocker b3(m_generalCardTypeCBs[i]);
            m_generalCardTypeCBs[i]->setCurrentIndex(gct->id());
        }
    } else {
        // Equal number of types, just change the index
        for (int i = 1; i < m_activeCardTypes; ++i) {
            const FCardType *ct = ct_data[i];
            const FGeneralCardType *gct = gct_data[i];
            QSignalBlocker b4(m_cardTypeCBs[i]);
            m_cardTypeCBs[i]->setCurrentIndex(ct->id());

            updateGeneralCardTypeState(i, true);

            QSignalBlocker b5(m_generalCardTypeCBs[i]);
            m_generalCardTypeCBs[i]->setCurrentIndex(gct->id());
        }
    }


    m_activeCardTypes -= diff;
    qDebug("m_activeCardTypes: %d", m_activeCardTypes);

    ui->btn_add_cardtype->setEnabled(m_activeCardTypes < MAX_CARD_TYPES);
    qDebug("== end ==");
}

void MainWindow::buildCardTypeComboBox(int indexCardType, int indexGeneralCardType, bool blockSignal)
{
    // New Card Type
    FComboBox *cb_new = new FComboBox(this);
    cb_new->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    const QVector<FAbstractObject*> *cardTypeData = m_cardTypeModel->dataVec();
    QVector<FAbstractObject*>::const_iterator cardTypeIter;
    for (cardTypeIter = cardTypeData->constBegin(); cardTypeIter != cardTypeData->constEnd(); ++cardTypeIter) {
        const FCardType *cardType = static_cast<const FCardType*>(*cardTypeIter);
        cb_new->addItem(cardType->name(), QVariant(cardType->id()));
    }
    m_cardTypeCBs.push_back(cb_new);

    // New General Card Type
    FComboBox *cb_gct_new = new FComboBox(this);
    m_generalCardTypeCBs.push_back(cb_gct_new);
    //cb_gct_new->addItem("--", -1);
    const QVector<FAbstractObject*> *generalCardTypeData = m_generalCardTypeModel->dataVec();
    QVector<FAbstractObject*>::const_iterator gctIter;
    for (gctIter = generalCardTypeData->constBegin(); gctIter != generalCardTypeData->constEnd(); ++gctIter) {
        const FGeneralCardType *gct = static_cast<const FGeneralCardType*>(*gctIter);
        cb_gct_new->addItem(gct->name(), QVariant(gct->id()));
    }

    if (blockSignal) {
        cb_new->setCurrentIndex(indexCardType);
        cb_gct_new->setCurrentIndex(indexGeneralCardType);
        QObject::connect(cb_new, QOverload<int>::of(&FComboBox::currentIndexChanged), this, &MainWindow::cardTypeChanged);
        QObject::connect(cb_gct_new, QOverload<int>::of(&FComboBox::currentIndexChanged), this, &MainWindow::generalCardTypeChanged);

    } else {
        QObject::connect(cb_new, QOverload<int>::of(&FComboBox::currentIndexChanged), this, &MainWindow::cardTypeChanged);
        QObject::connect(cb_gct_new, QOverload<int>::of(&FComboBox::currentIndexChanged), this, &MainWindow::generalCardTypeChanged);
        cb_new->setCurrentIndex(indexCardType);
        cb_gct_new->setCurrentIndex(indexGeneralCardType);
    }

    QHBoxLayout *cb_wrapper = new QHBoxLayout();
    cb_wrapper->setMargin(0);
    cb_wrapper->addWidget(cb_new);

    QPushButton *btn_delete_cb = new QPushButton(QIcon::fromTheme("minus"), "", ui->scrollAreaWidgetContents);
    QObject::connect(btn_delete_cb, &QPushButton::clicked, this, &MainWindow::deleteCardType);
    m_deleteCardTypeBTNs.push_back(btn_delete_cb);
    cb_wrapper->addWidget(btn_delete_cb);

    ui->verticalLayout_cardtypes->addLayout(cb_wrapper);
    ui->verticalLayout_generalcardtypes->addWidget(cb_gct_new);
}

void MainWindow::deleteCardType()
{
    QPushButton *btn = static_cast<QPushButton*>(QObject::sender());
    int index = m_deleteCardTypeBTNs.indexOf(btn) + 1;
    m_deleteCardTypeBTNs[index-1]->hide();
    m_cardTypeCBs[index]->hide();
    m_generalCardTypeCBs[index]->hide();
    m_activeCardTypes--;
    if (m_activeCardTypes < MAX_CARD_TYPES) {
        ui->btn_add_cardtype->setEnabled(true);
    }
    const FCardType *ct = m_cardTypeModel->get(m_cardTypeCBs[index]->currentData().value<int>());
    if (ct) {
        m_selectedCard->removeCardType(index);
    }
    const FGeneralCardType *gct = m_generalCardTypeModel->get(m_generalCardTypeCBs[index]->currentData().value<int>());
    if (gct) {
        m_selectedCard->removeGeneralCardType(index);
    }
    enableCardTypes(m_cardTypeCBs[index]->currentIndex(), true, m_cardTypeCBs[index]);
    updateTypeOptions();
}

void MainWindow::on_actionOptions_triggered()
{
    m_optionsWindow->show();
}

void MainWindow::on_actionExport_as_PNG_triggered()
{
    // Save scene to file
    ui->widget_cardpreview->saveToPNG("Card.png");
}

void MainWindow::on_btn_ability_text_add_clicked()
{
    m_textEditDialog->refresh(tr("Ability"));
    QObject::connect(m_textEditDialog, &LanguageStringEditDialog::accepted_with_data, this, &MainWindow::dialog_ability_accepted);
    m_textEditDialog->show();
}

void MainWindow::dialog_ability_accepted(const FLanguageString langstring)
{
    if (langstring.getFilledOut() == 0) return;

    m_selectedCard->abilityTextModel()->addLanguageString(langstring);
    updateAbilityButtonStates();
}

void MainWindow::dialog_ability_edit_accepted(const FLanguageString langstring)
{
    if (langstring.getFilledOut() == 0) return;

    QModelIndex idx = ui->listView_abilities->currentIndex();
    if (!idx.isValid()) return;

    QVariant v;
    v.setValue(langstring);
    ui->listView_abilities->model()->setData(idx, v, Qt::EditRole);
}

void MainWindow::listView_ability_selectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "selection changed";
    updateAbilityButtonStates();
}

void MainWindow::on_btn_ability_text_edit_clicked()
{
    QModelIndex idx = ui->listView_abilities->currentIndex();
    if (!idx.isValid()) return;
    m_textEditDialog->refresh(tr("Ability"));
    FLanguageString langstring = ui->listView_abilities->model()->data(idx, Qt::EditRole).value<FLanguageString>();
    m_textEditDialog->setData(langstring);

    QObject::connect(m_textEditDialog, &LanguageStringEditDialog::accepted_with_data, this, &MainWindow::dialog_ability_edit_accepted);

    m_textEditDialog->show();
}

void MainWindow::on_btn_ability_text_remove_clicked()
{
    QModelIndex idx = ui->listView_abilities->currentIndex();
    if (!idx.isValid()) return;

    ui->listView_abilities->model()->removeRows(idx.row(), 1, QModelIndex());
    updateAbilityButtonStates();
}
