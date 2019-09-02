#ifndef MULTILANGTEXTEDIT_H
#define MULTILANGTEXTEDIT_H

#include <QWidget>

namespace Ui {
class MultiLangLineEditWidget;
}

class MultiLangLineEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MultiLangLineEditWidget(QWidget *parent = nullptr);
    ~MultiLangLineEditWidget();

private:
    Ui::MultiLangLineEditWidget *ui;
};

#endif // MULTILANGTEXTEDIT_H
