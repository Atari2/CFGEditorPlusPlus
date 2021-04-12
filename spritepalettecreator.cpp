#include "spritepalettecreator.h"

void SpritePaletteCreator::ReadPaletteFile(int offset, int rows, int columns) {
    QFile pal{":/palettedata/Resources/sprites_palettes.pal"};
    pal.open(QFile::OpenModeFlag::ReadOnly);
    auto bytes = pal.readAll();
    for (int row = 0; row < rows; row++) {
        paletteData.append(QVector<QColor>());
        for (int col = 0; col < columns; col++) {
            paletteData[row].append(QColor::fromRgb(
                                        bytes[offset + columns * 3 * row + 3 * col + 0] & 0xFF,
                                        bytes[offset + columns * 3 * row + 3 * col + 1] & 0xFF,
                                        bytes[offset + columns * 3 * row + 3 * col + 2] & 0xFF
                                    ));
        }
    }
}
QPixmap SpritePaletteCreator::MakePalette(int index) {
    QPixmap b(8*16, 16);
    QPainter p{&b};
    p.setBrush(QBrush(Qt::BrushStyle::SolidPattern));
    for (int i = 0; i < 8; i++) {
        p.fillRect(QRect(16 * i, 0, 16, 16), paletteData[index][i]);
    }
    p.end();
    return b;
}
