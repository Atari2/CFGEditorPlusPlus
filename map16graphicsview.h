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
};


enum class SelectorType {
    Eight = 8,
    Sixteen = 16
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
public:
    int imageWidth = 0;
    int imageHeight = 0;
    bool useGrid = false;
    bool usePageSep = false;
    SelectorType currType = SelectorType::Sixteen;
    QVector<QVector<FullTile>> tiles;
    Map16GraphicsView(QWidget* parent = nullptr);
    void setControllingLabel(QLabel *tileNoLabel);
    void mouseMoveEvent(QMouseEvent *event);
    void readInternalMap16File(const QString& name = ":/Resources/spriteMapData.map16");
    void readExternalMap16File(const QString& name);
    void drawInternalMap16File();
    int mouseCoordinatesToTile(QPoint position);
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
