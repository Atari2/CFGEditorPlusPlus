#ifndef EIGHTBYEIGHTVIEW_H
#define EIGHTBYEIGHTVIEW_H

#include <QGraphicsView>
#include <QCloseEvent>
#include <QGraphicsItem>
#include <QTimer>
#include <QScrollBar>
#include <QToolTip>

class EightByEightView : public QGraphicsView
{
public:
    EightByEightView(QGraphicsScene* scene);
    void updateForChange(QImage* image);
    void mouseMoveEvent(QMouseEvent *event);
    ~EightByEightView();
    void open();
	void close(QCloseEvent* event);
private:
    QGraphicsPixmapItem* currentItem = nullptr;
    bool m_open = false;
    void closeEvent(QCloseEvent* event);
    int convertPointToTile(const QPointF& point);
};

#endif // EIGHTBYEIGHTVIEW_H
