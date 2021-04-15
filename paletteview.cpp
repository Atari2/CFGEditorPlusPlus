#include "paletteview.h"

PaletteView::PaletteView(QGraphicsScene* ogscene) : QGraphicsView(ogscene) {
    setWindowTitle("Palette");
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, &QListView::customContextMenuRequested, this, [&](QPoint pos) {
        QMenu* menu = new QMenu(this);
        menu->addAction("&Load Palette from file", this, [&]() {
            QString filename = QFileDialog::getOpenFileName();
            if (filename.length() == 0)
                return;
            SpritePaletteCreator::ReadPaletteFile(0, 16, 16, filename);
            updateForChange(SpritePaletteCreator::MakeFullPalette());
            emit paletteChanged();
        });
        menu->addAction("&Save Palette to file", this, [&]() {
            QString filename = QFileDialog::getSaveFileName();
            if (filename.length() == 0)
                return;
            SpritePaletteCreator::PaletteToFile(currentItem->pixmap().toImage(), filename);
        });
        menu->popup(viewport()->mapToGlobal(pos));
    });
}

QPoint PaletteView::convertPointToTile(const QPointF& point) {
    int intx = point.toPoint().x();
    int inty = point.toPoint().y();
    // this might *seem* like a useless calculation but in reality without this it doesn't work properly
    // yay for rounding
    return QPoint(intx - (intx % 16), inty - (inty % 16));
}


void PaletteView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        emit customContextMenuRequested(event->position().toPoint());
        return;
    }
    QImage m = currentItem->pixmap().toImage();
    QPainter p{&m};
    p.setBrush(QBrush(Qt::BrushStyle::SolidPattern));
    QColor color = QColorDialog::getColor(m.pixelColor(event->position().toPoint()), this);
    if (!color.isValid())
        return;
    QPoint colorIndex = convertPointToTile(event->position());
    SpritePaletteCreator::changePaletteColor(color, colorIndex);
    emit paletteChanged();
    p.fillRect(QRect{colorIndex, QSize(16, 16)}, color);
    currentItem->setPixmap(QPixmap::fromImage(m));
    event->accept();
}

void PaletteView::open() {
    m_open = true;
    show();
    raise();
}

void PaletteView::updateForChange(const QPixmap& image) {
    if (currentItem) {
        scene()->removeItem(currentItem);
        currentItem->~QGraphicsPixmapItem();
    }
    currentItem = new QGraphicsPixmapItem(image);
    currentItem->setAcceptHoverEvents(true);
    currentItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    scene()->addItem(currentItem);
    setFixedSize(256, 128);
}
void PaletteView::closeEvent(QCloseEvent* event) {
    m_open = false;
    event->accept();
}

PaletteView::~PaletteView() {
    qDebug() << "PaletteView destructor called";
    delete currentItem;
}
