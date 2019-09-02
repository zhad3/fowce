#ifndef UTIL_H
#define UTIL_H

#include <QColor>
#include <QTextFormat>
#include <QDomNode>
#include <QStringBuilder>
#include <QFile>
#include <QtSvg>

#define APPNAME "FOWCE\0"
#define ORGNAME "FOWCE\0"

#define MAX_CARDSIDES 3
#define MAX_CARD_TYPES 2

#define MAX_CARD_COST 5
#define MAX_CARD_COST_GENERIC 9
#define MAX_CARD_COST_DYNAMIC 2 // Maximum dynamic cost. 1 = X, 2 = XX, 3 = XXX, ...
#define MAX_TOTAL_CARD_COST 5 // Non-generic
#define MAX_TOTAL_CARD_COST_GENERIC 9

class Util
{
public:
/*static QString StringToQString(std::string str)
{
    int size;
    if (str.size() > INT_MAX) {
        size = INT_MAX;
    } else {
        size = static_cast<int>(str.size());
    }
    return QString::fromUtf8(str.data(), size);
};*/
    static bool DrawDebugInfo;

    class XML
    {
    public:
        static QDomNode findElementById(QDomNode root, const QString &id)
        {
            if (root.isElement() && root.attributes().size() > 0) {
                QDomElement rootEl = root.toElement();
                if (rootEl.hasAttribute("id") && rootEl.attribute("id", "") == id) {
                    return root;
                }
            }
            if (root.childNodes().length() == 0) {
                return QDomNode();
            }
            QDomNodeList children = root.childNodes();
            QDomNode found;
            for (int i = 0; i < children.size(); ++i) {
                found = findElementById(children.at(i), id);
                if (!found.isNull()) {
                    return found;
                }
            }
            return QDomNode();
        }

        static QMap<QString, QString> getStyleAttrib(QDomElement node)
        {
            QMap<QString, QString> styles;
            if (node.hasAttribute("style")) {
                QString styleString = node.attribute("style", "");
                QStringList styleList = styleString.split(";", QString::SplitBehavior::SkipEmptyParts);
                QStringList::iterator styleIter = styleList.begin();
                for (; styleIter != styleList.end(); ++styleIter) {
                    QStringList keyValue = styleIter->split(":");
                    if (keyValue.size() != 2) {
                        continue;
                    }
                    styles[keyValue.at(0)] = keyValue.at(1);
                }
            }
            return styles;
        }

        static QString styleAttribToString(QMap<QString, QString> styles)
        {
            QString styleString("");
            QMap<QString, QString>::const_iterator styleIter = styles.constBegin();
            for (;styleIter != styles.constEnd(); ++styleIter) {
                if (styleIter+1 == styles.constEnd()) {
                    styleString = styleString % styleIter.key() % ":" % styleIter.value();
                } else {
                    styleString = styleString % styleIter.key() % ":" % styleIter.value() % ";";
                }
            }
            return styleString;
        }

        static QPixmap svgToPixmap(const QString &filename, const QSize &size, const QPen &outline = Qt::NoPen, bool increaseQuality = true)
        {
            if (size.isNull() || filename.isEmpty() || (size.width() < 0 && size.height() < 0)) {
                return QPixmap();
            }
            QFile svgFile(filename);
            if (!svgFile.open(QIODevice::ReadOnly)) {
                return QPixmap();
            }
            QByteArray svgData(svgFile.readAll());

            QDomDocument domDoc;
            domDoc.setContent(svgData);

            QDomElement svg = domDoc.firstChildElement();
            int svgHeight = svg.attribute("height").toInt();
            int svgWidth = svg.attribute("width").toInt();
            qreal aspectRatio = svgHeight / svgWidth;
            qreal outlineWidth = 0.0;
            int height = size.height() * (increaseQuality ? 2 : 1); // We render at x2 for increased quality
            int width = size.width() * (increaseQuality ? 2 : 1);

            if (size.width() == -1) {
                width = qCeil((size.height() / aspectRatio) * (increaseQuality ? 2 : 1));
            } else if (size.height() == -1) {
                height = qCeil((size.width() * aspectRatio) * (increaseQuality ? 2 : 1));
            }

            if (outline.color().alpha() > 0 && outline != Qt::NoPen) {
                outlineWidth = qFloor((svgHeight/height) * outline.widthF() * (increaseQuality ? 2 : 1));
            }

            // If we have an outline set, this will increase the viewBox so the outline won't be cut off
            QRectF viewBox = QRectF(-outlineWidth, -outlineWidth, svgHeight + outlineWidth*2, svgWidth + outlineWidth*2);

            QPixmap pix(QSize(qCeil(width), qCeil(height)));
            pix.fill(Qt::transparent);

            QPainter p(&pix);

            if (outline.color().alpha() > 0 && outline != Qt::NoPen) {
                // Do DOM modification to set the outline directly inside the SVG
                // We search for a specific element with id "background" and therefore
                // assume the SVGs provided to this function will have the right SVG structure
                QDomElement backgroundElem = findElementById(domDoc, "background").toElement();
                if (!backgroundElem.isNull()) {
                    QString oldStyles = backgroundElem.attribute("style");

                    QMap<QString, QString> styles = getStyleAttrib(backgroundElem);
                    styles["stroke"] = outline.color().name();
                    styles["stroke-width"] = QString::number(outlineWidth * (increaseQuality ? 2 : 1)); // Times two, because we also increased size by x2
                    QString styleString = styleAttribToString(styles);
                    backgroundElem.setAttribute("style", styleString);

                    // We render the outline twice because it is centered on the border
                    // First render outline and then render the normal SVG ontop
                    QByteArray svgOutlineData = domDoc.toByteArray();
                    QSvgRenderer outlineRenderer(svgOutlineData);
                    outlineRenderer.setViewBox(viewBox);
                    outlineRenderer.render(&p, pix.rect());

                    // Restore style because we render again without outline
                    backgroundElem.setAttribute("style", oldStyles);
                }
            }

            QSvgRenderer renderer(domDoc.toByteArray());
            renderer.setViewBox(viewBox);
            renderer.render(&p, pix.rect());

            return pix;
        }
    };

    class TextObject
    {
    public:
        enum Format { KeywordTextFormat = QTextFormat::UserObject + 1, SymbolTextFormat = QTextFormat::UserObject + 2 };
        enum { KeywordData = 1, KeywordGradient = 2 };
        enum { SymbolData = 1, SymbolOutlineWidth = 2, SymbolText = 3, SymbolFont = 4 };

        struct Replacement
        {
            Util::TextObject::Format objectType;
            QString symbolName;
        };
        const static QMap<QString, Util::TextObject::Replacement> Replacements;
    };

    /* Colors */
    const static QColor Blue;
    const static QColor Red;
    const static QColor Green;

    const static QColor BoxDefaultColor;
    const static QColor TextBoxTopRegionColor;
    const static QColor TextBoxBottomRegionColor;

};

class DialogRole
{
    Q_GADGET
    public:
    enum Role : unsigned int {
        Add = 0,
        Edit
    };
    Q_ENUM(Role)
};

#endif // UTIL_H
