#ifndef FGENERALCARDTYPEMODEL_H
#define FGENERALCARDTYPEMODEL_H

#include "fabstractobject.h"
#include "fabstractyamlmodel.h"

class FGeneralCardTypeText
{
public:
    FGeneralCardTypeText() : m_name("") {}
    FGeneralCardTypeText(const QString &name) : m_name(name) {}

    const QString name() const { return m_name; }

private:
    QString m_name;
};

class FGeneralCardType : public FAbstractObject
{
public:
    FGeneralCardType() : m_stringId("") {}
    FGeneralCardType(int id, const QString &stringId) : FAbstractObject(id), m_stringId(stringId) {}
    ~FGeneralCardType();

    void addText(const QString &countryCode, FGeneralCardTypeText *text);

    const QString stringId() const { return m_stringId; }
    const QString name() const;
    const QString name(const QString &countryCode) const;

private:
    QString m_stringId;
    QHash<QString, FGeneralCardTypeText*> m_text;
};

class FGeneralCardTypeModel : public FAbstractYAMLModel
{
public:
    static const FGeneralCardTypeModel* Instance() { return m_instance; }
    static void SetInstance(const FGeneralCardTypeModel *instance) { m_instance = instance; }

    explicit FGeneralCardTypeModel(QObject *parent = nullptr);

    void loadText(const QString &countryCode);
    const FGeneralCardType* get(const QString &stringId) const;
    const FGeneralCardType* get(int id) const;

    void load() override;
    const QString YAMLFile() const override { return m_yamlFile; }

private:
    static const FGeneralCardTypeModel *m_instance;
    const QString m_yamlFile = "GeneralCardTypes.yaml";

    FGeneralCardType *value_p(const QString &stringId);
};

#endif // FGENERALCARDTYPEMODEL_H
