#ifndef FCOMBOBOX_H
#define FCOMBOBOX_H

#include <QComboBox>

class FComboBox : public QComboBox
{
    Q_OBJECT
public:
    FComboBox(QWidget *parent = nullptr) : QComboBox(parent), m_previousIndex(0) {}
    int previousIndex() { return m_previousIndex; }
    bool event(QEvent *event) override;

private:
    int m_previousIndex;
};

#endif // FCOMBOBOX_H
