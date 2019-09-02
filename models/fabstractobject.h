#ifndef FABSTRACTOBJECT_H
#define FABSTRACTOBJECT_H


class FAbstractObject
{
public:
    FAbstractObject() = default;
    FAbstractObject(int id) : m_id(id) {}
    int id() const { return m_id; }

    bool operator<(const FAbstractObject &other) { return id() < other.id(); }

protected:
    int m_id;
};

#endif // FABSTRACTOBJECT_H
