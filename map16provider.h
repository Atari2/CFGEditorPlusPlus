#ifndef MAP16PROVIDER_H
#define MAP16PROVIDER_H
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>
#include "clipboardtile.h"

class Map16Provider : public QLabel
{
    Q_OBJECT
public:
    Map16Provider(QWidget* parent = nullptr);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    int mouseCoordinatesToTile(QPoint position);
    void setCopiedTile(ClipboardTile& tile);
    QPoint alignToGrid(QPoint position, int size);
    static QPixmap createRedGrid();
    ClipboardTile* getCopiedTile();
private:
    ClipboardTile* copiedTile = nullptr;
signals:
};

#endif // MAP16PROVIDER_H
