#include "eightbyeightview.h"

EightByEightView::EightByEightView(QGraphicsScene* ogscene) : QGraphicsView(ogscene) {
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setWindowTitle("8x8 Tile Viewer");
    verticalScrollBar()->setRange(0, 16);
    QObject::connect(verticalScrollBar(), QOverload<int>::of(&QScrollBar::valueChanged), [&](int value) {
        const int step = 16;
        int offset = value % step;
        if (offset != 0)
            verticalScrollBar()->setValue(value - offset);
    });
}

int EightByEightView::convertPointToTile(const QPointF& point) {
    int intx = (int)point.x();
    int inty = (int)point.y();
    int scrollbary = (int)verticalScrollBar()->sliderPosition();
    // this might *seem* like a useless calculation but in reality without this it doesn't work properly
    // yay for rounding
    return  (((inty / 16) * 16) + ((scrollbary / 16) * 16)) + (intx / 16);
}

void EightByEightView::mouseMoveEvent(QMouseEvent *event) {
    if (!m_open || !hasFocus()) {
        event->ignore();
        return;
    }
    // this is probably a bad idea but it's the only idea I have
    QToolTip::showText(event->globalPosition().toPoint(), QString::asprintf("Tile: %03X", convertPointToTile(event->position())), this, rect());
}

void EightByEightView::open() {
    m_open = true;
    show();
    raise();
}
void EightByEightView::updateForChange(QImage* image) {
    if (currentItem) {
        scene()->removeItem(currentItem);
        currentItem->~QGraphicsPixmapItem();
    }
    currentItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image).scaled(image->size() * 2));
    currentItem->setAcceptHoverEvents(true);
    scene()->addItem(currentItem);
    setFixedSize(275, 256);
}
void EightByEightView::closeEvent(QCloseEvent* event) {
    m_open = false;
    event->accept();
}

EightByEightView::~EightByEightView() {
    qDebug() << "EightbyEight destructor called";
    delete currentItem;
}
