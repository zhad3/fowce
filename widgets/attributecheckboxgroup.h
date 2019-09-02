#ifndef ATTRIBUTECHECKBOXGROUP_H
#define ATTRIBUTECHECKBOXGROUP_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include "models/fattributemodel.h"

#define ATTRIBUTES_PER_ROW 3

namespace Ui {
class AttributeCheckBoxGroup;
}

class AttributeCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit AttributeCheckBox(QWidget *parent = nullptr, const FAttribute *attribute = nullptr)
        : QCheckBox(parent), m_attribute(attribute) {}

    void setAttribute(const FAttribute *attribute) { m_attribute = attribute; }
    const FAttribute *attribute() const { return m_attribute; }

private:
    const FAttribute *m_attribute;
};

class AttributeCheckBoxGroup : public QWidget
{
    Q_OBJECT

public:
    explicit AttributeCheckBoxGroup(QWidget *parent = nullptr, const FAttributeModel *attributeModel = nullptr);
    ~AttributeCheckBoxGroup();

    void setAttributeModel(const FAttributeModel *attributeModel) { m_attributeModel = attributeModel; }
    const FAttributeModel *attributeModel() const { return m_attributeModel; }

    void setAttributes(const QMap<int, const FAttribute*> attributes);

    void rebuildUI();

private:
    const FAttributeModel *m_attributeModel;
    QHash<const FAttribute*, AttributeCheckBox*> m_inputs;
    QVBoxLayout *m_mainVerticalLayout;
    Ui::AttributeCheckBoxGroup *ui;


private slots:
    void attributeChanged(int state);

signals:
    void attributeChecked(const FAttribute *attribute);
    void attributeUnchecked(const FAttribute *attribute);
};

#endif // ATTRIBUTECHECKBOXGROUP_H
