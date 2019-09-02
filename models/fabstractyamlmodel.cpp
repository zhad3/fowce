#include "fabstractyamlmodel.h"

FAbstractYAMLModel::FAbstractYAMLModel(QObject *parent) : QObject(parent), m_loading(false) {}
FAbstractYAMLModel::~FAbstractYAMLModel()
{
    QVector<FAbstractObject*>::iterator it = m_vec.begin();
    for (; it != m_vec.end(); ++it) {
        delete (*it);
    }
}

/*void FAbstractYAMLModel::insert(const QString &key, const QVariant &value)
{
    if (!m_data.contains(key)) {
        m_data.insert(key, value);
        if (!m_loading)
            emit dataAdded(key);
    } else {
        m_data.insert(key, value);
        if (!m_loading)
            emit dataModified(key);
    }
}*/

void FAbstractYAMLModel::insert(const QString &key, FAbstractObject *value)
{
    if (!m_data.contains(key)) {
        m_data.insert(key, value);
        m_vec.push_back(value);
        if (!m_loading)
            emit dataAdded(key);
    } else {
        m_data.insert(key, value);
        m_vec.replace(m_vec.indexOf(value), value);
        delete m_data[key]; // This also deletes the entry in m_vec!
        if (!m_loading)
            emit dataModified(key);
    }
}

/*bool FAbstractYAMLModel::remove(const QString &key)
{
    if (m_data.contains(key)) {
        m_data.remove(key);
        if (!m_loading)
            emit dataDeleted(key);
        return true;
    }
    return false;
}*/

bool FAbstractYAMLModel::remove(const QString &key)
{
    if (m_data.contains(key)) {
        FAbstractObject *value = m_data.value(key);
        m_data.remove(key);
        m_vec.remove(m_vec.indexOf(value));
        delete value;
        if (!m_loading)
            emit dataDeleted(key);
        return true;
    }
    return false;
}

/*bool FAbstractYAMLModel::modify(const QString &key, const QVariant &value)
{
    if(m_data.contains(key)) {
        m_data[key] = value;
        if (!m_loading)
            emit dataModified(key);
        return true;
    }
    return false;
}*/

bool FAbstractYAMLModel::modify(const QString &key, FAbstractObject *value)
{
    if (m_data.contains(key)) {
        FAbstractObject *old = m_data.value(key);
        m_data[key] = value;
        m_vec.replace(m_vec.indexOf(old), value);
        delete old;
        if (!m_loading)
            emit dataModified(key);
        return true;
    }
    return false;
}
