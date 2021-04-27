#ifndef CLIPBOARDTILE_H
#define CLIPBOARDTILE_H
#include <QImage>
#include "snesgfxconverter.h"
#include "spritepalettecreator.h"

struct TileInfo {
    TileInfo(quint16 tile);
    bool vflip;
    bool hflip;
    bool prio;
    quint8 pal;
    quint16 tilenum;
    QImage get8x8Tile();
    QImage get8x8Scaled(int width);
};

struct FullTile {
    FullTile(quint16 tl, quint16 bl, quint16 tr, quint16 br);
    TileInfo topleft;
    TileInfo bottomleft;
    TileInfo topright;
    TileInfo bottomright;
    QImage getFullTile();
    QImage getScaled(int width);
    void SetPalette(int pal);
    void FlipX();
    void FlipY();
};

enum class TileType {
    Sixteen,
    Eight
};

class ClipboardTile
{
private:
    TileType type;
    FullTile tile;
    TileInfo quarter;
public:
    ClipboardTile();
    void update(const FullTile& tile);
    void update(const TileInfo& quarter);
    void update(const ClipboardTile& other);
    QImage draw(int size);
    int size();
};

#endif // CLIPBOARDTILE_H
