#include <QLocale>
#include "flanguagestring.h"

/*FLanguageString::FLanguageString(const FLanguageModel *languageModel) :
    m_languageModel(languageModel), m_filledOut(0)
{
}

const QString FLanguageString::text() const
{
    if (m_languageModel) {
        return m_strings.value(m_languageModel->selectedLanguage()->countryCode(), QString());
    }
    if (m_strings.size() > 0) {
        m_strings.first();
    }
    return QString();
}

void FLanguageString::setText_p(const QString &countryCode, const QString &text)
{
    QLocale locale(countryCode);
    if (locale.language() == QLocale::Language::C) {
        qWarning(qUtf8Printable(QString("Invalid language id '%1'.").arg(countryCode)));
    } else {
        if (m_languageModel && !m_languageModel->data()->contains(countryCode)) {
            qWarning(qUtf8Printable(QString("Language '%1' does not exist in the language model.").arg(countryCode)));
        }
        if (!text.isEmpty()) {
            if (!m_strings.contains(countryCode) || m_strings.value(countryCode).isEmpty()) {
                m_filledOut++;
            }
        } else {
            if (m_strings.contains(countryCode) && !m_strings.value(countryCode).isEmpty()) {
                m_filledOut--;
            }
        }
        m_strings.insert(countryCode, text);
    }
}

void FLanguageString::updateFilledOut()
{
    unsigned int filled_out = 0;
    if (m_languageModel) {
        QHash<QString,FAbstractObject*>::const_iterator it;
        for (it = m_languageModel->data()->begin(); it != m_languageModel->data()->end(); ++it) {
            if (m_strings.contains(it.key()) && !m_strings.value(it.key()).isEmpty()) {
                filled_out++;
            }
        }
    } else {
        QMap<QString, QString>::const_iterator it;
        for (it = m_strings.begin(); it != m_strings.end(); ++it) {
            if (!m_strings.value(it.key()).isEmpty()) {
                filled_out++;
            }
        }
    }
    m_filledOut = filled_out;
}*/

FLanguageString::FLanguageString()
{
    d = new FLanguageStringData;
    d->languageModel = FLanguageModel::Instance();
    d->filledOut = 0;
}

FLanguageString::FLanguageString(const FLanguageModel *languageModel)
{
    d = new FLanguageStringData;
    d->languageModel = languageModel;
    d->filledOut = 0;
}

const QString FLanguageString::text() const
{
    if (d->languageModel) {
        return d->data.value(d->languageModel->selectedLanguage()->countryCode(), QString());
    }
    return QString();
}

void FLanguageString::setText_p(const QString &countryCode, const QString &text)
{
    QLocale locale(countryCode);
    if (locale.language() == QLocale::Language::C) {
        qWarning(qUtf8Printable(QString("Invalid language id '%1'.").arg(countryCode)));
    } else {
        if (d->languageModel && !d->languageModel->data()->contains(countryCode)) {
            qWarning(qUtf8Printable(QString("Language '%1' does not exist in the language model.").arg(countryCode)));
        }
        if (!text.isEmpty()) {
            if (!d->data.contains(countryCode) || d->data.value(countryCode).isEmpty()) {
                d->filledOut++;
            }
        } else {
            if (d->data.contains(countryCode) && !d->data.value(countryCode).isEmpty()) {
                d->filledOut--;
            }
        }
        d->data.insert(countryCode, text);
    }
}

void FLanguageString::updateFilledOut()
{
    int filled_out = 0;
    if (d->languageModel) {
        QHash<QString,FAbstractObject*>::const_iterator it;
        for (it = d->languageModel->data()->begin(); it != d->languageModel->data()->end(); ++it) {
            if (d->data.contains(it.key()) && !d->data.value(it.key()).isEmpty()) {
                filled_out++;
            }
        }
    } else {
        QHash<QString, QString>::const_iterator it;
        for (it = d->data.begin(); it != d->data.end(); ++it) {
            if (!d->data.value(it.key()).isEmpty()) {
                filled_out++;
            }
        }
    }
    d->filledOut = filled_out;
}
