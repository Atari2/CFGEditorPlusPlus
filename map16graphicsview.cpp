#include "map16graphicsview.h"

TileInfo::TileInfo(quint16 info) {
    vflip = info & 0x8000;
    hflip = info & 0x4000;
    prio = info & 0x2000;
    pal = ((info & 0x1C00) >> 10) & 0xFF;
    tilenum = (info & 0x3FF);
}

FullTile::FullTile(quint16 tl, quint16 bl, quint16 tr, quint16 br) :
    topleft(tl),
    bottomleft(bl),
    topright(tr),
    bottomright(br)
{

}

QImage TileInfo::get8x8Tile() {
    auto ig = SnesGFXConverter::get8x8TileFromVect(tilenum, SpritePaletteCreator::getPalette(pal + 8));
    ig.mirror(hflip, vflip);
    return ig;
}

QImage FullTile::getFullTile() {
    QImage img{16, 16, QImage::Format::Format_RGB32};
    QPainter p{&img};
    p.drawImage(QRect{0, 0, 8, 8}, topleft.get8x8Tile());
    p.drawImage(QRect{0, 8, 8, 8}, bottomleft.get8x8Tile());
    p.drawImage(QRect{8, 0, 8, 8}, topright.get8x8Tile());
    p.drawImage(QRect{8, 8, 8, 8}, bottomright.get8x8Tile());
    p.end();
    return img;
}


Map16GraphicsView::Map16GraphicsView(QWidget* parent) : QGraphicsView(parent)
{
    setMouseTracking(true);
    setFixedSize(352, 352);
    setScene(new QGraphicsScene);
}

void Map16GraphicsView::readInternalMap16File() {
    struct {
        quint32 offset;
        quint32 size;
    } map16DataIndex;
    struct {
        quint32 tableOffset;
        quint32 tableSize;
        quint32 sizeX;
        quint32 sizeY;
    } tableInformation;
    QFile file{":/Resources/spriteMapData.map16"};
    file.open(QFile::OpenModeFlag::ReadOnly);
    QDataStream byteStream{file.readAll()};
    byteStream.setByteOrder(QDataStream::LittleEndian);
    byteStream.skipRawData(16);     // skip metadata
    byteStream >> tableInformation.tableOffset;
    byteStream >> tableInformation.tableSize;
    byteStream >> tableInformation.sizeX;
    byteStream >> tableInformation.sizeY;
    tiles.reserve(tableInformation.sizeY);
    qDebug() << "Table offset = " << tableInformation.tableOffset << " Table size: " << tableInformation.tableSize;
    qDebug() << "Size X = " << tableInformation.sizeX << " Size Y = " << tableInformation.sizeY;
    byteStream.skipRawData(12 + 0x14);     // skip base coords, and "various flags" and 0x14 unused bytes
    // should have read 0x40 bytes at this point
    byteStream.skipRawData(tableInformation.tableOffset - 0x40);       // skip directly to the data, we don't care about the comment field
    byteStream >> map16DataIndex.offset;
    byteStream >> map16DataIndex.size;
    qDebug() << "Map16 Data offset = " << map16DataIndex.offset << " Map16 Data size: " << map16DataIndex.size;
    byteStream.skipRawData(tableInformation.tableSize - 8);
    // we're at the data now, hopefully
    Q_ASSERT(map16DataIndex.size == (tableInformation.sizeX * tableInformation.sizeY * 4 * 2));
    byteStream.setByteOrder(QDataStream::LittleEndian);
    for (quint32 i = 0; i < tableInformation.sizeY; i++) {
        QVector<FullTile> subVector{};
        subVector.reserve(tableInformation.sizeY);
        for (quint32 j = 0; j < tableInformation.sizeX; j++) {
            quint16 tl, bl, tr, br;
            byteStream >> tl;
            byteStream >> bl;
            byteStream >> tr;
            byteStream >> br;
            subVector.append({tl, bl, tr, br});
        }
        tiles.append(subVector);
    }
    // we don't really care about the rest of the file, now we can draw
    QImage map16{(int)tableInformation.sizeX * 16, (int)tableInformation.sizeY * 16, QImage::Format::Format_RGB32};
    QPainter p{&map16};

    for (int i = 0; i < tiles.length(); i++) {
        for (int j = 0; j < tiles[i].length(); j++) {
            p.drawImage(QRect{j * 16, i * 16, 16, 16}, tiles[i][j].getFullTile());
        }
    }
    p.end();
    scene()->addPixmap(QPixmap::fromImage(map16).scaled(map16.size()));
}

int Map16GraphicsView::mouseCoordinatesToTile(QPoint position) {
    return ((position.y() / 16) * 16) + (position.x() / 16);
}

void Map16GraphicsView::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "Tile " << mouseCoordinatesToTile(event->position().toPoint());
    event->accept();
}

