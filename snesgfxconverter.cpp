#include "snesgfxconverter.h"

SnesGFXConverter::SnesGFXConverter(const QString& name) {
    QFile file{name};
    file.open(QFile::OpenModeFlag::ReadOnly);
    imageData = file.readAll();
}
QImage SnesGFXConverter::get8x8Tile(int row, int column, const QVector<QColor>& colors) {
    int offset = (row * 16 * 32) + (32 * column);
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

        // this for some reason doesn't pick the right colors in the palette?
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t pixel = 0;
            for (int i = 0; i < 4; i++)
                pixel |= ((bytes[i] & (1 << bit)) >> bit) << (3 - i);
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
            qDebug() << "Drawing 16x16 rectangle at " << row * 16 << " " << col * 16;
            // QGraphicsScene* scene = new QGraphicsScene;
            // QGraphicsView* view = new QGraphicsView(scene);
            // QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(fullTile));
            // scene->addItem(item);
            // view->show();
            f.drawImage(QRect{col * 16, row * 16, 16, 16}, fullTile, QRect{0, 0, 16, 16});
        }
    }
    f.end();
    return fullMap;
}
