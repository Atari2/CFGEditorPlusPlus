#ifndef MAP16PROVIDER_H
#define MAP16PROVIDER_H
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>
#include "clipboardtile.h"

enum SizeSelector : int {
    Sixteen = 16,
    Eight = 8,
    Four = 4,
    Two = 2,
    One = 1
};

struct TiledPosition {
    static inline size_t unique_index = 0;
    TileChangeType type = TileChangeType::All;
    FullTile tile;
    QPoint pos;
    int zpos;
    size_t tid;
    bool operator==(const TiledPosition& other) {
        return tid == other.tid;
    }
};

class Map16Provider : public QLabel
{
    Q_OBJECT
public:
    using DisplayTiles = QVector<TiledPosition>;
    Map16Provider(QWidget* parent = nullptr);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    int mouseCoordinatesToTile(QPoint position);
    void setCopiedTile(ClipboardTile& tile);
    QPoint alignToGrid(QPoint position, int size);
    QPixmap drawSelectedTile();
    QPixmap createGrid();
    QPixmap createBase();
    QPixmap overlay();
    void redraw();
    ClipboardTile* getCopiedTile();
    QPixmap tileGrid;
    QPixmap base;
    bool currentlyPressed = false;
    // tiles should not be saved here
    // they need to be saved elsewhere too
    const DisplayTiles& Tiles();
    SizeSelector getSelectorSize();
    void setSelectorSize(SizeSelector size);
    void addDisplay(int index = -1);
    void removeDisplay(int index = -1);
    void changeDisplay(int newindex);
    void cloneDisplay(int index = -1);
private:
    TiledPosition invalid {TileChangeType::All, {0, 0, 0, 0}, {0, 0}, 0, SIZE_MAX};
    TiledPosition& findIndex(size_t index);
    size_t currentSelected = SIZE_MAX;
    int currentIndex = -1;
    SizeSelector selectorSize = SizeSelector::Sixteen;
    QVector<DisplayTiles> m_tiles;
    ClipboardTile* copiedTile = nullptr;
    QVector<QPixmap> displays;
signals:
};

#endif // MAP16PROVIDER_H
