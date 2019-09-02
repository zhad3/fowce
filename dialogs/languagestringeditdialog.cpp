#include "languagestringeditdialog.h"
#include "ui_multilanglineeditdialog.h"
#include "util.h"
#include "models/flanguagemodel.h"
#include "widgets/multilangtextedit.h"

#include <QSettings>
#include <QLineEdit>

LanguageStringEditDialog::LanguageStringEditDialog(QWidget *parent, const FLanguageModel *languageModel, bool useTextEdit) :
    QDialog(parent),
    ui(new Ui::LanguageStringEditDialog),
    isInit(false), useTextEdit(useTextEdit)
{
    if (!languageModel) {
        m_languageModel = FLanguageModel::Instance();
    } else {
        m_languageModel = languageModel;
    }
    m_inputs.reserve(int(m_languageModel->dataCount()));
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

LanguageStringEditDialog::~LanguageStringEditDialog()
{
    delete ui;
}

void LanguageStringEditDialog::setTitle(const QString &title)
{
    setWindowTitle(tr("Edit %1", "Dialog Title").arg(title));
    ui->label_title->setText(title);
}

void LanguageStringEditDialog::setData(const FLanguageString langstring)
{
    if (!isInit) init();
    QVector<QWidget*> inputs = m_inputs;
    QVector<QWidget*>::const_iterator iter = inputs.constBegin();
    for (; iter != inputs.constEnd(); ++iter) {
        if (useTextEdit) {
            MultiLangTextEdit *edit = static_cast<MultiLangTextEdit*>(*iter);
            edit->setPlainText(langstring.text(edit->countryCode()));
        } else {
            MultiLangLineEdit *edit = qobject_cast<MultiLangLineEdit*>(*iter);
            edit->setText(langstring.text(edit->countryCode()));
        }
    }
}

void LanguageStringEditDialog::setLanguageModel(const FLanguageModel *languageModel)
{
    m_languageModel = languageModel;
    if (!isInit) {
        init();
    }
}

/*!
 * \brief Builds the UI by creating the labels and QLineEdits for each supported language. Does nothing when already initialized.
 * \param title
 */
void LanguageStringEditDialog::init()
{
    if (isInit) return;
    if (!m_languageModel) {
        qCritical(qUtf8Printable("No language model set. Cannot build dialog."));
        return;
    }

    int row = 0;

    QFont tmp_font;
    tmp_font.setPointSize(12);

    const FLanguage *selectedLanguage = m_languageModel->selectedLanguage();

    // First row: selected language
    if (useTextEdit) {
        MultiLangTextEdit *lang_edit = new MultiLangTextEdit(this, FLanguageModel::Instance(), Qt::EditRole);
        lang_edit->setFont(tmp_font);
        lang_edit->setCountryCode(selectedLanguage->countryCode());
        lang_edit->setMaximumHeight(50);

        ui->horizontalLayout->addWidget(lang_edit);

        m_inputs.push_back(lang_edit);
    } else {
        MultiLangLineEdit *lang_edit = new MultiLangLineEdit(this, FLanguageModel::Instance(), Qt::EditRole);
        lang_edit->setFont(tmp_font);
        lang_edit->setCountryCode(selectedLanguage->countryCode());

        ui->horizontalLayout->addWidget(lang_edit);

        m_inputs.push_back(lang_edit);
    }
    ui->label_selectedLanguage->setPixmap(QIcon::fromTheme(selectedLanguage->flagIconPath()).pixmap(QSize(32,32)));
//    ui->input_selectedLanguage->setRole(Qt::EditRole);
//    ui->input_selectedLanguage->setCountryCode(selectedLanguage->countryCode());

//    m_inputs.push_back(ui->input_selectedLanguage);

    // Remaining rows: iterate over language model data
    const QVector<FAbstractObject*> *data = m_languageModel->dataVec();
    QVector<FAbstractObject*>::const_iterator it;
    for (it = data->constBegin(); it != data->constEnd(); ++it) {
        const FLanguage *language = static_cast<FLanguage*>(*it);
        if (language->id() == selectedLanguage->id()) {
            continue;
        }
        row++;

        QLabel *lang_iconlabel = new QLabel(this);
        lang_iconlabel->setPixmap(QIcon::fromTheme(language->flagIconPath()).pixmap(QSize(32,32)));
        ui->gridLayout->addWidget(lang_iconlabel, row, 0);

        if (useTextEdit) {
            MultiLangTextEdit *lang_edit = new MultiLangTextEdit(this, FLanguageModel::Instance(), Qt::EditRole);
            lang_edit->setFont(tmp_font);
            lang_edit->setCountryCode(language->countryCode());
            lang_edit->setMaximumHeight(50);

            ui->gridLayout->addWidget(lang_edit, row, 1);

            m_inputs.push_back(lang_edit);
        } else {
            MultiLangLineEdit *lang_edit = new MultiLangLineEdit(this, FLanguageModel::Instance(), Qt::EditRole);
            lang_edit->setFont(tmp_font);
            lang_edit->setCountryCode(language->countryCode());

            ui->gridLayout->addWidget(lang_edit, row, 1);

            m_inputs.push_back(lang_edit);
        }
    }

    isInit = true;
}

void LanguageStringEditDialog::clearInputs()
{
    if (!isInit) return;
    QVector<QWidget*> inputs = m_inputs;
    QVector<QWidget*>::const_iterator iter = inputs.constBegin();
    for (; iter != inputs.constEnd(); ++iter) {
        if (useTextEdit) {
            MultiLangTextEdit *edit = static_cast<MultiLangTextEdit*>(*iter);
            edit->clear();
        } else {
            MultiLangLineEdit *edit = static_cast<MultiLangLineEdit*>(*iter);
            edit->clear();
        }
    }
}
/*!
 * \brief Sets the title, clears all QLineEdit inputs and disconnects all connections
 * \param title
 */
void LanguageStringEditDialog::refresh(const QString title)
{
    setTitle(title);
    //setData(langstring);
    QVector<QWidget*> inputs = m_inputs;
    QVector<QWidget*>::const_iterator iter = inputs.constBegin();
    for (; iter != inputs.constEnd(); ++iter) {
        if (useTextEdit) {
            MultiLangTextEdit *edit = static_cast<MultiLangTextEdit*>(*iter);
            edit->setPlainText("");
        } else {
            MultiLangLineEdit *edit = static_cast<MultiLangLineEdit*>(*iter);
            edit->setText("");
        }
    }
    disconnect(); // Remove all connections
}

void LanguageStringEditDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    m_inputs.first()->setFocus(Qt::PopupFocusReason);
}

void LanguageStringEditDialog::accept()
{
    // Serialize the inputs
    FLanguageString langstring(m_languageModel);
    QVector<QWidget*> inputs = m_inputs;
    QVector<QWidget*>::const_iterator iter = inputs.constBegin();
    for (; iter != inputs.constEnd(); ++iter) {
        if (useTextEdit) {
            MultiLangTextEdit *edit = static_cast<MultiLangTextEdit*>(*iter);
            langstring.setText(edit->countryCode(), edit->toPlainText());
        } else {
            MultiLangLineEdit *edit = static_cast<MultiLangLineEdit*>(*iter);
            langstring.setText(edit->countryCode(), edit->text());
        }
    }
    emit accepted_with_data(langstring);
    done(Accepted);
}

void LanguageStringEditDialog::rebuildUI(const FLanguage *language)
{
    /*QWidget *l = ui->gridLayout->itemAtPosition(0, 0)->widget();
    if (l) {
        QLabel *label = static_cast<QLabel*>(l);
        label->setPixmap(QIcon::fromTheme(language->flagIconPath()).pixmap(QSize(32,32)));
    }
    QWidget *w = ui->gridLayout->itemAtPosition(0, 1)->widget();
    if (w) {
        MultiLangLineEdit *lineEdit = static_cast<MultiLangLineEdit*>(w);
        lineEdit->setCountryCode(language->countryCode());
    }*/
    ui->label_selectedLanguage->setPixmap(QIcon::fromTheme(language->flagIconPath()).pixmap(QSize(32,32)));
    if (useTextEdit) {
        static_cast<MultiLangTextEdit*>(m_inputs[0])->setCountryCode(language->countryCode());
    } else {
        static_cast<MultiLangLineEdit*>(m_inputs[0])->setCountryCode(language->countryCode());
    }

    QFont tmp_font;
    tmp_font.setPointSize(12);



    // ==
    // This needs to be changed when modifying the layout to match up the row count
    int i = 0;
    // ==

    const QVector<FAbstractObject*> *data = m_languageModel->dataVec();
    QVector<FAbstractObject*>::const_iterator it;
    for (it = data->constBegin(); it != data->constEnd(); ++it) {
        const FLanguage *otherLanguage = static_cast<FLanguage*>(*it);
        if (otherLanguage->id() == language->id()) {
            continue;
        }

        QWidget *w = ui->gridLayout->itemAtPosition(i, 1)->widget();
        if (w) {
            // Override
            if (useTextEdit) {
                MultiLangTextEdit *edit = static_cast<MultiLangTextEdit*>(w);
                edit->setCountryCode(otherLanguage->countryCode());
            } else {
                MultiLangLineEdit *edit = static_cast<MultiLangLineEdit*>(w);
                edit->setCountryCode(otherLanguage->countryCode());
            }

            QWidget *l = ui->gridLayout->itemAtPosition(i, 0)->widget();
            if (l) {
                QLabel *label = static_cast<QLabel*>(l);
                label->setPixmap(QIcon::fromTheme(otherLanguage->flagIconPath()).pixmap(QSize(32,32)));
            }
        } else {
            // Create new
            QLabel *lang_iconlabel = new QLabel(this);
            lang_iconlabel->setPixmap(QIcon::fromTheme(otherLanguage->flagIconPath()).pixmap(QSize(32,32)));
            ui->gridLayout->addWidget(lang_iconlabel, i, 0);

            if (useTextEdit) {
                MultiLangTextEdit *lang_edit = new MultiLangTextEdit(this, FLanguageModel::Instance(), Qt::EditRole);
                lang_edit->setFont(tmp_font);
                lang_edit->setCountryCode(otherLanguage->countryCode());
                lang_edit->setMaximumHeight(50);

                ui->gridLayout->addWidget(lang_edit, i, 1);

                m_inputs.push_back(lang_edit);
            } else {
                MultiLangLineEdit *lang_edit = new MultiLangLineEdit(this, FLanguageModel::Instance(), Qt::EditRole);
                lang_edit->setFont(tmp_font);
                lang_edit->setCountryCode(otherLanguage->countryCode());

                ui->gridLayout->addWidget(lang_edit, i, 1);

                m_inputs.push_back(lang_edit);
            }
        }
        i++;
    }

    // Hide additional inputs
    for (int a = i+1; a < m_inputs.size(); ++a) {
        QWidget *w = ui->gridLayout->itemAtPosition(a, 1)->widget();
        if(w) {
            w->hide();
        }
        QWidget *l = ui->gridLayout->itemAtPosition(a, 0)->widget();
        if (l) {
            l->hide();
        }
    }
}


