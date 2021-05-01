#include "clipboardtile.h"


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

FullTile::FullTile(TileInfo tl, TileInfo bl, TileInfo tr, TileInfo br) :
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

bool TileInfo::isEmpty() {
    quint16 val = 0;
    val |= (vflip ? 0x8000 : 0);
    val |= (hflip ? 0x4000 : 0);
    val |= (prio ? 0x2000 : 0);
    val |= ((pal & 7)  << 10);
    val |= (tilenum & 0x3FF);
    return val == 0;
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
    QImage img{16, 16, QImage::Format::Format_ARGB32};
    img.fill(Qt::transparent);
    QPainter p{&img};
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
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

bool FullTile::isEmpty() {
    return topleft.isEmpty() && topright.isEmpty() && bottomleft.isEmpty() && topleft.isEmpty();
}

ClipboardTile::ClipboardTile() :
    valid(false)
  , type(TileChangeType::All)
  , tile(0, 0, 0, 0)
  , quarter(0)
{

}

void ClipboardTile::update(const FullTile& tile) {
    valid = true;
    type = TileChangeType::All;
    this->tile = tile;
}
void ClipboardTile::update(const TileInfo& quarter, TileChangeType type) {
    valid = true;
    this->type = type;
    this->quarter = quarter;
}

void ClipboardTile::update(const ClipboardTile& other) {
    valid = true;
    type = other.type;
    tile = other.tile;
    quarter = other.quarter;
}

QImage ClipboardTile::draw() {
    if (type != TileChangeType::All) {
        return quarter.get8x8Tile();
    } else {
        return tile.getFullTile();
    }
}

int ClipboardTile::size() {
    return type == TileChangeType::All ? 16 : 8;
}

TileChangeType ClipboardTile::getType() {
    return type;
}

FullTile ClipboardTile::getTile() {
    switch (type) {
    case TileChangeType::All:
        return tile;
    case TileChangeType::BottomLeft:
        return FullTile{0, quarter, 0, 0};
    case TileChangeType::BottomRight:
        return FullTile{0, 0, 0, quarter};
    case TileChangeType::TopLeft:
        return FullTile{quarter, 0, 0, 0};
    case TileChangeType::TopRight:
        return FullTile{0, 0, quarter, 0};
    default:
        Q_ASSERT(false);
    }
    return {0, 0, 0, 0};
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
