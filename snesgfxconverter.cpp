#include "snesgfxconverter.h"

SnesGFXConverter::SnesGFXConverter(const QString& name) {
    QFile file{name};
    file.open(QFile::OpenModeFlag::ReadOnly);
    imageData = file.readAll();
}

void SnesGFXConverter::setCustomExanimation(const QString& other) {
    GFXExAnimations = other;
}

void SnesGFXConverter::populateFullMap16Data(const QVector<QString>& names) {
    qDebug() << "Populating map16 data with " << names;
    fullmap16data.clear();
    for (auto& name : names) {
        if (QDir(name).isAbsolute()) {
            QFile file{name};
            file.open(QFile::OpenModeFlag::ReadOnly);
            fullmap16data.append(file.readAll());
        }
        else {
            QFile file{":/Resources/Graphics/" + name + ".bin"};
            file.open(QFile::OpenModeFlag::ReadOnly);
            fullmap16data.append(file.readAll());
       }
    }
    QFile file{GFXExAnimations};
    file.open(QFile::OpenModeFlag::ReadOnly);
    fullmap16data.append(file.readAll());
}

QImage SnesGFXConverter::get8x8TileFromVect(int index, const QVector<QColor>& colors) {
    int offset = index * 8 * 4;
    QImage image(8, 8, QImage::Format_Indexed8);
    QVector<QRgb> rgbColors;
    std::for_each(colors.cbegin(), colors.cend(), [&](const QColor& col) {
         rgbColors.append(col.rgb());
    });
    image.setColorTable(rgbColors);
    for (int row = 0; row < 8; row++) {
        uint8_t bytes[4];
        for (int i = 0; i < 4; i++) {
            bytes[i] = fullmap16data[row * 0x2 + offset + (i & 1) + ((i & 0xFE) << 3)];
        }
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t pixel = 0;
            for (int i = 0; i < 4; i++)
                pixel |= ((bytes[i] & (1 << bit)) >> bit) << i;
            image.setPixel(7 - bit, row, pixel);
        }
    }
    return image;
}

QImage SnesGFXConverter::get8x8Tile(int orig_row, int orig_column, const QVector<QColor>& colors) {
    int offset = (orig_row * 16 * 32) + (32 * orig_column);
    QImage image(8, 8, QImage::Format_Indexed8);
    image.setColorCount(16);
    QVector<QRgb> rgbColors;
    std::for_each(colors.cbegin(), colors.cend(), [&](const QColor& col) {
         rgbColors.append(col.rgb());
    });
    image.setColorTable(rgbColors);
    for (int row = 0; row < 8; row++) {
        uint8_t bytes[4];
        for (int i = 0; i < 4; i++) {
            bytes[i] = imageData[row * 0x2 + offset + (i & 1) + ((i & 0xFE) << 3)];
        }
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t pixel = 0;
            for (int i = 0; i < 4; i++)
                pixel |= ((bytes[i] & (1 << bit)) >> bit) << i;
            image.setPixel(7 - bit, row, pixel);
        }
    }
    return image;
}
QImage SnesGFXConverter::fromResource(const QString& name, const QVector<QColor>& colors) {
    SnesGFXConverter converter{name};
    QImage fullMap(128, 64, QImage::Format::Format_RGB32);
    QPainter f(&fullMap);
    f.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 8; col++) {
            QImage fullTile(16, 16, QImage::Format::Format_RGB32);
            QPainter p(&fullTile);
            p.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
            p.drawImage(QRect{0, 0, 8, 8}, converter.get8x8Tile(row * 2, col * 2, colors), QRect{0, 0, 8, 8});
            p.drawImage(QRect{8, 0, 8, 8}, converter.get8x8Tile(row * 2, col * 2 + 1, colors), QRect{0, 0, 8, 8});
            p.drawImage(QRect{0, 8, 8, 8}, converter.get8x8Tile(row * 2 + 1, col * 2, colors), QRect{0, 0, 8, 8});
            p.drawImage(QRect{8, 8, 8, 8}, converter.get8x8Tile(row * 2 + 1, col * 2 + 1, colors), QRect{0, 0, 8, 8});
            p.end();
            f.drawImage(QRect{col * 16, row * 16, 16, 16}, fullTile, QRect{0, 0, 16, 16});
        }
    }
    f.end();
    return fullMap;
}
