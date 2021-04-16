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

class Map16GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:

    QVector<QVector<FullTile>> tiles;
    Map16GraphicsView(QWidget* parent = nullptr);
    void mouseMoveEvent(QMouseEvent *event);
    void readInternalMap16File();
    int mouseCoordinatesToTile(QPoint position);
signals:
};

#endif // MAP16GRAPHICSVIEW_H
