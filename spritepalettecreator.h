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
    static void ReadPaletteFile(int offset = 0, int rows = 8, int columns = 16, const QString& filename = ":/Resources/sprites_palettes.pal");
    constexpr static int nSpritePalettes() { return 8; }
    static QPixmap MakePalette(int index);
    static QPixmap MakeFullPalette();
    static void PaletteToFile(const QImage& image, const QString& name);
    static void changePaletteColor(const QColor& color, const QPoint& index);
};

#endif // SPRITEPALETTECREATOR_H
