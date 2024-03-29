#ifndef MAP16PROVIDER_H
#define MAP16PROVIDER_H
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>
#include "spritedatamodel.h"
#include "map16graphicsview.h"

enum SizeSelector : int {
    Sixteen = 16,
    Eight = 8,
    Four = 4,
    Two = 2,
    One = 1
};

struct TiledPosition {
    static inline size_t unique_index = 0;
    FullTile tile;
    QPoint pos;
    int zpos;
    size_t tid;
    int map16tileno;
    static TiledPosition getInvalid() {
        TiledPosition pos;
        pos.tile = {0, 0, 0, 0};
        pos.pos = QPoint{0, 0};
        pos.zpos = 0;
        pos.tid = SIZE_MAX;
        pos.map16tileno = -1;
        return pos;
    }

    bool operator==(const TiledPosition& other) const {
        return tid == other.tid;
    }
    bool operator==(size_t other) const {
        return tid == other;
    }
};

class Map16Provider : public QLabel
{
    Q_OBJECT
public:
    using DisplayTiles = QVector<TiledPosition>;
    Map16Provider(QWidget* parent = nullptr);
    void attachMap16View(Map16GraphicsView* view);
    void focusOutEvent(QFocusEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    int mouseCoordinatesToTile(QPoint position);
    void setCopiedTile(ClipboardTile& tile);
    QPoint alignToGrid(QPoint position, int size);
    void insertText(const QString& text);
    QPixmap drawSelectedTile();
    void drawLetters(QPainter& p);
    QPixmap createGrid();
    QPixmap createBase();
    QPixmap overlay();
    void redraw();
    void redrawNoSort();
    void redrawFirstIndex();
    ClipboardTile* getCopiedTile();
    void reset();
    QPixmap tileGrid;
    QPixmap base;
    bool currentlyPressed = false;
    const DisplayTiles& Tiles();
    SizeSelector getSelectorSize();
    void setSelectorSize(SizeSelector size);
    void setUseText(bool enabled);
    void addDisplay(int index = -1);
    void removeDisplay(int index = -1);
    void changeDisplay(int newindex);
    void cloneDisplay(int index = -1);
    void serializeDisplays(QVector<DisplayData>& data);
    void deserializeDisplays(const QVector<JSONDisplay>& display, Map16GraphicsView* view);
private:
    TiledPosition invalid = TiledPosition::getInvalid();
    TiledPosition& findIndex(size_t index);
    size_t currentSelected = SIZE_MAX;
    int currentIndex = -1;
    SizeSelector selectorSize = SizeSelector::Sixteen;
    std::array<QImage, 26 + 26 + 10 + 9> m_letters;
    const std::map<char, int> table = {{'a', 0}, {'b', 1}, {'c', 2}, {'d', 3}, {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}, {'i', 8}, {'j', 9}, {'k', 10}, {'l', 11}, {'m', 12}, {'n', 13}, {'o', 14}, {'p', 15}, {'q', 16}, {'r', 17}, {'s', 18}, {'t', 19}, {'u', 20}, {'v', 21}, {'w', 22}, {'x', 23}, {'y', 24}, {'z', 25}, {'A', 26}, {'B', 27}, {'C', 28}, {'D', 29}, {'E', 30}, {'F', 31}, {'G', 32}, {'H', 33}, {'I', 34}, {'J', 35}, {'K', 36}, {'L', 37}, {'M', 38}, {'N', 39}, {'O', 40}, {'P', 41}, {'Q', 42}, {'R', 43}, {'S', 44}, {'T', 45}, {'U', 46}, {'V', 47}, {'W', 48}, {'X', 49}, {'Y', 50}, {'Z', 51}, {'0', 52}, {'1', 53}, {'2', 54}, {'3', 55}, {'4', 56}, {'5', 57}, {'6', 58}, {'7', 59}, {'8', 60}, {'9', 61}, {',', 62}, {'.', 63}, {'!', 64}, {'?', 65}, {'-', 66}, {' ', 67}, {'\'', 68}, {'"', 69}, {'&', 70}};
    QVector<QString> m_descriptions;
    QVector<bool> usesText;
    QVector<DisplayTiles> m_tiles;
    ClipboardTile* copiedTile = nullptr;
    Map16GraphicsView* view = nullptr;
    QVector<QPixmap> m_displays;
signals:
};

#endif // MAP16PROVIDER_H
