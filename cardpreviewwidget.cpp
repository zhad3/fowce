#include "cardpreviewwidget.h"
#include "ui_cardpreviewwidget.h"
#include "cardpreviewitem.h"

#include <QDebug>

#if QT_CONFIG(wheelevent)
#include <QWheelEvent>
#endif

CardPreviewWidget::CardPreviewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CardPreviewWidget),
    view_width(100),
    view_height(30)
{
    ui->setupUi(this);

    ui->graphicsView->setPreviewWidget(this);
    ui->graphicsView->setBackgroundBrush(QBrush(QColor(230,230,230)));

    scene = new QGraphicsScene(this);

    ui->graphicsView->setScene(scene);

    QObject::connect(ui->slider_zoom, &QSlider::valueChanged, ui->spinbox_zoom, &QSpinBox::setValue);
    QObject::connect(ui->slider_zoom, &QSlider::valueChanged, this, &CardPreviewWidget::setZoom);
    QObject::connect(ui->spinbox_zoom, QOverload<int>::of(&QSpinBox::valueChanged), ui->slider_zoom, &QSlider::setValue);
    QObject::connect(ui->spinbox_zoom, QOverload<int>::of(&QSpinBox::valueChanged), this, &CardPreviewWidget::setZoom);

    QObject::connect(this, &CardPreviewWidget::zoomChanged, ui->slider_zoom, &QSlider::setValue);

    QObject::connect(ui->btn_fit_view, &QPushButton::clicked, this, &CardPreviewWidget::fitInView);
}

CardPreviewWidget::~CardPreviewWidget()
{
    delete ui;
}

void CardPreviewWidget::addCard(const Card *card)
{
    if (!m_cards.contains(card)) {
        CardPreviewItem *item = new CardPreviewItem(card);
        qreal totalWidth = 0;
        if (m_cards.size() > 1) {
            const QVector<CardPreviewItem*> data = m_items;
            QVector<CardPreviewItem*>::const_iterator it = data.constBegin();
            for (; it != data.constEnd(); ++it) {
                totalWidth += (*it)->boundingRect().width() + CARD_MARGIN;
            }
        } else if (m_cards.size() == 1) {
            totalWidth = m_items.first()->boundingRect().width() + CARD_MARGIN;
        }
        m_items.push_back(item);
        m_cards.push_back(card);
        item->setPos(totalWidth, 0);
        scene->addItem(item);
    }
}

void CardPreviewWidget::saveToPNG(const QString &filename)
{

    QRectF sceneRect = scene->sceneRect();
    QPixmap pix(sceneRect.size().toSize());
    QPainter p(&pix);
    scene->render(&p, sceneRect);
    pix.save(filename, "png", 100);
}

void CardPreviewWidget::zoomIn(int level)
{
    int zoom = ui->spinbox_zoom->value() + level;
    emit zoomChanged(zoom);
}
void CardPreviewWidget::zoomOut(int level)
{
    int zoom = ui->spinbox_zoom->value() + level;
    emit zoomChanged(zoom);
}
void CardPreviewWidget::setZoom(int zoom)
{
    // Update graphicsView
    qreal xscale = qreal(zoom) / 100;
    qreal yscale = qreal(zoom) / 100;
    QRectF unity = ui->graphicsView->matrix().mapRect(QRectF(0, 0, 1, 1));
    if (unity.isEmpty()) return;
    ui->graphicsView->scale(1 / unity.width(), 1 / unity.height());
    ui->graphicsView->scale(xscale, yscale);
}
void CardPreviewWidget::fitInView()
{
    if (m_items.size() == 0) return;

    qreal totalWidth = 0;
    if (m_items.size() > 1) {
        const QVector<CardPreviewItem*> data = m_items;
        QVector<CardPreviewItem*>::const_iterator it = data.constBegin();
        for (; it != data.constEnd(); ++it) {
            totalWidth += (*it)->boundingRect().width() + CARD_MARGIN;
        }
        totalWidth -= CARD_MARGIN; // Remove last margin
    } else if (m_items.size() == 1) {
        totalWidth = m_items.first()->boundingRect().width();
    }
    // Fit to view
    QRectF itemsRect = QRectF(0, 0, totalWidth, m_items.first()->boundingRect().height());
    ui->graphicsView->fitInView(itemsRect, Qt::AspectRatioMode::KeepAspectRatio);

    // QGraphicsView::fitInView
    // Reset the view scale to 1:1.
    QRectF unity = ui->graphicsView->matrix().mapRect(QRectF(0, 0, 1, 1));
    if (unity.isEmpty()) return;

    ui->graphicsView->scale(1 / unity.width(), 1 / unity.height());

    // Find the ideal x / y scaling ratio to fit a rect in the view.
    int margin = 2;
    QRectF viewRect = ui->graphicsView->viewport()->rect().adjusted(margin, margin, -margin, -margin);
    if (viewRect.isEmpty()) return;

    QRectF sceneRect = ui->graphicsView->matrix().mapRect(itemsRect);
    if (sceneRect.isEmpty()) return;

    qreal xratio = viewRect.width() / sceneRect.width();
    qreal yratio = viewRect.height() / sceneRect.height();

    // Respect the aspect ratio mode.
    xratio = yratio = qMin(xratio, yratio);

    ui->graphicsView->scale(xratio, yratio);
    ui->graphicsView->centerOn(itemsRect.center());

    const QSignalBlocker blocker(ui->slider_zoom);
    const QSignalBlocker blocker2(ui->spinbox_zoom);
    ui->slider_zoom->setValue(int(round(xratio * 100)));
    ui->spinbox_zoom->setValue(int(round(xratio * 100)));
}

void CardPreviewWidget::setFont(OptionsWindow::FontUpdateType type, const QFont &font)
{
    QVector<CardPreviewItem*> data = m_items;
    QVector<CardPreviewItem*>::iterator iter = data.begin();
    for (; iter != data.end(); ++iter) {
        (*iter)->setTextItemFont(type, font);
    }
}

void CardPreviewWidget::on_btn_debug_toggled(bool checked)
{
    Util::DrawDebugInfo = checked;
    QVector<CardPreviewItem*> data = m_items;
    QVector<CardPreviewItem*>::iterator iter = data.begin();
    for (; iter != data.end(); ++iter) {
        (*iter)->redraw();
    }
}

void CardPreviewWidget::on_btn_zoom_100_clicked()
{
    //ui->graphicsView->scale(1, 1);
    ui->slider_zoom->setValue(100);
}

void CardPreviewWidget::on_btn_zoom_50_clicked()
{
    //ui->graphicsView->scale(0.5, 0.5);
    ui->slider_zoom->setValue(50);
}

void CardPreview::resizeEvent(QResizeEvent *e)
{
    m_calculatedSize = e->size();
    QGraphicsView::resizeEvent(e);
}

#if QT_CONFIG(wheelevent)
void CardPreview::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->delta() > 0) {
            m_previewWidget->zoomIn(6);
        } else {
            m_previewWidget->zoomOut(-6);
        }
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}
#endif
