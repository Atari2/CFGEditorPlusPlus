#include "map16provider.h"

Map16Provider::Map16Provider(QWidget* parent) : QLabel(parent)  {
    setPixmap(createRedGrid());
    setMargin(0);
    setFixedSize(208, 208);
    setMouseTracking(true);
}

int Map16Provider::mouseCoordinatesToTile(QPoint position) {
    return ((position.y() / 16) * 16) + (position.x() / 16);
}

QPoint Map16Provider::alignToGrid(QPoint position, int size) {
    return QPoint(position.x() - (position.x() % size), position.y() - (position.y() % size));
}

void Map16Provider::mousePressEvent(QMouseEvent *event) {
    QPixmap map = pixmap();
    QPainter p{&map};
    if (event->button() == Qt::MouseButton::LeftButton) {
        qDebug() << "Left mouse button pressed";
        return;
    } else if (event->button() == Qt::MouseButton::RightButton) {
        qDebug() << "Right mouse button pressed";
        int size = copiedTile->size();
        QPoint aligned = alignToGrid(event->position().toPoint(), size);
        p.drawImage(QRect{aligned.x(), aligned.y(), size, size}, copiedTile->draw(size));
    }
    setPixmap(map);
    event->accept();
}

void Map16Provider::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << QString::asprintf("Tile %02X", mouseCoordinatesToTile(event->position().toPoint()));
    event->accept();
}

void Map16Provider::setCopiedTile(ClipboardTile& tile) {
    copiedTile = &tile;
}

ClipboardTile* Map16Provider::getCopiedTile() {
    return copiedTile;
}

QPixmap Map16Provider::createRedGrid() {
    QImage img{208, 208, QImage::Format::Format_RGB32};
    QPainter p{&img};
    QPen pen(Qt::white, 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
    p.fillRect(QRect{0, 0, 208, 208}, QBrush(QGradient(QGradient::AmourAmour)));
    p.setPen(pen);
    for (int i = 16; i < 208; i += 16) {
        p.drawLine(0, i, 208, i);
        p.drawLine(i, 0, i, 208);
    }
    return QPixmap::fromImage(img);
}
