#ifndef FABSTRACTYAMLMODEL_H
#define FABSTRACTYAMLMODEL_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QVariant>

#include "fabstractobject.h"

class FAbstractYAMLModel : public QObject
{
    Q_OBJECT
public:
    explicit FAbstractYAMLModel(QObject *parent);
    virtual ~FAbstractYAMLModel();

    void beginLoading() { m_loading = true; }
    void endLoading() { m_loading = false; }

    virtual void load() = 0;
    //virtual int dataCount() const { return m_data.size(); }
    virtual const QString YAMLFile() const = 0;
    //virtual void insert(const QString &key, const QVariant &value);
    //virtual bool remove(const QString &key);
    //virtual bool modify(const QString &key, const QVariant &value);
    //virtual const QVariant value(const QString &key) const { return m_data[key]; }
    //virtual const QMap<QString, QVariant>* data() const { return &m_data; }

    virtual int dataCount() const { return std::max(0, m_data.size()); }
    virtual void insert(const QString &key, FAbstractObject *value);
    virtual bool remove(const QString &key);
    virtual bool modify(const QString &key, FAbstractObject *value);
    virtual const FAbstractObject* value(const QString &key) const { return m_data[key]; }
    virtual const QHash<QString, FAbstractObject*>* data() const { return &m_data; }
    virtual const QVector<FAbstractObject*>* dataVec() const { return &m_vec; }

protected:
    //QMap<QString, QVariant> m_data;
    QHash<QString, FAbstractObject*> m_data;
    QVector<FAbstractObject*> m_vec;

private:
    bool m_loading;

signals:
    void dataAdded(const QString &key);
    void dataModified(const QString &key);
    void dataDeleted(const QString &key);
};

#endif // FABSTRACTYAMLMODEL_H
