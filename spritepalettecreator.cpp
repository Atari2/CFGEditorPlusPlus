#include "spritepalettecreator.h"

const QVector<QColor>& SpritePaletteCreator::getPalette(int index) {
    return paletteData[index];
}

void SpritePaletteCreator::ReadPaletteFile(int offset, int rows, int columns, const QString& filename) {
    if (paletteData.length() != 0) {
        for (int i = 0; i < paletteData.length(); i++)
            paletteData[i].clear();
        paletteData.clear();
    }
    QFile pal{filename};
    pal.open(QFile::OpenModeFlag::ReadOnly);
    auto bytes = pal.readAll();
    for (int row = 0; row < rows; row++) {
        paletteData.append(QVector<QColor>());
        for (int col = 0; col < columns; col++) {
            paletteData[row].append(QColor::fromRgba(
                                                   qRgba(
                                                        bytes[offset + columns * 3 * row + 3 * col + 0] & 0xFF,
                                                        bytes[offset + columns * 3 * row + 3 * col + 1] & 0xFF,
                                                        bytes[offset + columns * 3 * row + 3 * col + 2] & 0xFF,
                                                        255
                                                   ))
                                   );
        }
    }
}
QPixmap SpritePaletteCreator::MakePalette(int index) {
    QPixmap b(16*8, 16);
    QPainter p{&b};
    p.setBrush(QBrush(Qt::BrushStyle::SolidPattern));
    for (int i = 0; i < 8; i++) {
        p.fillRect(QRect(16 * i, 0, 16, 16), paletteData[index + 8][i]);
    }
    p.end();
    return b;
}

QPixmap SpritePaletteCreator::MakeFullPalette() {
    QPixmap b(16*16, 8*16);
    QPainter p{&b};
    p.setBrush(QBrush(Qt::BrushStyle::SolidPattern));
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            p.fillRect(QRect(16 * j, 16 * i, 16, 16), paletteData[i + 8][j]);
        }
    }
    p.end();
    return b;
}

void SpritePaletteCreator::PaletteToFile(const QImage& image, const QString& name) {
    QVector<QColor> colors;
    QString palmaskName = name.endsWith(".pal") ? name.chopped(3) + "palmask" : name + ".palmask";
    for (int i = 0; i < image.height(); i += 16)
        for (int j = 0; j < image.width(); j += 16)
            colors.append(image.pixel(j, i));
    QFile file{name};
    QFile palmask{palmaskName};
    file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Truncate);
    palmask.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Truncate);
    qDebug() << colors.length();
    QByteArray blankPalmask(colors.length(), 0);
    QByteArray coloredPalmask(colors.length(), 1);
    palmask.write(blankPalmask);
    palmask.write(coloredPalmask);
    palmask.write(QByteArray(1, 0));
    QByteArray blank(colors.length() * 3, 0);
    file.write(blank);
    for (const QColor& color : colors) {
        int r, g, b;
        color.getRgb(&r, &g, &b);
        QByteArray arr;
        arr.append((uint8_t)r);
        arr.append((uint8_t)g);
        arr.append((uint8_t)b);
        file.write(arr);
    }
}

void SpritePaletteCreator::changePaletteColor(const QColor& color, const QPoint& index) {
    qDebug() << "X: " << index.x() << "Y: " << index.y();
    paletteData[(index.y() / 16) + 8][index.x() / 16] = color;
}
