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

void Map16Provider::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "Tile " << mouseCoordinatesToTile(event->position().toPoint());
    event->accept();
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
