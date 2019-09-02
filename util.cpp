#include "util.h"

bool Util::DrawDebugInfo = false;

const QColor Util::Blue = QColor(120, 200, 255);
const QColor Util::Red = QColor(255, 120, 120);
const QColor Util::Green = QColor(120, 255, 120);

const QColor Util::BoxDefaultColor = QColor(23, 23, 23, 127);
const QColor Util::TextBoxTopRegionColor = QColor(160, 160, 160, 230);
const QColor Util::TextBoxBottomRegionColor = QColor(255, 255, 255, 200);

const QMap<QString, Util::TextObject::Replacement> Util::TextObject::Replacements =
{
    {"w", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-light.svg"}},
    {"r", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-fire.svg"}},
    {"u", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-water.svg"}},
    {"g", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-wind.svg"}},
    {"b", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-darkness.svg"}},
    {"v", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-void.svg"}},
    {"moon", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-moon.svg"}},
    {"time", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/attribute-time.svg"}},
    {"rest", Util::TextObject::Replacement{Util::TextObject::SymbolTextFormat, ":/svg/symbol-rest.svg"}}
};
