#ifndef MAP16GRAPHICSVIEW_H
#define MAP16GRAPHICSVIEW_H

#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QDataStream>
#include <QFile>
#include <QLabel>
#include <QScrollBar>
#include <functional>
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
};

struct FullTile {
    FullTile(quint16 tl, quint16 bl, quint16 tr, quint16 br);
    TileInfo topleft;
    TileInfo bottomleft;
    TileInfo topright;
    TileInfo bottomright;
    QImage getFullTile();
    void SetPalette(int pal);
    void FlipX();
    void FlipY();
};


enum class SelectorType {
    Eight = 8,
    Sixteen = 16
};

enum class TileChangeType {
    All,
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight
};

enum class TileChangeAction {
    Number,
    FlipX,
    FlipY,
    Palette
};

class Map16GraphicsView : public QGraphicsView
{
    Q_OBJECT
private:
    QGraphicsScene* currScene = nullptr;
    QGraphicsPixmapItem* currentMap16 = nullptr;
    QLabel* tileNumLabel;
    QImage TileMap;
    QImage Grid;
    QImage PageSep;
    QPixmap currentWithNoHighlight;
    QPixmap currentWithNoSelection;
public:
    int imageWidth = 0;
    int imageHeight = 0;
    bool useGrid = false;
    bool usePageSep = false;
    bool hasAlreadyDrawnOnce = false;
    int currentTile = -1;
    int currentClickedTile = -1;
    QPoint currentClickedPoint;
    int previousTile = -1;
    SelectorType currType = SelectorType::Sixteen;
    std::function<void(FullTile, int, int)> clickCallback;
    QVector<QVector<FullTile>> tiles;
    Map16GraphicsView(QWidget* parent = nullptr);
    void setControllingLabel(QLabel *tileNoLabel);
    void mouseMoveEvent(QMouseEvent *event);
    void readInternalMap16File(const QString& name = ":/Resources/spriteMapData.map16");
    void readExternalMap16File(const QString& name);
    void drawInternalMap16File();
    int mouseCoordinatesToTile(QPoint position);
    QPoint translateToRect(QPoint position);
    FullTile& tileNumToTile();
    void drawCurrentSelectedTile(QPixmap& map);
    void tileChanged(TileChangeAction action, TileChangeType type = TileChangeType::All, int value = -1);
    void mousePressEvent(QMouseEvent* event);
    void registerMouseClickCallback(const std::function<void(FullTile, int, int)>& callback);
    ~Map16GraphicsView();
    void useGridChanged();
    void usePageSepChanged();
    void addGrid();
    void removeGrid();
    void addPageSep();
    void removePageSep();

    void switchCurrSelectionType();
};

#endif // MAP16GRAPHICSVIEW_H
