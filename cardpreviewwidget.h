#ifndef CARDPREVIEWWIDGET_H
#define CARDPREVIEWWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "card.h"
#include "cardpreviewitem.h"
#include "dialogs/optionswindow.h"

namespace Ui {
class CardPreviewWidget;
}

#define CARD_MARGIN 20

class CardPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardPreviewWidget(QWidget *parent = nullptr);
    ~CardPreviewWidget();

    void addCard(const Card *card);
    void saveToPNG(const QString &filename);

private:
    Ui::CardPreviewWidget *ui;
    QVector<const Card*> m_cards;
    QVector<CardPreviewItem*> m_items;
    QGraphicsScene *scene;
    QFont m_voidCostFont;

    int view_width;
    int view_height;

public slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);
    void setZoom(int zoom = 100);
    void fitInView();
    void setFont(OptionsWindow::FontUpdateType type, const QFont &font);

signals:
    void zoomChanged(int zoom);
private slots:
    void on_btn_debug_toggled(bool checked);
    void on_btn_zoom_100_clicked();
    void on_btn_zoom_50_clicked();
};

class CardPreview : public QGraphicsView
{
    Q_OBJECT
public:
    CardPreview(QWidget *parent = nullptr) : QGraphicsView(parent), m_calculatedSize(QSize(100,30)) {}
    CardPreview(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent), m_calculatedSize(QSize(100,30)) {}

    void setPreviewWidget(CardPreviewWidget *previewWidget) { m_previewWidget = previewWidget; }
    const QSize calcSize() const { return m_calculatedSize; }

    void resizeEvent(QResizeEvent *e) override;

#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *e) override;
#endif

private:
    CardPreviewWidget *m_previewWidget;
    QSize m_calculatedSize;
};

#endif // CARDPREVIEWWIDGET_H
