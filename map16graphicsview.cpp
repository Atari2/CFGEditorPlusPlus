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

void FullTile::SetPalette(int pal) {
    topleft.pal = pal;
    topright.pal = pal;
    bottomleft.pal = pal;
    bottomright.pal = pal;
}

void FullTile::FlipX() {
    auto temp1 = topleft;
    auto temp2 = bottomleft;
    topleft = topright;
    bottomleft = bottomright;
    topright = temp1;
    bottomright = temp2;
    topleft.hflip = !topleft.hflip;
    bottomleft.hflip = !bottomleft.hflip;
    topright.hflip = !topright.hflip;
    bottomright.hflip = !bottomright.hflip;
}

void FullTile::FlipY() {
    auto temp1 = topleft;
    auto temp2 = topright;
    topleft = bottomleft;
    topright = bottomright;
    bottomleft = temp1;
    bottomright = temp2;
    topleft.vflip = !topleft.vflip;
    bottomleft.vflip = !bottomleft.vflip;
    topright.vflip = !topright.vflip;
    bottomright.vflip = !bottomright.vflip;
}

QImage TileInfo::get8x8Tile() {
    auto ig = SnesGFXConverter::get8x8TileFromVect(tilenum, SpritePaletteCreator::getPalette(pal + 8));
    ig.mirror(hflip, vflip);
    return ig;
}

QImage TileInfo::get8x8Scaled(int width) {
    return get8x8Tile().scaledToWidth(width, Qt::SmoothTransformation);
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

QImage FullTile::getScaled(int width) {
    return getFullTile().scaledToWidth(width, Qt::SmoothTransformation);
}


Map16GraphicsView::Map16GraphicsView(QWidget* parent) : QGraphicsView(parent)
{
    currScene = new QGraphicsScene(this);
    setMouseTracking(true);
    verticalScrollBar()->setFixedWidth(16);
    setScene(currScene);
    QObject::connect(verticalScrollBar(), QOverload<int>::of(&QScrollBar::valueChanged), [&](int value) {
        int offset = value % CellSize();
        if (offset != 0)
            verticalScrollBar()->setValue(value - offset);
    });
}

int Map16GraphicsView::CellSize() {
    if (currType == SelectorType::Sixteen) {
        return imageWidth / 16;
    } else {
        return imageWidth / 32;
    }
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
    TileMap = TileMap.scaledToWidth(352, Qt::SmoothTransformation);
    imageWidth = TileMap.width();
    imageHeight = TileMap.height();
    // and now we draw the grid and the page separator
    Grid = QImage{imageWidth, imageHeight, QImage::Format::Format_ARGB32};
    Grid.fill(qRgba(0, 0, 0, 0));
    QPainter gridPainter{&Grid};
    QPen pen(Qt::white, 1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    gridPainter.setPen(pen);
    for (int i = 0; i < tiles.length(); i++) {
        gridPainter.drawLine(0, i * CellSize(), imageWidth, i * CellSize());
        gridPainter.drawLine(i * CellSize(), 0, i * CellSize(), imageHeight);
    }
    gridPainter.end();

    PageSep = QImage{imageWidth, imageHeight, QImage::Format::Format_ARGB32};
    PageSep.fill(qRgba(0, 0, 0, 0));
    QPainter pageSepPainter{&PageSep};
    QPen penSep(Qt::blue, 2, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    pageSepPainter.setPen(penSep);

    for (int i = 0; i <= imageHeight; i += CellSize() * 16) {
        qDebug() << imageHeight << " " << i;
        pageSepPainter.drawRect(QRect{0, i, imageWidth, CellSize() * 16});
    }
    currentMap16 = scene()->addPixmap(QPixmap::fromImage(TileMap));
    setFixedWidth(currentMap16->pixmap().width() + 19);
    currentWithNoHighlight = currentMap16->pixmap();
    currentWithNoSelection = currentMap16->pixmap();
}

// todo: this apparently doesn't take into account the scrollbar
// very nice
// update: this very much takes into account the scrollbar, maybe it does way too much
void Map16GraphicsView::drawCurrentSelectedTile(QPixmap& map) {
    if (currentClickedTile == -1)
        return;
    QPainter p{&map};
    QPen pen{Qt::white, 1, Qt::DotLine, Qt::SquareCap, Qt::RoundJoin};
    p.setPen(pen);
    p.drawRect(QRect{currentTopLeftClicked.x(), currentTopLeftClicked.y(), CellSize(), CellSize()});
    p.end();
}

void Map16GraphicsView::addGrid() {
    if (useGrid)
        return;
    useGrid = true;
    QPixmap pixmap = currentWithNoSelection;
    QPainter paintGrid{&pixmap};
    paintGrid.drawImage(QRect{0, 0, imageWidth, imageHeight}, Grid);
    paintGrid.end();
    currentWithNoSelection = pixmap;
    drawCurrentSelectedTile(pixmap);
    currentWithNoHighlight = pixmap;
    currentMap16->setPixmap(pixmap);
}

void Map16GraphicsView::removeGrid() {
    if (!useGrid)
        return;
    useGrid = false;
    QPixmap pixmap = QPixmap::fromImage(TileMap);
    if (usePageSep) {
        QPainter paint{&pixmap};
        // if the page separator is being used, we have to draw it
        paint.drawImage(QRect{0, 0, imageWidth, imageHeight}, PageSep);
    }
    currentWithNoSelection = pixmap;
    drawCurrentSelectedTile(pixmap);
    currentWithNoHighlight = pixmap;
    currentMap16->setPixmap(pixmap);
}

void Map16GraphicsView::addPageSep() {
    if (usePageSep)
        return;
    usePageSep = true;
    QPixmap pixmap = currentWithNoSelection;
    QPainter paintPageSep{&pixmap};
    paintPageSep.drawImage(QRect{0, 0, imageWidth, imageHeight}, PageSep);
    paintPageSep.end();
    currentWithNoSelection = pixmap;
    drawCurrentSelectedTile(pixmap);
    currentWithNoHighlight = pixmap;
    currentMap16->setPixmap(pixmap);
}

void Map16GraphicsView::removePageSep() {
    if (!usePageSep)
        return;
    usePageSep = false;
    QPixmap pixmap = QPixmap::fromImage(TileMap);
    if (useGrid) {
        QPainter paint{&pixmap};
        // if the grid is being used, we have to draw it
        paint.drawImage(QRect{0, 0, imageWidth, imageHeight}, Grid);
    }
    currentWithNoSelection = pixmap;
    drawCurrentSelectedTile(pixmap);
    currentWithNoHighlight = pixmap;
    currentMap16->setPixmap(pixmap);
}
int Map16GraphicsView::mouseCoordinatesToTile(QPoint position) {
    int diff = CellSize();
    int tilePerRow = currType == SelectorType::Sixteen ? 16 : 32;
    int scrollbary = verticalScrollBar()->sliderPosition() / diff;
    return (((position.y() / diff) + scrollbary) * tilePerRow) + (position.x() / diff);
}

QPoint Map16GraphicsView::translateToRect(QPoint position) {
    int size = CellSize();
    int scrollbarDiff = verticalScrollBar()->sliderPosition() - (verticalScrollBar()->sliderPosition() % size);
    return QPoint(
                position.x() - (position.x() % size),
                position.y() - (position.y() % size) + scrollbarDiff
                );
}

FullTile& Map16GraphicsView::tileNumToTile() {
    int realClickedTile = currType == SelectorType::Sixteen ? currentClickedTile : currentClickedTile >> 2;
    int row = realClickedTile / (imageWidth / 22);
    int col = realClickedTile % (imageWidth / 22);
    qDebug() << row << " " << col << " " << imageWidth;
    return tiles[row][col];
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
    auto map = currentWithNoHighlight;
    QPainter paint{&map};
    paint.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    paint.fillRect(QRect{origin.x(), origin.y(), CellSize(), CellSize()}, QBrush(QColor(0,0,0,128)));
    paint.end();
    currentMap16->setPixmap(map);
    event->accept();
}

void Map16GraphicsView::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse" << (event->button() == Qt::LeftButton ? "left click" : "right click") << "was pressed";
    if (event->button() == Qt::RightButton)
        // todo: implement right button => save selected tile to ""clipboard""
        return;
    if (currentTile == -1)
        return;
    currentClickedTile = currentTile;
    currentTopLeftClicked = translateToRect(event->position().toPoint());
    clickCallback(tileNumToTile(), currentTile, currType);
    QPixmap selected = currentWithNoSelection;
    drawCurrentSelectedTile(selected);
    currentMap16->setPixmap(selected);
    currentWithNoHighlight = selected;
    event->accept();
}

void Map16GraphicsView::registerMouseClickCallback(const std::function<void(FullTile, int, SelectorType)>& callback) {
    clickCallback = callback;
}

void Map16GraphicsView::useGridChanged() {
    useGrid ? removeGrid() : addGrid();
}
void Map16GraphicsView::usePageSepChanged() {
    usePageSep ? removePageSep() : addPageSep();
}

TileChangeType Map16GraphicsView::getChangeType() {
    if (currType == SelectorType::Sixteen)
        return TileChangeType::All;
    int nRow = currentTopLeftClicked.y() / CellSize();
    int nCol = currentTopLeftClicked.x() / CellSize();
    if (nCol % 2 == 0 && nRow % 2 == 0)
        return TileChangeType::TopLeft;
    else if (nCol % 2 == 1 && nRow % 2 == 0)
        return TileChangeType::TopRight;
    else if (nCol % 2 == 0 && nRow % 2 == 1)
        return TileChangeType::BottomLeft;
    else
        return TileChangeType::BottomRight;
}

void Map16GraphicsView::changePaletteIndex(QComboBox* box, FullTile tileInfo) {
    if (currType == SelectorType::Sixteen) {
        if ((tileInfo.bottomleft.pal == tileInfo.bottomright.pal)
                && (tileInfo.bottomleft.pal == tileInfo.topleft.pal)
                && (tileInfo.bottomleft.pal == tileInfo.topright.pal)) {
            box->setCurrentIndex(tileInfo.bottomleft.pal);
        } else {
            box->setCurrentIndex(8);
        }
    } else {
        auto ac = getChangeType();
        switch(ac) {
        case TileChangeType::TopLeft:
            box->setCurrentIndex(tileInfo.topleft.pal);
            break;
        case TileChangeType::BottomLeft:
            box->setCurrentIndex(tileInfo.bottomleft.pal);
            break;
        case TileChangeType::TopRight:
            box->setCurrentIndex(tileInfo.topright.pal);
            break;
        case TileChangeType::BottomRight:
            box->setCurrentIndex(tileInfo.bottomright.pal);
            break;
        default:
            Q_ASSERT(false);
        }
    }
}

void Map16GraphicsView::tileChanged(QObject* toBlock, TileChangeAction action, TileChangeType type, int value) {
    if (noSignals)
        return;
    if (
            currentClickedTile == -1 ||
            ((currType == SelectorType::Sixteen && currentClickedTile < 0x300) ||
             (currType == SelectorType::Eight && currentClickedTile < 0xC00))
        )
        return;
    QSignalBlocker block{toBlock};
    FullTile& tile = tileNumToTile();
    TileInfo* partial = nullptr;
    switch (type) {
    case TileChangeType::BottomLeft:
       partial = &tile.bottomleft;
       break;
    case TileChangeType::BottomRight:
       partial = &tile.bottomright;
       break;
    case TileChangeType::TopLeft:
       partial = &tile.topleft;
       break;
    case TileChangeType::TopRight:
       partial = &tile.topright;
       break;
    default:
       break;
    }

    if (partial != nullptr) {
        switch (action) {
        case TileChangeAction::Number:
            partial->tilenum = (value & 0x37F);
            break;
        case TileChangeAction::Palette:
            partial->pal = value;
            break;
        case TileChangeAction::FlipX:
            partial->vflip = !partial->vflip;
            break;
        case TileChangeAction::FlipY:
            partial->hflip = !partial->hflip;
            break;
        }
    } else {
        switch (action) {
        case TileChangeAction::Palette:
            tile.SetPalette(value);
            break;
        case TileChangeAction::FlipX:
            tile.FlipX();
            break;
        case TileChangeAction::FlipY:
            tile.FlipY();
            break;
        default:
            Q_ASSERT(false);
        }
    }

    QPainter og{&TileMap};
    QPainter high{&currentWithNoHighlight};
    QPainter sel{&currentWithNoSelection};
    auto size = CellSize();
    if (currType == SelectorType::Sixteen || partial == nullptr) {
        auto img = tile.getFullTile();
        high.drawImage(QRect{(currentClickedTile % 16) * size, (currentClickedTile / 16) * size, size, size}, img);
        sel.drawImage(QRect{(currentClickedTile % 16) * size, (currentClickedTile / 16) * size, size, size}, img);
        og.drawImage(QRect{(currentClickedTile % 16) * size, (currentClickedTile / 16) * size, size, size}, img);
    }
    else {
        auto img = partial->get8x8Tile();
        high.drawImage(QRect{(currentClickedTile % 32) * size, (currentClickedTile / 32) * size, size, size}, img);
        sel.drawImage(QRect{(currentClickedTile % 32) * size, (currentClickedTile / 32) * size, size, size}, img);
        og.drawImage(QRect{(currentClickedTile % 32) * size, (currentClickedTile / 32) * size, size, size}, img);
    }
    sel.end();
    high.end();
    og.end();
    drawCurrentSelectedTile(currentWithNoHighlight);
    currentMap16->setPixmap(currentWithNoHighlight);
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
