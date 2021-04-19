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
    currScene = new QGraphicsScene(this);
    setMouseTracking(true);
    setFixedSize(352, 352);
    setScene(currScene);
    QObject::connect(verticalScrollBar(), QOverload<int>::of(&QScrollBar::valueChanged), [&](int value) {
        int offset = value % static_cast<int>(currType);
        if (offset != 0)
            verticalScrollBar()->setValue(value);
    });
}

void Map16GraphicsView::setControllingLabel(QLabel *tileNoLabel) {
    tileNumLabel = tileNoLabel;
}

void Map16GraphicsView::readInternalMap16File(const QString& name) {
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
    QFile file{name};
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
    for (quint32 i = 0; i < tableInformation.sizeX; i++) {
        QVector<FullTile> subVector{};
        subVector.reserve(16);
        for (quint32 j = 0; j < 16; j++)
            subVector.append({0x00, 0x00, 0x00, 0x00});
        tiles.append(subVector);
    }
    imageWidth = (int)tableInformation.sizeX * 16;
    imageHeight = ((int)tableInformation.sizeY + 16) * 16;
    // we don't really care about the rest of the file, now we can draw
    drawInternalMap16File();
}

void Map16GraphicsView::readExternalMap16File(const QString &name) {
    readInternalMap16File(name);
}

void Map16GraphicsView::drawInternalMap16File() {
    TileMap = QImage{imageWidth, imageHeight, QImage::Format::Format_ARGB32};
    QPainter p{&TileMap};
    for (int i = 0; i < tiles.length(); i++) {
        for (int j = 0; j < tiles[i].length(); j++) {
            p.drawImage(QRect{j * 16, i * 16, 16, 16}, tiles[i][j].getFullTile());
        }
    }
    p.end();

    // and now we draw the one with the grid
    Grid = QImage{imageWidth, imageHeight, QImage::Format::Format_ARGB32};
    Grid.fill(qRgba(0, 0, 0, 0));
    QPainter gridPainter{&Grid};
    QPen pen(Qt::white, 1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    gridPainter.setPen(pen);
    for (int i = 0; i < tiles.length(); i++) {
        gridPainter.drawLine(0, i * 16, imageWidth, i * 16);
        gridPainter.drawLine(i * 16, 0, i * 16, imageHeight);
    }
    gridPainter.end();

    PageSep = QImage{imageWidth, imageHeight, QImage::Format::Format_ARGB32};
    PageSep.fill(qRgba(0, 0, 0, 0));
    QPainter pageSepPainter{&PageSep};
    QPen penSep(Qt::blue, 2, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    pageSepPainter.setPen(penSep);

    for (int i = 0; i <= imageHeight; i += 0x100) {
        qDebug() << imageHeight << " " << i;
        pageSepPainter.drawRect(QRect{0, i, imageWidth, 0x100});
    }

    currentMap16 = scene()->addPixmap(QPixmap::fromImage(TileMap) /* .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation) */);
    setFixedWidth(imageWidth + 20);
    hasAlreadyDrawnOnce = false;
}

void Map16GraphicsView::addGrid() {
    if (useGrid)
        return;
    useGrid = true;
    hasAlreadyDrawnOnce = true;
    QPixmap pixmap = currentMap16->pixmap();
    QPainter paintGrid{&pixmap};
    paintGrid.drawImage(QRect{0, 0, imageWidth, imageHeight}, Grid);
    paintGrid.end();
    currentWithNoHighlight = pixmap;
    currentMap16->setPixmap(pixmap);
}

void Map16GraphicsView::removeGrid() {
    if (!useGrid)
        return;
    useGrid = false;
    QPixmap pixmap = QPixmap::fromImage(TileMap);
    QPainter paint{&pixmap};
    if (usePageSep) {
        // if the page separator is being used, we have to draw it
        paint.drawImage(QRect{0, 0, imageWidth, imageHeight}, PageSep);
    }
    currentMap16->setPixmap(pixmap);
    currentWithNoHighlight = pixmap;
}

void Map16GraphicsView::addPageSep() {
    if (usePageSep)
        return;
    usePageSep = true;
    hasAlreadyDrawnOnce = true;
    QPixmap pixmap = currentWithNoHighlight;
    QPainter paintPageSep{&pixmap};
    paintPageSep.drawImage(QRect{0, 0, imageWidth, imageHeight}, PageSep);
    paintPageSep.end();
    currentWithNoHighlight = pixmap;
    currentMap16->setPixmap(pixmap);
}

void Map16GraphicsView::removePageSep() {
    if (!usePageSep)
        return;
    usePageSep = false;
    QPixmap pixmap = QPixmap::fromImage(TileMap);
    QPainter paint{&pixmap};
    if (useGrid) {
        // if the grid is being used, we have to draw it
        paint.drawImage(QRect{0, 0, imageWidth, imageHeight}, Grid);
    }
    currentMap16->setPixmap(pixmap);
    currentWithNoHighlight = pixmap;
}
int Map16GraphicsView::mouseCoordinatesToTile(QPoint position) {
    int diff = static_cast<int>(currType);
    int tilePerRow = diff == 16 ? 16 : 32;
    int scrollbary = verticalScrollBar()->sliderPosition() / diff;
    return (((position.y() / diff) + scrollbary) * tilePerRow) + (position.x() / diff);
}

QPoint Map16GraphicsView::translateToRect(QPoint position) {
    int scrollbarDiff = verticalScrollBar()->sliderPosition() - (verticalScrollBar()->sliderPosition() % static_cast<int>(currType));
    return QPoint(
                position.x() - (position.x() % static_cast<int>(currType)),
                position.y() - (position.y() % static_cast<int>(currType)) + scrollbarDiff
                );
}

void Map16GraphicsView::mouseMoveEvent(QMouseEvent *event) {
    currentTile = mouseCoordinatesToTile(event->position().toPoint());
    auto origin = translateToRect(event->position().toPoint());
    if (currentTile == previousTile)
        return;
    previousTile = currentTile;
    QString tileText = QString::asprintf("Tile: 0x%03X", currentTile);
    tileNumLabel->setText(tileText);
    // highlight the tile in some way
    if (!hasAlreadyDrawnOnce) {
        currentWithNoHighlight = currentMap16->pixmap();
        hasAlreadyDrawnOnce = true;
    }
    auto map = currentWithNoHighlight;
    QPainter paint{&map};
    paint.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    paint.fillRect(QRect{origin.x(), origin.y(), static_cast<int>(currType), static_cast<int>(currType)}, QBrush(QColor(0,0,0,128)));
    paint.end();
    currentMap16->setPixmap(map);
    event->accept();
}

void Map16GraphicsView::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse" << (event->button() == Qt::LeftButton ? "left click" : "right click") << "was pressed";
    if (currentTile == -1)
        return;
    int intCurrType = static_cast<int>(currType);
    clickCallback(tiles[currentTile / intCurrType][currentTile % intCurrType], currentTile, intCurrType);
    event->accept();
}

void Map16GraphicsView::registerMouseClickCallback(const std::function<void(FullTile, int, int)>& callback) {
    clickCallback = callback;
}

void Map16GraphicsView::useGridChanged() {
    useGrid ? removeGrid() : addGrid();
}
void Map16GraphicsView::usePageSepChanged() {
    usePageSep ? removePageSep() : addPageSep();
}

void Map16GraphicsView::switchCurrSelectionType() {
    if (currType == SelectorType::Eight) {
        currType = SelectorType::Sixteen;
    } else {
        currType = SelectorType::Eight;
    }
}

Map16GraphicsView::~Map16GraphicsView() {
    qDebug() << "Map16 GraphicsView Destructor called";
    if (currScene)
        delete currScene;
}
