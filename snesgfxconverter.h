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

class SnesGFXConverter
{
    static inline QString GFXExAnimations = ":/Resources/Graphics/GFX33.bin";
    static inline QByteArray fullmap16data;
    QByteArray imageData;
private:
    SnesGFXConverter(const QString& name);
    ~SnesGFXConverter();
    QImage get8x8Tile(int row, int column, const QVector<QColor>& colors);
public:
    static void populateFullMap16Data(const QVector<QString>& names);
    static QImage fromResource(const QString& name, const QVector<QColor>& colors);
    static QImage get8x8TileFromVect(int index, const QVector<QColor>& colors);
    static void setCustomExanimation(const QString& other);
};

#endif // SNESGFXCONVERTER_H
