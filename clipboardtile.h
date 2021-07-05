#ifndef CLIPBOARDTILE_H
#define CLIPBOARDTILE_H
#include <QImage>
#include "snesgfxconverter.h"
#include "spritepalettecreator.h"

enum class TileChangeType {
    All,
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight
};

struct ExternalGfxInfo {
    qsizetype start;
    qsizetype end;
    int basetile;
};

struct TileInfo {
    TileInfo(quint16 tile);
    TileInfo() = default;
    bool is_this_tile = true;
    bool vflip;
    bool hflip;
    bool prio;
    quint8 pal;
    quint16 tilenum;
    QImage get8x8Tile(int offset);
    QImage get8x8Scaled(int width, int offset);
    bool isEmpty();
    bool isThisTile();
    quint16 TileValue();
};

struct FullTile {
    FullTile(quint16 tl, quint16 bl, quint16 tr, quint16 br, TileChangeType type = TileChangeType::All);
    FullTile(TileInfo tl, TileInfo bl, TileInfo tr, TileInfo br, TileChangeType type = TileChangeType::All);
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
    bool isFullTile();

    void setTileInfoByType(TileInfo info, TileChangeType type);
    TileInfo getTileInfoByType(TileChangeType type);
    QImage getPartialTile(TileChangeType type);
};

class ClipboardTile
{
private:
    bool valid;
    FullTile tile;
    int map16tile;
public:
    ClipboardTile();
    void update(const FullTile& tile);
    void update(const ClipboardTile& other);
    QImage draw();
    FullTile getTile();
    int size();
    bool isValid();
    void setTileNum(int num);
    int TileNum();
};

#endif // CLIPBOARDTILE_H
