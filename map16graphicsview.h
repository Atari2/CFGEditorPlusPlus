#ifndef MAP16GRAPHICSVIEW_H
#define MAP16GRAPHICSVIEW_H

#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QDataStream>
#include <QFile>
#include <QLabel>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QComboBox>
#include <functional>
#include "clipboardtile.h"

enum class SelectorType : int {
    Eight = 8,
    Sixteen = 16
};

enum class TileChangeType {
    All,
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight
};

enum class TileChangeAction {
    Number,
    FlipX,
    FlipY,
    Palette
};

class Map16GraphicsView : public QGraphicsView
{
    Q_OBJECT
private:
    QGraphicsScene* currScene = nullptr;
    QGraphicsPixmapItem* currentMap16 = nullptr;
    QLabel* tileNumLabel;
    QImage TileMap;
    QImage Grid;
    QImage PageSep;
    QPixmap currentWithNoHighlight;
    QPixmap currentWithNoSelection;
    ClipboardTile* copiedTile = nullptr;
public:
    int imageWidth = 0;
    int imageHeight = 0;
    bool noSignals = false;
    bool useGrid = false;
    bool usePageSep = false;
    int currentTile = -1;
    int currentClickedTile = -1;
    QPoint currentTopLeftClicked;
    int previousTile = -1;
    SelectorType currType = SelectorType::Sixteen;
    int CellSize();
    std::function<void(FullTile, int, SelectorType)> clickCallback;
    QVector<QVector<FullTile>> tiles;
    Map16GraphicsView(QWidget* parent = nullptr);
    void setControllingLabel(QLabel *tileNoLabel);
    void changePaletteIndex(QComboBox* box, FullTile tile);
    void mouseMoveEvent(QMouseEvent *event);
    void readInternalMap16File(const QString& name = ":/Resources/spriteMapData.map16");
    void readExternalMap16File(const QString& name);
    void drawInternalMap16File();
    int mouseCoordinatesToTile(QPoint position);
    QPoint translateToRect(QPoint position);
    FullTile& tileNumToTile();
    void drawCurrentSelectedTile(QPixmap& map);
    void tileChanged(QObject* toBlock, TileChangeAction action, TileChangeType type = TileChangeType::All, int value = -1);
    void mousePressEvent(QMouseEvent* event);
    void registerMouseClickCallback(const std::function<void(FullTile, int, SelectorType)>& callback);
    void copyTileToClipboard(const FullTile& tile);
    TileChangeType getChangeType();
    ~Map16GraphicsView();
    void useGridChanged();
    void usePageSepChanged();
    void addGrid();
    void removeGrid();
    void addPageSep();
    void removePageSep();
    void setCopiedTile(ClipboardTile& tile);
    const ClipboardTile& getCopiedTile();
    void switchCurrSelectionType();
};

#endif // MAP16GRAPHICSVIEW_H
