#include "clipboardtile.h"
#include <QFileSystemWatcher>


TileInfo::TileInfo(quint16 info) {
    vflip = info & 0x8000;
    hflip = info & 0x4000;
    prio = info & 0x2000;
    pal = ((info & 0x1C00) >> 10) & 0xFF;
    tilenum = (info & 0x3FF);
}

FullTile::FullTile(quint16 tl, quint16 bl, quint16 tr, quint16 br, TileChangeType type) :
    topleft(tl),
    bottomleft(bl),
    topright(tr),
    bottomright(br)
{
    switch (type) {
    case TileChangeType::BottomLeft:
        topleft.is_this_tile = false;
        topright.is_this_tile = false;
        bottomright.is_this_tile = false;
        break;
    case TileChangeType::TopLeft:
        bottomleft.is_this_tile = false;
        topright.is_this_tile = false;
        bottomright.is_this_tile = false;
        break;
    case TileChangeType::BottomRight:
        topleft.is_this_tile = false;
        topright.is_this_tile = false;
        bottomleft.is_this_tile = false;
        break;
    case TileChangeType::TopRight:
        topleft.is_this_tile = false;
        bottomleft.is_this_tile = false;
        bottomright.is_this_tile = false;
        break;
    default:
        break;
    }
}

FullTile::FullTile(TileInfo tl, TileInfo bl, TileInfo tr, TileInfo br, TileChangeType type) :
    topleft(tl),
    bottomleft(bl),
    topright(tr),
    bottomright(br)
{
    switch (type) {
    case TileChangeType::BottomLeft:
        topleft.is_this_tile = false;
        topright.is_this_tile = false;
        bottomright.is_this_tile = false;
        break;
    case TileChangeType::TopLeft:
        bottomleft.is_this_tile = false;
        topright.is_this_tile = false;
        bottomright.is_this_tile = false;
        break;
    case TileChangeType::BottomRight:
        topleft.is_this_tile = false;
        topright.is_this_tile = false;
        bottomleft.is_this_tile = false;
        break;
    case TileChangeType::TopRight:
        topleft.is_this_tile = false;
        bottomleft.is_this_tile = false;
        bottomright.is_this_tile = false;
        break;
    default:
        break;
    }
}

void FullTile::SetPalette(int pal) {
    topleft.pal = pal;
    topright.pal = pal;
    bottomleft.pal = pal;
    bottomright.pal = pal;
}

void FullTile::SetOffset(int off) {
    offset = off;
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

QImage TileInfo::get8x8Tile(int offset) {
    if (!isThisTile()) {
        QImage tile(8, 8, QImage::Format::Format_ARGB32);
        tile.fill(Qt::transparent);
        return tile;
    }
    QImage ig;
    if (offset == -1)
        ig = SnesGFXConverter::get8x8TileFromVect(tilenum, SpritePaletteCreator::getPalette(pal + 8));
    else
        ig = SnesGFXConverter::get8x8TileFromExternal(tilenum, SpritePaletteCreator::getPalette(pal + 8), offset);
    ig.mirror(hflip, vflip);
    return ig;
}

QImage TileInfo::get8x8Scaled(int width, int offset) {
    return get8x8Tile(offset).scaledToWidth(width, Qt::SmoothTransformation);
}


bool TileInfo::isEmpty() {
    quint16 val = 0;
    val |= (vflip ? 0x8000 : 0);
    val |= (hflip ? 0x4000 : 0);
    val |= (prio ? 0x2000 : 0);
    val |= ((pal & 7)  << 10);
    val |= (tilenum & 0x3FF);
    return val == 0;
}

bool TileInfo::isThisTile() {
    return is_this_tile;
}

quint16 TileInfo::TileValue() {
    quint16 val = 0;
    val |= (vflip ? 0x8000 : 0);
    val |= (hflip ? 0x4000 : 0);
    val |= (prio ? 0x2000 : 0);
    val |= ((pal & 7)  << 10);
    val |= (tilenum & 0x3FF);
    return val;
}

QImage FullTile::getFullTile() {
    if (isFullTile()) {
        QImage img{16, 16, QImage::Format::Format_ARGB32};
        img.fill(Qt::transparent);
        QPainter p{&img};
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawImage(QRect{0, 0, 8, 8}, topleft.get8x8Tile(offset));
        p.drawImage(QRect{0, 8, 8, 8}, bottomleft.get8x8Tile(offset));
        p.drawImage(QRect{8, 0, 8, 8}, topright.get8x8Tile(offset));
        p.drawImage(QRect{8, 8, 8, 8}, bottomright.get8x8Tile(offset));
        p.end();
        return img;
    } else {
        QImage img{8, 8, QImage::Format::Format_ARGB32};
        img.fill(Qt::transparent);
        QPainter p{&img};
        QRect area{0, 0, 8, 8};
        if (topleft.isThisTile()) {
            p.drawImage(area, topleft.get8x8Tile(offset));
        } else if (topright.isThisTile()) {
            p.drawImage(area, topright.get8x8Tile(offset));
        } else if (bottomleft.isThisTile()) {
            p.drawImage(area, bottomleft.get8x8Tile(offset));
        } else if (bottomright.isThisTile()) {
            p.drawImage(area, bottomright.get8x8Tile(offset));
        }
        p.end();
        return img;
    }
}

QImage FullTile::getScaled(int width) {
    return getFullTile().scaledToWidth(width, Qt::SmoothTransformation);
}

bool FullTile::isEmpty() {
    return topleft.isEmpty() && topright.isEmpty() && bottomleft.isEmpty() && topleft.isEmpty();
}

bool FullTile::isFullTile() {
    return topleft.isThisTile() && topright.isThisTile() && bottomleft.isThisTile() && bottomright.isThisTile();
}

ClipboardTile::ClipboardTile() :
    valid(false)
  , tile(0, 0, 0, 0)
{

}

void ClipboardTile::update(const FullTile& tile) {
    valid = true;
    this->tile = tile;
}

void ClipboardTile::update(const ClipboardTile& other) {
    valid = other.valid;
    map16tile = other.map16tile;
    tile = other.tile;
}

QImage ClipboardTile::draw() {
    return tile.getFullTile();
}

int ClipboardTile::size() {
    return 16;
}

FullTile ClipboardTile::getTile() {
    return tile;
}

bool ClipboardTile::isValid() {
    return valid;
}

void ClipboardTile::setTileNum(int num) {
    map16tile = num;
}

int ClipboardTile::TileNum() {
    return map16tile;
}


void FullTile::setTileInfoByType(TileInfo info, TileChangeType type) {
    switch (type) {
    case TileChangeType::BottomLeft:
        bottomleft = info;
        break;
    case TileChangeType::TopLeft:
        topleft = info;
        break;
    case TileChangeType::BottomRight:
        bottomright = info;
        break;
    case TileChangeType::TopRight:
        topright = info;
        break;
    default:
        break;
    }
}

TileInfo FullTile::getTileInfoByType(TileChangeType type) {
    switch (type) {
    case TileChangeType::BottomLeft:
        return bottomleft;
    case TileChangeType::TopLeft:
        return topleft;
    case TileChangeType::BottomRight:
        return bottomright;
    case TileChangeType::TopRight:
        return topright;
    default:
        break;
    }
    return 0;
}

QImage FullTile::getPartialTile(TileChangeType type) {
    switch (type) {
    case TileChangeType::BottomLeft:
        return bottomleft.get8x8Tile(offset);
    case TileChangeType::TopLeft:
        return topleft.get8x8Tile(offset);
    case TileChangeType::BottomRight:
        return bottomright.get8x8Tile(offset);
    case TileChangeType::TopRight:
        return topright.get8x8Tile(offset);
    default:
        break;
    }
    return QImage{};
}
