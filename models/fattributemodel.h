#ifndef FATTRIBUTEMODEL_H
#define FATTRIBUTEMODEL_H

#include <QColor>

#include "fabstractyamlmodel.h"
#include "fabstractobject.h"
#include "fwillcharacteristicmodel.h"

class FAttributeText
{
public:
    FAttributeText() : m_short(""), m_name(""), m_alt("") {}
    FAttributeText(const QString &shortName, const QString &name, const QString &altName)
        : m_short(shortName), m_name(name), m_alt(altName) {}

    const QString shortName() const { return m_short; }
    const QString name() const { return m_name; }
    const QString altName() const { return m_alt; }

private:
    QString m_short;
    QString m_name;
    QString m_alt;
};

class FAttribute : public FAbstractObject
{
public:
    FAttribute() : m_stringId(""), m_iconPath(""), m_color(QColor(0,0,0,0)), m_color2(QColor(0,0,0,0)), m_exclusive(false), m_generic(false), m_maxCost(0) {}
    FAttribute(int id, const QString &stringId, const QString &iconPath, const QColor &color, const QColor &color2, bool exclusive, bool generic, int maxCost)
        : FAbstractObject(id), m_stringId(stringId), m_iconPath(iconPath), m_color(color), m_color2(color2), m_exclusive(exclusive), m_generic(generic), m_maxCost(maxCost) {}
    ~FAttribute();

    void addText(const QString &countryCode, FAttributeText *attributeText);
    void addCharacteristic(const QString &stringId);

    const QString iconPath() const { return m_iconPath; }
    const QString stringId() const { return m_stringId; }
    const QColor color() const { return m_color; }
    const QColor color2() const { return m_color2; }

    const QString shortName() const;
    const QString shortName(const QString &countryCode) const;
    const QString name() const;
    const QString name(const QString &countryCode) const;
    const QString altName() const;
    const QString altName(const QString &countryCode) const;
    bool isExclusive() const { return m_exclusive; }
    bool isGeneric() const { return m_generic; }
    int maxCost() const { return m_maxCost; }

    const QVector<const FWillCharacteristic*>* characteristics() const { return &m_characteristics; }
    //bool characteristicAllowed(const QString &stringId) const;
    //bool characteristicAllowed(const FWillCharacteristic *characteristic) const;

private:
    QString m_stringId;
    QString m_iconPath;
    QColor m_color;
    QColor m_color2;
    bool m_exclusive;
    bool m_generic;
    int m_maxCost;
    QHash<QString, FAttributeText*> m_text;
    QVector<const FWillCharacteristic*> m_characteristics;
};

class FAttributeModel : public FAbstractYAMLModel
{
    Q_OBJECT
public:
    static const FAttributeModel* Instance() { return m_instance; }
    static void SetInstance(const FAttributeModel *instance) { m_instance = instance; }

    explicit FAttributeModel(QObject *parent = nullptr);

    void loadText(const QString &countryCode);
    const FAttribute* get(const QString &stringId) const;
    const FAttribute* get(int id) const;
    const FAttribute* defaultAttribute() const { return m_defaultAttribute; }

    void load() override;
    const QString YAMLFile() const override { return m_yamlFile; }

private:
    static const FAttributeModel *m_instance;
    const QString m_yamlFile = "Attributes.yaml";
    FAttribute *m_defaultAttribute;

    FAttribute *value_p(const QString &stringId);
};

#endif // FATTRIBUTEMODEL_H
