#ifndef FRARITYMODEL_H
#define FRARITYMODEL_H

#include "fabstractobject.h"
#include "fabstractyamlmodel.h"

class FRarityText
{
public:
    FRarityText() : m_name(""), m_short("") {}
    FRarityText(const QString &name, const QString &shortName) : m_name(name), m_short(shortName) {}

    const QString name() const { return m_name; }
    const QString shortName() const { return m_short; }

private:
    QString m_name;
    QString m_short;
};

class FRarity : public FAbstractObject
{
public:
    FRarity() : m_stringId("") {}
    FRarity(int id, const QString &stringId) : FAbstractObject(id), m_stringId(stringId) {}
    ~FRarity();

    void addText(const QString &countryCode, FRarityText *text);

    const QString name() const;
    const QString name(const QString &countryCode) const;
    const QString shortName() const;
    const QString shortName(const QString &countryCode) const;

private:
    QString m_stringId;
    QHash<QString, FRarityText*> m_text;
};

class FRarityModel : public FAbstractYAMLModel
{
public:
    static const FRarityModel* Instance() { return m_instance; }
    static void SetInstance(const FRarityModel *instance) { m_instance = instance; }
    explicit FRarityModel(QObject *parent = nullptr);

    void loadText(const QString &countryCode);
    const FRarity* get(const QString &stringId) const;
    const FRarity* get(int id) const;

    void load() override;
    const QString YAMLFile() const override { return m_yamlFile; }

private:
    static const FRarityModel *m_instance;
    const QString m_yamlFile = "Rarities.yaml";

    FRarity *value_p(const QString &stringId);
};

#endif // FRARITYMODEL_H
