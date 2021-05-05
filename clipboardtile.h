#ifndef CLIPBOARDTILE_H
#define CLIPBOARDTILE_H
#include <QImage>
#include "snesgfxconverter.h"
#include "spritepalettecreator.h"

struct ExternalGfxInfo {
    qsizetype start;
    qsizetype end;
    int basetile;
};

struct TileInfo {
    TileInfo(quint16 tile);
    TileInfo() = default;
    bool vflip;
    bool hflip;
    bool prio;
    quint8 pal;
    quint16 tilenum;
    QImage get8x8Tile(int offset);
    QImage get8x8Scaled(int width, int offset);
    bool isEmpty();
    quint16 TileValue();
};

struct FullTile {
    FullTile(quint16 tl, quint16 bl, quint16 tr, quint16 br);
    FullTile(TileInfo tl, TileInfo bl, TileInfo tr, TileInfo br);
    FullTile() = default;
    int offset = -1;
    TileInfo topleft;
    TileInfo bottomleft;
    TileInfo topright;
    TileInfo bottomright;
    QImage getFullTile();
    QImage getScaled(int width);
    void SetPalette(int pal);
    void SetOffset(int off);
    void FlipX();
    void FlipY();
    bool isEmpty();
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
    int map16tile;
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
    void setTileNum(int num);
    int TileNum();
};

#endif // CLIPBOARDTILE_H
