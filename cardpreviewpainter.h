#ifndef CARDPREVIEWPAINTER_H
#define CARDPREVIEWPAINTER_H

#include <QPainter>

class CardPreviewPainter : public QPainter
{
public:
    CardPreviewPainter() : QPainter() {}
    CardPreviewPainter(QPaintDevice *device) : QPainter(device) {}
};

#endif // CARDPREVIEWPAINTER_H
