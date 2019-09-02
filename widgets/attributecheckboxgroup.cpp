#include "attributecheckboxgroup.h"
#include "ui_attributecheckboxgroup.h"
#include <QHBoxLayout>

AttributeCheckBoxGroup::AttributeCheckBoxGroup(QWidget *parent, const FAttributeModel *attributeModel) :
    QWidget(parent),
    ui(new Ui::AttributeCheckBoxGroup)
{
    if (!attributeModel && FAttributeModel::Instance()) {
        m_attributeModel = FAttributeModel::Instance();
    }

    m_mainVerticalLayout = new QVBoxLayout();
    m_mainVerticalLayout->setContentsMargins(0, 0, 0, 0);

    ui->setupUi(this);

    if (m_attributeModel) {
        QVector<FAbstractObject*>::const_iterator it;
        int i = 0;
        QHBoxLayout *hbox = new QHBoxLayout();
        m_mainVerticalLayout->addLayout(hbox);
        for (it = m_attributeModel->dataVec()->begin(); it != m_attributeModel->dataVec()->end(); ++it) {
            const FAttribute *attribute = static_cast<const FAttribute*>(*it);

            AttributeCheckBox *cb = new AttributeCheckBox();
            cb->setIcon(QIcon::fromTheme(attribute->iconPath()));
            cb->setText(attribute->name());
            cb->setAttribute(attribute);

            if (attribute->isExclusive() && m_attributeModel->defaultAttribute() == attribute) {
                cb->setEnabled(false);
            }
            // If we do not allow 'no attribute' uncomment the following block
            /*if (m_attributeModel->defaultAttribute() == attribute) {
                cb->setCheckState(Qt::Checked);
            }*/
            QObject::connect(cb, &AttributeCheckBox::stateChanged, this, &AttributeCheckBoxGroup::attributeChanged);
            m_inputs.insert(attribute, cb);

            hbox->addWidget(cb);

            i++;

            // End row when maximum reached or end of data
            if (i % ATTRIBUTES_PER_ROW == 0 || it + 1 == m_attributeModel->dataVec()->end()) {
                hbox->addStretch(0);
            }

            // There is more data, add new row
            if (i % ATTRIBUTES_PER_ROW == 0 && it + 1 != m_attributeModel->dataVec()->end()) {
                hbox = new QHBoxLayout();
                m_mainVerticalLayout->addLayout(hbox);
            }
        }
    }

    this->setLayout(m_mainVerticalLayout);
}

AttributeCheckBoxGroup::~AttributeCheckBoxGroup()
{
    delete ui;
}

void AttributeCheckBoxGroup::setAttributes(const QMap<int, const FAttribute*> attributes)
{
    const QHash<const FAttribute*, AttributeCheckBox*> data = m_inputs;
    QHash<const FAttribute*, AttributeCheckBox*>::const_iterator it = data.constBegin();
    for (; it != data.constEnd(); ++it) {
        (*it)->setChecked(attributes.contains(it.key()->id()));
    }
}

void AttributeCheckBoxGroup::rebuildUI()
{

}

void AttributeCheckBoxGroup::attributeChanged(int state)
{
    const AttributeCheckBox *cb = static_cast<const AttributeCheckBox*>(sender());

    int check_count = 0;
    if (state == Qt::CheckState::Checked) {
        check_count++;
        emit attributeChecked(cb->attribute());
    } else if (state == Qt::CheckState::Unchecked) {
        emit attributeUnchecked(cb->attribute());
    }

    // Uncheck all others if the current selected is tagged 'exclusive'
    QHash<const FAttribute*, AttributeCheckBox*>::iterator it;
    for (it = m_inputs.begin(); it != m_inputs.end(); ++it) {
        AttributeCheckBox *iter_cb = it.value();
        if (iter_cb == cb) continue;
        if ((cb->attribute()->isExclusive() || iter_cb->attribute()->isExclusive()) && state == Qt::CheckState::Checked && iter_cb->isChecked()) {
            const QSignalBlocker blocker(iter_cb);
            iter_cb->setChecked(false);
            emit attributeUnchecked(iter_cb->attribute());
        }
        if (iter_cb->attribute() != m_attributeModel->defaultAttribute() && iter_cb->isChecked()) {
            check_count++;
        }
    }

    // If we do not allow 'no attribute' uncomment the following block
    /*if (check_count == 0) {
        AttributeCheckBox *default_cb = m_inputs[m_attributeModel->defaultAttribute()];
        const QSignalBlocker blocker(default_cb);
        default_cb->setChecked(true);
        emit attributeChecked(default_cb->attribute());
    }*/
}
