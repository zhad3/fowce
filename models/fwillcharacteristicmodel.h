#ifndef FWILLCHARACTERISTICMODEL_H
#define FWILLCHARACTERISTICMODEL_H

#include "fabstractyamlmodel.h"

class FWillCharacteristicText
{
public:
    FWillCharacteristicText() : m_name("") {}
    FWillCharacteristicText(const QString &name) : m_name(name) {}

    const QString name() const { return m_name; }
private:
    QString m_name;
};

class FWillCharacteristic : public FAbstractObject
{
public:
    FWillCharacteristic();
    FWillCharacteristic(int id, const QString &stringId, const QString &iconPath)
        : FAbstractObject(id), m_stringId(stringId), m_iconPath(iconPath) {}
    ~FWillCharacteristic();

    void addText(const QString &countryCode, FWillCharacteristicText *characteristicText);

    const QString stringId() const { return m_stringId; }
    const QString iconPath() const { return m_iconPath; }

    const QString name() const;
    const QString name(const QString &countryCode) const;

private:
    QString m_stringId;
    QString m_iconPath;
    QHash<QString, FWillCharacteristicText*> m_text;
};

class FWillCharacteristicModel : public FAbstractYAMLModel
{
    Q_OBJECT
public:
    static const FWillCharacteristicModel* Instance() { return m_instance; }
    static void SetInstance(const FWillCharacteristicModel *instance) { m_instance = instance; }

    explicit FWillCharacteristicModel(QObject *parent = nullptr);

    void loadText(const QString &countryCode);
    const FWillCharacteristic* get(const QString &stringId) const;
    const FWillCharacteristic* get(int id) const;

    void load() override;
    const QString YAMLFile() const override { return m_yamlFile; }

private:
    static const FWillCharacteristicModel *m_instance;
    const QString m_yamlFile = "WillCharacteristics.yaml";

    FWillCharacteristic *value_p(const QString &stringId);
};

#endif // FWILLCHARACTERISTICMODEL_H
