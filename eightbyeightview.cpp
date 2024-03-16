#include "eightbyeightview.h"
#include "eightbyeightviewcontainer.h"

PgUpDownEventFilter::PgUpDownEventFilter(QObject* parent) : QObject{parent} {

}

bool PgUpDownEventFilter::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_PageDown || keyEvent->key() == Qt::Key_PageUp) {
            return true;
        }
        return false;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void EightByEightView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_PageDown) {
        auto new_index = m_paletteComboBox->currentIndex() + 1;
        if (new_index < m_paletteComboBox->count()) {
            m_paletteComboBox->setCurrentIndex(new_index);
        }
        event->accept();
    } else if (event->key() == Qt::Key_PageUp) {
        auto new_index = m_paletteComboBox->currentIndex() - 1;
        if (new_index > 0) {
            m_paletteComboBox->setCurrentIndex(new_index);
        }
        event->accept();
    }
}

EightByEightView::EightByEightView(QGraphicsScene* ogscene) : QGraphicsView(ogscene) {
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setWindowTitle("8x8 Tile Viewer");
    verticalScrollBar()->setRange(0, 16);
    QObject::connect(verticalScrollBar(), QOverload<int>::of(&QScrollBar::valueChanged), this, [&](int value) {
        const int step = 16;
        int offset = value % step;
        if (offset != 0)
            verticalScrollBar()->setValue(value - offset);
    });
    verticalScrollBar()->installEventFilter(new PgUpDownEventFilter(this));
	setFixedSize(275, 256);
}

int EightByEightView::convertPointToTile(const QPointF& point) {
    int intx = (int)point.x();
    int inty = (int)point.y();
    int scrollbary = (int)verticalScrollBar()->sliderPosition();
    // this might *seem* like a useless calculation but in reality without this it doesn't work properly
    // yay for rounding
    return  (((inty / 16) * 16) + ((scrollbary / 16) * 16)) + (intx / 16);
}

void EightByEightView::setComboBox(QComboBox* comboBox) {
    m_paletteComboBox = comboBox;
}

void EightByEightView::mouseMoveEvent(QMouseEvent *event) {
    if (!m_open || !hasFocus()) {
        event->ignore();
        return;
    }
	static_cast<EightByEightViewContainer*>(parent())->updateTileLabel(QString::asprintf("Tile: %03X", convertPointToTile(event->position())));
}

void EightByEightView::open() {
    m_open = true;
    show();
    raise();
    verticalScrollBar()->setFocusPolicy(Qt::NoFocus);
}

void EightByEightView::close(QCloseEvent *event) {
	closeEvent(event);
}

void EightByEightView::updateForChange(QImage* image) {
    if (currentItem) {
        scene()->removeItem(currentItem);
		delete currentItem;
    }
    currentItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image).scaled(image->size() * 2));
    currentItem->setAcceptHoverEvents(true);
    scene()->addItem(currentItem);
    setFixedSize(256 + 18, 256);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
void EightByEightView::closeEvent(QCloseEvent* event) {
    m_open = false;
    event->accept();
}

EightByEightView::~EightByEightView() {
    qDebug() << "EightbyEight destructor called";
    delete currentItem;
}
