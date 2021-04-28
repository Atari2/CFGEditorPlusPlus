#include "map16provider.h"

Map16Provider::Map16Provider(QWidget* parent) : QLabel(parent)  {
    tileGrid = createGrid();
    base = createCurrent();
    setPixmap(overlay());
    setMargin(0);
    setFixedSize(208, 208);
    setMouseTracking(true);
}

int Map16Provider::mouseCoordinatesToTile(QPoint position) {
    return ((position.y() / 16) * 16) + (position.x() / 16);
}

TiledPosition& Map16Provider::findIndex(size_t index) {
    auto ret = std::find_if(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [index](TiledPosition& pos) {
        return pos.tid == index;
    });
    if (ret == m_tiles[currentIndex].end())
        return invalid;
    return *ret;
}

QPoint Map16Provider::alignToGrid(QPoint position, int size) {
    return QPoint(position.x() - (position.x() % size), position.y() - (position.y() % size));
}

QPixmap Map16Provider::drawSelectedTile() {
    QPixmap curr = overlay();
    QPainter p{&curr};
    QPen pen{Qt::white, 1, Qt::DotLine, Qt::SquareCap, Qt::BevelJoin};
    p.setPen(pen);
    auto& t = findIndex(currentSelected);
    p.drawRect(QRect{t.pos.x(), t.pos.y(), 16, 16});
    p.end();
    return curr;
}

void Map16Provider::mousePressEvent(QMouseEvent *event) {
    if (m_tiles.length() == 0)
        return;
    QPainter p{&displays[currentIndex]};
    if (event->button() == Qt::MouseButton::LeftButton) {
        // todo:
        // also make the selected tile move around? idk how to do that
        // will have to save offsets
        // also keep in mind the "text only" version
        // we also need to serialize the tiles + offsets in some way
        // save them to the sprite display if the user changes index?
        // gonna be complex
        // if the user changes back to us we need to bring back up the old one with the saved tiles
        // a matrix of pair of FullTile, QPoint could work
        auto ret = std::find_if(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [&](TiledPosition& pos) {
                return QRect{pos.pos.x(), pos.pos.y(), 16, 16}.contains(event->position().toPoint(), true);
            });
        if (ret == m_tiles[currentIndex].end())
            return;
        currentSelected = (*ret).tid;
        setPixmap(drawSelectedTile());
        currentlyPressed = true;
        qDebug() << "Left mouse button pressed";
        return;
    } else if (event->button() == Qt::MouseButton::RightButton) {
        qDebug() << "Right mouse button pressed";
        int size = static_cast<int>(selectorSize);
        QPoint aligned = alignToGrid(event->position().toPoint(), size);
        m_tiles[currentIndex].append({copiedTile->getType(), copiedTile->getTile(), aligned, 0, TiledPosition::unique_index++});
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawImage(QRect{aligned.x(), aligned.y(), copiedTile->size(), copiedTile->size()}, copiedTile->draw());
    }
    setPixmap(overlay());
    event->accept();
}

void Map16Provider::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::LeftButton) {
        currentlyPressed = false;
    }
}

void Map16Provider::mouseMoveEvent(QMouseEvent *event) {
    if (currentlyPressed) {
        qDebug() << "Do thing";
    }
    event->accept();
}

void Map16Provider::wheelEvent(QWheelEvent *event) {
    if (currentSelected == SIZE_MAX) {
        event->accept();
    } else {
        auto& t = findIndex(currentSelected);
        if (event->angleDelta().y() < 0 && t.zpos > INT_MIN) {
            t.zpos--;
            redraw();
        } else if (event->angleDelta().y() > 0 && t.zpos < INT_MAX){
            t.zpos++;
            redraw();
        }
    }
}

void Map16Provider::redraw() {
    QPainter p{&displays[currentIndex]};
    std::sort(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [](TiledPosition& lhs, TiledPosition& rhs) {
        return lhs.zpos < rhs.zpos;
    });
    qDebug() << "length " << m_tiles[currentIndex].length();
    for (auto& t : m_tiles[currentIndex]) {
        p.drawImage(QRect{t.pos.x(), t.pos.y(), 16, 16}, t.tile.getFullTile());
    }
    setPixmap(drawSelectedTile());
}

void Map16Provider::setCopiedTile(ClipboardTile& tile) {
    copiedTile = &tile;
}

ClipboardTile* Map16Provider::getCopiedTile() {
    return copiedTile;
}

QPixmap Map16Provider::createCurrent() {
    QImage img{208, 208, QImage::Format::Format_ARGB32};
    QPainter p{&img};
    p.fillRect(QRect{0, 0, 208, 208}, QBrush(QGradient(QGradient::AmourAmour)));
    p.end();
    return QPixmap::fromImage(img);
}

QPixmap Map16Provider::createGrid() {
    QImage img{208, 208, QImage::Format::Format_ARGB32};
    QPainter p{&img};
    QPen pen(Qt::lightGray, 0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    pen.setWidthF(0.5);
    p.setPen(pen);
    img.fill(qRgba(0, 0, 0, 0));
    int size = static_cast<int>(selectorSize);
    if (size != 8 && size != 16)
        return QPixmap::fromImage(img);
    for (int i = size; i < 208; i += size) {
        p.drawLine(0, i, 208, i);
        p.drawLine(i, 0, i, 208);
    }
    return QPixmap::fromImage(img);
}

QPixmap Map16Provider::overlay() {
    QImage img;
    if (currentIndex == -1) {
        img = base.toImage();
    } else {
        img = displays[currentIndex].toImage();
    }
    QPainter p{&img};
    p.drawImage(img.rect(), tileGrid.toImage(), tileGrid.rect());
    return QPixmap::fromImage(img);
}

const Map16Provider::DisplayTiles& Map16Provider::Tiles() {
    return m_tiles[currentIndex];
}

void Map16Provider::addDisplay(int index) {
    if (index == -1) {
        m_tiles.append(DisplayTiles());
        displays.append(overlay());
        setPixmap(displays.last());
        currentIndex++;
    }
    else {
        m_tiles.insert(index, DisplayTiles());
        displays.insert(index, overlay());
        setPixmap(displays[index]);
        currentIndex = index;
    }
}
void Map16Provider::removeDisplay(int index) {
    if (index == -1)
        index = currentIndex;
    displays.removeAt(index);
    m_tiles.removeAt(index);
    if (displays.length() == 0) {
        currentIndex = -1;
        setPixmap(overlay());
    }
}
void Map16Provider::changeDisplay(int index) {
    displays[currentIndex] = pixmap();
    currentIndex = index;
    setPixmap(displays[currentIndex]);
}

void Map16Provider::cloneDisplay(int index) {
    if (index == -1)
        index = currentIndex;
    displays[index] = pixmap();
    displays.insert(index, displays[index]);
    m_tiles.insert(index, m_tiles.last());
}
SizeSelector Map16Provider::getSelectorSize() {
    return selectorSize;
}
void Map16Provider::setSelectorSize(SizeSelector size) {
    selectorSize = size;
    tileGrid = createGrid();
    setPixmap(overlay());
}
