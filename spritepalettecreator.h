#ifndef SPRITEPALETTECREATOR_H
#define SPRITEPALETTECREATOR_H

#include <QImage>
#include <QColor>
#include <QVector>
#include <QBitmap>
#include <QFile>
#include <QPaintEngine>
#include <QDebug>
class SpritePaletteCreator
{
    static inline QVector<QVector<QColor>> paletteData;
public:
    static const QVector<QColor>& getPalette(int index);
    static void ReadPaletteFile(int offset = 0, int rows = 8, int columns = 16);
    constexpr static int npalettes() { return 22; }
    static QPixmap MakePalette(int index);
};

#endif // SPRITEPALETTECREATOR_H
