#ifndef MULTILANGTEXTEDIT_H
#define MULTILANGTEXTEDIT_H

#include <QPlainTextEdit>
#include "models/flanguagemodel.h"
#include "models/flanguagestring.h"
#include "util.h"

class MultiLangTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    MultiLangTextEdit(QWidget *parent, const FLanguageModel *languageModel = nullptr, Qt::ItemDataRole role = Qt::DisplayRole);
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

#endif // MULTILANGTEXTEDIT_H
