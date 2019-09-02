#ifndef MULTILANGLINEEDIT_H
#define MULTILANGLINEEDIT_H

#include <QLineEdit>
#include "models/flanguagemodel.h"
#include "models/flanguagestring.h"
#include "util.h"

class MultiLangLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    MultiLangLineEdit(QWidget *parent, const FLanguageModel *languageModel = nullptr, Qt::ItemDataRole role = Qt::DisplayRole);
    void setLanguageString(FLanguageString langstring);
    void setRole(Qt::ItemDataRole role);
    void setCountryCode(QString code) { m_countryCode = code; }
    const FLanguageString getLanguageString() const { return m_langstring; }
    Qt::ItemDataRole role() const { return m_role; }
    const QString countryCode() const { return m_countryCode; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    FLanguageString m_langstring;
    const FLanguageModel *m_languageModel;
    QString m_countryCode;
    Qt::ItemDataRole m_role;
};

#endif // MULTILANGLINEEDIT_H
