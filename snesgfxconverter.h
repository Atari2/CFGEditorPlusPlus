#ifndef SNESGFXCONVERTER_H
#define SNESGFXCONVERTER_H

#include <QImage>
#include <QFile>
#include <QDebug>
#include <QRgb>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QDir>
#include "utils.h"

class SnesGFXConverter
{
    static inline QString GFXExAnimations = ":/Resources/Graphics/GFX33.bin";
    static inline QByteArray fullmap16data;
    static inline QByteArray exgfxmap16data;
    QByteArray imageData;
private:
    SnesGFXConverter(const QString& name);
    ~SnesGFXConverter();
    QImage get8x8Tile(int row, int column, const QVector<QColor>& colors);
public:
    static void populateFullMap16Data(const QVector<QString>& names);
    static QImage fromResource(const QString& name, const QVector<QColor>& colors);
    static QImage get8x8TileFromVect(int index, const QVector<QColor>& colors);
    static QImage get8x8TileFromExternal(int index, const QVector<QColor>& colors, int gfxfileno);
    static void setCustomExanimation(const QString& other);
    static void populateExternalMap16Data(const QVector<QString>& names);
    static void clearnExternalMap16Data();
};

#endif // SNESGFXCONVERTER_H
