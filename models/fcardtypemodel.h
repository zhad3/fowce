#ifndef FCARDTYPEMODEL_H
#define FCARDTYPEMODEL_H

#include "fabstractyamlmodel.h"
#include "fabstractobject.h"
#include "fgeneralcardtypemodel.h"

class FCardTypeText
{
public:
    FCardTypeText() : m_name("") {}
    FCardTypeText(const QString &name) : m_name(name) {}

    const QString name() const { return m_name; }
    const QString name(const QString &generalCardTypeId);

    void addCombinedName(const QString &typeId, const QString &name);
private:
    QString m_name;
    QHash<QString, QString> m_combinedName;
};

class FCardType : public FAbstractObject
{
public:
    FCardType() : m_stringId(""), m_canFight(false), m_hasDivinity(false), m_hasCost(true) {}
    FCardType(int id, const QString &stringId, bool canFight, bool hasDivinity, bool hasCost)
        : FAbstractObject(id), m_stringId(stringId), m_canFight(canFight), m_hasDivinity(hasDivinity), m_hasCost(hasCost) {}
    ~FCardType();

    void addText(const QString &countryCode, FCardTypeText *cardTypeText);
    void addGeneralCardType(const QString &stringId);

    const QString stringId() const { return m_stringId; }
    bool canFight() const { return m_canFight; }
    bool hasDivinity() const { return m_hasDivinity; }
    bool hasCost() const { return m_hasCost; }

    const QString name() const;
    const QString name(const QString &countryCode) const;
    const QString nameCombined(const QString &generalCardTypeId) const;
    const QString nameCombined(const QString &countryCode, const QString &generalCardTypeId) const;

    const QVector<const FGeneralCardType*>* generalCardTypes() const { return &m_generalCardTypes; }
private:
    QString m_stringId;
    bool m_canFight;
    bool m_hasDivinity;
    bool m_hasCost;
    QHash<QString, FCardTypeText*> m_text;
    QHash<QString, FCardTypeText*> m_combinedText;
    QVector<const FGeneralCardType*> m_generalCardTypes;
};

class FCardTypeModel : public FAbstractYAMLModel
{
    Q_OBJECT
public:
    static const FCardTypeModel* Instance() { return m_instance; }
    static void SetInstance(const FCardTypeModel *instance) { m_instance = instance; }

    explicit FCardTypeModel(QObject *parent = nullptr);

    void loadText(const QString &countryCode);
    const FCardType* get(const QString &stringId) const;
    const FCardType* get(int id) const;

    void load() override;
    const QString YAMLFile() const override { return m_yamlFile; }

private:
    static const FCardTypeModel *m_instance;
    const QString m_yamlFile = "CardTypes.yaml";

    FCardType *value_p(const QString &stringId);
};

#endif // FCARDTYPEMODEL_H
