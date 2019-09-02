#ifndef FLANGUAGESTRING_H
#define FLANGUAGESTRING_H

#include <QObject>
#include <QMap>

#include "flanguagemodel.h"

class FLanguageStringData : public QSharedData
{
public:
    FLanguageStringData() {}
    FLanguageStringData(const FLanguageStringData &other)
        : QSharedData(other), data(other.data), languageModel(other.languageModel), filledOut(other.filledOut) {}
    ~FLanguageStringData() {}

    QHash<QString, QString> data;
    const FLanguageModel *languageModel;
    int filledOut;

};

class FLanguageString
{
public:
    FLanguageString();
    FLanguageString(const FLanguageString &other) : d(other.d) {}
    FLanguageString(const FLanguageModel *languageModel);

    const QHash<QString, QString>* data() const { return &d->data; }
    const FLanguageModel *languageModel() const { return d->languageModel; }
    void setLanguageModel(const FLanguageModel *languageModel) { d->languageModel = languageModel; }
    int getFilledOut() const { return d->filledOut; }

    const QString text() const;
    const QString text(const QString &countryCode) const { return d->data.value(countryCode, QString()); }
    const QString text(const FLanguage &language) const { return d->data.value(language.countryCode(), QString()); }
    void setText(const QString &countryCode, const QString &text) { setText_p(countryCode, text); }
    void setText(const FLanguage &language, const QString &text) { setText_p(language.countryCode(), text); }

private:
    QSharedDataPointer<FLanguageStringData> d;

    void setText_p(const QString &countryCode, const QString &text);
    void updateFilledOut();
};

Q_DECLARE_METATYPE(FLanguageString);
Q_DECLARE_METATYPE(FLanguageString*);

/*class FLanguageString
{
public:
    FLanguageString(const FLanguageModel *languageModel = nullptr);
    const QMap<QString, QString>* data() const { return &m_strings; }

    const FLanguageModel* languageModel() const { return m_languageModel; }
    void setLanguageModel(const FLanguageModel *languageModel) { m_languageModel = languageModel; }

    unsigned int getFilledOut() const { return m_filledOut; }

    const QString text() const;
    const QString text(const QString &countryCode) const { return m_strings.value(countryCode, QString()); }
    const QString text(const FLanguage &language) const { return m_strings.value(language.countryCode(), QString()); }
    void setText(const QString &countryCode, const QString &text) { setText_p(countryCode, text); }
    void setText(const FLanguage &language, const QString &text) { setText_p(language.countryCode(), text); }

private:
    QMap<QString, QString> m_strings;
    const FLanguageModel *m_languageModel;
    unsigned int m_filledOut;

    void setText_p(const QString &countryCode, const QString &text);
    void updateFilledOut();
};

Q_DECLARE_METATYPE(FLanguageString);*/

#endif // FLANGUAGESTRING_H
