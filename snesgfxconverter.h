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
#include <windows.h>

class SnesGFXConverter
{
    QByteArray imageData;
private:
    SnesGFXConverter(const QString& name);
    QImage get8x8Tile(int row, int column, const QVector<QColor>& colors);
public:
    static QImage fromResource(const QString& name, const QVector<QColor>& colors);
};

#endif // SNESGFXCONVERTER_H
