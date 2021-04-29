#ifndef CLIPBOARDTILE_H
#define CLIPBOARDTILE_H
#include <QImage>
#include "snesgfxconverter.h"
#include "spritepalettecreator.h"

struct TileInfo {
    TileInfo(quint16 tile);
    TileInfo() = default;
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
    FullTile(TileInfo tl, TileInfo bl, TileInfo tr, TileInfo br);
    FullTile() = default;
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

enum class TileChangeType {
    All,
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight
};

class ClipboardTile
{
private:
    bool valid;
    TileChangeType type;
    FullTile tile;
    TileInfo quarter;
public:
    ClipboardTile();
    void update(const FullTile& tile);
    void update(const TileInfo& quarter, TileChangeType type);
    void update(const ClipboardTile& other);
    QImage draw();
    FullTile getTile();
    int size();
    TileChangeType getType();
    bool isValid();
};

#endif // CLIPBOARDTILE_H
