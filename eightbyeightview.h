#ifndef EIGHTBYEIGHTVIEW_H
#define EIGHTBYEIGHTVIEW_H

#include <QGraphicsView>
#include <QCloseEvent>
#include <QGraphicsItem>
#include <QTimer>
#include <QScrollBar>
#include <QToolTip>
#include <QComboBox>

class PgUpDownEventFilter : public QObject {
    Q_OBJECT

public:
    PgUpDownEventFilter(QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

class EightByEightView : public QGraphicsView
{
public:
    EightByEightView(QGraphicsScene* scene);
    void updateForChange(QImage* image);
    void setComboBox(QComboBox* comboBox);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent* event) override;
    ~EightByEightView();
    void open();
	void close(QCloseEvent* event);
private:
    QGraphicsPixmapItem* currentItem = nullptr;
    QComboBox* m_paletteComboBox = nullptr;
    bool m_open = false;
    void closeEvent(QCloseEvent* event);
    int convertPointToTile(const QPointF& point);
};

#endif // EIGHTBYEIGHTVIEW_H
