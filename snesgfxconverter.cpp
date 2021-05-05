#include "snesgfxconverter.h"

SnesGFXConverter::SnesGFXConverter(const QString& name) {
    QFile file{name};
    file.open(QFile::OpenModeFlag::ReadOnly);
    imageData = file.readAll();
    file.close();
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

void SnesGFXConverter::populateExternalMap16Data(const QVector<QString>& names) {
    auto ret = std::find_if_not(names.cbegin(), names.cend(), [](const auto& name) {
            return assert_filesize<LessThanOrEqual>(name, kb(32));
        });
    if (ret != names.cend())
        return;
    exgfxmap16data.clear();
    for (auto& name : names) {
        QFile file{name};
        file.open(QFile::OpenModeFlag::ReadOnly);
        auto data = file.readAll();
        if (data.length() < kb(32))
            data = data.leftJustified(kb(32), 0);
        exgfxmap16data.append(data);
    }
}

void SnesGFXConverter::clearnExternalMap16Data() {
    exgfxmap16data.clear();
}

QImage SnesGFXConverter::get8x8TileFromVect(int index, const QVector<QColor>& colors) {
    int offset = index * 8 * 4;
    QImage image(8, 8, QImage::Format::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));
    QVector<QRgb> rgbColors;
    rgbColors.reserve(15);
    // we skip the first color to make it transparent
    std::for_each(colors.cbegin() + 1, colors.cend(), [&](const QColor& col) {
         rgbColors.append(col.rgba());
    });
    for (int row = 0; row < 8; row++) {
        uint8_t bytes[4] = { 0 };
        for (int i = 0; i < 4; i++) {
            qsizetype val = row * 0x2 + offset + (i & 1) + ((i & 0xFE) << 3);
            if (val >= fullmap16data.length()) {
                DefaultAlertImpl(nullptr, QString::asprintf("8x8 Tile number %03X was out of bounds. Maybe missing an external file?", index))();
                return image;
            }
            bytes[i] = fullmap16data[val];
        }
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t pixel = 0;
            for (int i = 0; i < 4; i++)
                pixel |= ((bytes[i] & (1 << bit)) >> bit) << i;
            if (pixel != 0)
                image.setPixelColor(7 - bit, row, rgbColors[pixel - 1]);
        }
    }
    return image;
}

QImage SnesGFXConverter::get8x8TileFromExternal(int index, const QVector<QColor>& colors, int extra_offset) {
    int offset = index * 8 * 4 + (extra_offset * 8 * 4);
    QImage image(8, 8, QImage::Format::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));
    QVector<QRgb> rgbColors;
    rgbColors.reserve(15);
    // we skip the first color to make it transparent
    std::for_each(colors.cbegin() + 1, colors.cend(), [&](const QColor& col) {
         rgbColors.append(col.rgba());
    });
    for (int row = 0; row < 8; row++) {
        uint8_t bytes[4] = { 0 };
        for (int i = 0; i < 4; i++) {
            qsizetype val = row * 0x2 + offset + (i & 1) + ((i & 0xFE) << 3);
            bytes[i] = exgfxmap16data[val];
        }
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t pixel = 0;
            for (int i = 0; i < 4; i++)
                pixel |= ((bytes[i] & (1 << bit)) >> bit) << i;
            if (pixel != 0)
                image.setPixelColor(7 - bit, row, rgbColors[pixel - 1]);
        }
    }
    return image;
}

QImage SnesGFXConverter::get8x8Tile(int orig_row, int orig_column, const QVector<QColor>& colors) {
    int offset = (orig_row * 16 * 32) + (32 * orig_column);
    QImage image(8, 8, QImage::Format_ARGB32);
    QVector<QRgb> rgbColors;
    rgbColors.reserve(15);
    // skip the first color and append it manually cause it needs to be transparent
    std::for_each(colors.cbegin() + 1, colors.cend(), [&](const QColor& col) {
         rgbColors.append(col.rgba());
    });
    for (int row = 0; row < 8; row++) {
        uint8_t bytes[4];
        for (int i = 0; i < 4; i++) {
            bytes[i] = imageData[row * 0x2 + offset + (i & 1) + ((i & 0xFE) << 3)];
        }
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t pixel = 0;
            for (int i = 0; i < 4; i++)
                pixel |= ((bytes[i] & (1 << bit)) >> bit) << i;
            image.setPixelColor(7 - bit, row, pixel == 0 ? Qt::transparent : rgbColors[pixel - 1]);
        }
    }
    return image;
}
QImage SnesGFXConverter::fromResource(const QString& name, const QVector<QColor>& colors) {
    SnesGFXConverter converter{name};
    QImage fullMap(128, 64, QImage::Format::Format_ARGB32);
    QPainter f(&fullMap);
    f.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 8; col++) {
            QImage fullTile(16, 16, QImage::Format::Format_ARGB32);
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
    converter.imageData.clear();
    return fullMap;
}

SnesGFXConverter::~SnesGFXConverter() {
    imageData.clear();
    imageData.~QByteArray();
}
