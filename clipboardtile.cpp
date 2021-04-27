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


ClipboardTile::ClipboardTile() :
    type(TileType::Sixteen)
  , tile(0, 0, 0, 0)
  , quarter(0)
{

}

void ClipboardTile::update(const FullTile& tile) {
    type = TileType::Sixteen;
    this->tile = tile;
}
void ClipboardTile::update(const TileInfo& quarter) {
    type = TileType::Eight;
    this->quarter = quarter;
}

void ClipboardTile::update(const ClipboardTile& other) {
    type = other.type;
    tile = other.tile;
    quarter = other.quarter;
}

QImage ClipboardTile::draw(int size) {
    if (type == TileType::Eight) {
        return quarter.get8x8Scaled(size);
    } else {
        return tile.getScaled(size);
    }
}

int ClipboardTile::size() {
    return type == TileType::Eight ? 8 : 16;
}
