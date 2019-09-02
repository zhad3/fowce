#ifndef YAMLCONVERT_H
#define YAMLCONVERT_H

#include <QString>
#include <yaml-cpp/yaml.h>

namespace YAML {
template<>
struct convert<QString> {
    static Node encode(const QString& rhs) {
        return Node(rhs.toStdString());
    }

    static bool decode(const Node& node, QString& rhs) {
        if (!node.IsScalar())
            return false;
        rhs = QString::fromStdString(node.Scalar());
        return true;
    }
};
}

#endif // YAMLCONVERT_H
