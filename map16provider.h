#ifndef MAP16PROVIDER_H
#define MAP16PROVIDER_H
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>

class Map16Provider : public QLabel
{
    Q_OBJECT
public:
    Map16Provider(QWidget* parent = nullptr);
    void mouseMoveEvent(QMouseEvent *event);
    int mouseCoordinatesToTile(QPoint position);
    static QPixmap createRedGrid();
private:
signals:
};

#endif // MAP16PROVIDER_H
