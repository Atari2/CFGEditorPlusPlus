#ifndef PALETTEVIEW_H
#define PALETTEVIEW_H

#include <QGraphicsView>
#include <QCloseEvent>
#include <QGraphicsItem>
#include <QTimer>
#include <QScrollBar>
#include <QToolTip>
#include <QPushButton>
#include <QFileDialog>
#include <QColorDialog>
#include <QListView>
#include <QMenu>
#include "spritepalettecreator.h"
#include "utils.h"

class PaletteView : public QGraphicsView
{
    Q_OBJECT
public:
    PaletteView(QGraphicsScene* scene);
    void updateForChange(const QPixmap& image);
    void mousePressEvent(QMouseEvent *event);
    ~PaletteView();
    void open();
	void close(QCloseEvent* event);
	QGraphicsPixmapItem* getCurrentItem();
signals:
    void paletteChanged();
private:
    QGraphicsPixmapItem* currentItem = nullptr;
    QPushButton* button = nullptr;
    bool m_open = false;
    void closeEvent(QCloseEvent* event);
    QPoint convertPointToTile(const QPointF& point);
};

#endif // PALETTEVIEW_H
