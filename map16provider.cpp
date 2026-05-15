#include "map16provider.h"

Map16Provider::Map16Provider(QWidget* parent) : QLabel(parent)  {
    tileGrid = createGrid();
    base = createBase();
    setPixmap(overlay());
    setMargin(0);
    setFixedSize(208, 208);
    setMouseTracking(true);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    QImage letters{":/Resources/Text/Letters.png"};
    size_t n = 0;
    int sizes[3] = {26, 26, 19};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < sizes[i]; j++) {
            m_letters[n] = letters.copy(QRect{j * 8, i * 8, 8, 8});
            n++;
        }
    }
}

void Map16Provider::attachMap16View(Map16GraphicsView* view) {
    this->view = view;
    QObject::connect(this->view, QOverload<const FullTile&, int>::of(&Map16GraphicsView::signalTileUpdatedForDisplay), this, [&](const FullTile& t, int tileno) {
        bool anyChanged = false;
        for (int i = 0; i < m_tiles.size(); i++) {
            bool changed = false;
            for (auto& tile : m_tiles[i]) {
                if (tile.map16tileno == tileno) {
                    tile.tile = t;
                    changed = true;
                }
            }
            if (changed) {
                redrawAt(i);
                anyChanged = true;
            }
        }
        if (anyChanged && currentIndex != -1)
            setPixmap(drawSelectedTile());
    });
}

int Map16Provider::mouseCoordinatesToTile(QPoint position) {
    return ((position.y() / 16) * 16) + (position.x() / 16);
}

TiledPosition& Map16Provider::findIndex(size_t index) {
    auto ret = std::find_if(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [index](TiledPosition& pos) {
        return pos.tid == index;
    });
    if (ret == m_tiles[currentIndex].end())
        return invalid;
    return *ret;
}

void Map16Provider::setCurrentlySelected(size_t tid) {
    m_currentSelected = tid;
    if (tid == SIZE_MAX || currentIndex == -1) {
        emit currentlySelectedTileChanged(tid, false);
    } else {
        auto it = std::find_if(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [tid](const auto& t) {
            return t.tid == tid;
        });
        if (it == m_tiles[currentIndex].end()) {
            emit currentlySelectedTileChanged(tid, false);
        } else {
            emit currentlySelectedTileChanged(tid, it->translucent);
        }
    }
}

QPoint Map16Provider::alignToGrid(QPoint position, int size) {
    return QPoint(position.x() - (position.x() % size), position.y() - (position.y() % size));
}

QPixmap Map16Provider::drawSelectedTile() {
    QPixmap curr = overlay();
    if (m_currentSelected == SIZE_MAX)
        return curr;
    QPainter p{&curr};
    QPen pen{Qt::white, 1, Qt::DotLine, Qt::SquareCap, Qt::BevelJoin};
    p.setPen(pen);
    auto& t = findIndex(m_currentSelected);
    auto size = 16;
    p.drawRect(QRect{t.pos.x(), t.pos.y(), size, size});
    p.end();
    return curr;
}

void Map16Provider::insertText(const QString& text) {
    m_descriptions[currentIndex] = text;
    setPixmap(overlay());
}

void Map16Provider::mousePressEvent(QMouseEvent *event) {
    if (m_tiles.length() == 0 || currentIndex == -1)
        return;
    QPainter p{&m_displays[currentIndex]};
    if (event->button() == Qt::MouseButton::LeftButton) {
        grabKeyboard();
        qDebug() << "Left mouse button pressed";
        auto ret = std::find_if(m_tiles[currentIndex].rbegin(), m_tiles[currentIndex].rend(), [&](TiledPosition& pos) {
            int size = pos.tile.isFullTile() ? 16 : 8;
            return QRect{pos.pos.x(), pos.pos.y(), size, size}.contains(event->position().toPoint(), true);
        });
        if (ret == m_tiles[currentIndex].rend())
            return;
        setCurrentlySelected((*ret).tid);
        pressOffset = event->position().toPoint() - (*ret).pos;
        setPixmap(drawSelectedTile());
        currentlyPressed = true;
        return;
    } else if (event->button() == Qt::MouseButton::RightButton) {
        if (usesText[currentIndex])
            return;
        qDebug() << "Right mouse button pressed";
        if (!copiedTile->isValid())
            return;
        int size = 16;
        QPoint aligned = alignToGrid(event->position().toPoint(), size);
        FullTile fullTile = copiedTile->getTile();
        TiledPosition tile{fullTile, aligned, 0, TiledPosition::unique_index++, copiedTile->TileNum(), fullTile.translucent};
        setCurrentlySelected(tile.tid);
        m_tiles[currentIndex].append(std::move(tile));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawImage(QRect{aligned.x(), aligned.y(), copiedTile->size(), copiedTile->size()}, copiedTile->draw());
    }
    setPixmap(drawSelectedTile());
    event->accept();
}

void Map16Provider::mouseReleaseEvent(QMouseEvent *event) {
    if (currentlyPressed && m_currentSelected != SIZE_MAX) {
        auto& tile = findIndex(m_currentSelected);
        int size = static_cast<int>(selectorSize);
        QPoint p = tile.pos;
        tile.pos = QPoint(((p.x() + size / 2) / size) * size,
                          ((p.y() + size / 2) / size) * size);
        redrawNoSort();
    }
    currentlyPressed = false;
    event->accept();
}

void Map16Provider::mouseMoveEvent(QMouseEvent *event) {
    if (currentlyPressed && m_currentSelected != SIZE_MAX) {
        auto& tile = findIndex(m_currentSelected);
        tile.pos = event->position().toPoint() - pressOffset;
        redrawNoSort();
    }
    event->accept();
}

void Map16Provider::wheelEvent(QWheelEvent *event) {
    if (m_currentSelected == SIZE_MAX) {
        event->accept();
    } else {
        auto& t = findIndex(m_currentSelected);
        if (event->angleDelta().y() < 0 && t.zpos > INT_MIN) {
            t.zpos--;
            redraw();
        } else if (event->angleDelta().y() > 0 && t.zpos < INT_MAX){
            t.zpos++;
            redraw();
        }
    }
}

void Map16Provider::redrawNoSort() {
    m_displays[currentIndex].fill(Qt::transparent);
    QPainter p{&m_displays[currentIndex]};
    for (auto& t : m_tiles[currentIndex]) {
        int size = t.tile.isFullTile() ? 16 : 8;
        p.drawImage(QRect{t.pos.x(), t.pos.y(), size, size}, t.tile.getFullTile(t.translucent));
    }
    p.end();
    setPixmap(drawSelectedTile());
}

void Map16Provider::redrawFirstIndex() {
    if (m_tiles.empty())
        return;
    if (m_tiles.first().empty())
        return;
    m_displays.first().fill(Qt::transparent);
    QPainter p{&m_displays.first()};
    std::sort(m_tiles.first().begin(), m_tiles.first().end(), [](TiledPosition& lhs, TiledPosition& rhs) {
        return lhs.zpos < rhs.zpos;
    });
    for (auto& t : m_tiles.first()) {
        int size = t.tile.isFullTile() ? 16 : 8;
        p.drawImage(QRect{t.pos.x(), t.pos.y(), size, size}, t.tile.getFullTile(t.translucent));
    }
    setPixmap(drawSelectedTile());
}

void Map16Provider::redraw() {
    if (currentIndex == -1) {
        redrawFirstIndex();
        return;
    }
    m_displays[currentIndex].fill(Qt::transparent);
    QPainter p{&m_displays[currentIndex]};
    std::sort(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [](TiledPosition& lhs, TiledPosition& rhs) {
        return lhs.zpos < rhs.zpos;
    });
    for (auto& t : m_tiles[currentIndex]) {
        int size = t.tile.isFullTile() ? 16 : 8;
        p.drawImage(QRect{t.pos.x(), t.pos.y(), size, size}, t.tile.getFullTile(t.translucent));
    }
    setPixmap(drawSelectedTile());
}

void Map16Provider::redrawAt(int index) {
    if (index < 0 || index >= m_displays.size())
        return;
    m_displays[index].fill(Qt::transparent);
    QPainter p{&m_displays[index]};
    for (auto& t : m_tiles[index]) {
        int size = t.tile.isFullTile() ? 16 : 8;
        p.drawImage(QRect{t.pos.x(), t.pos.y(), size, size}, t.tile.getFullTile(t.translucent));
    }
}

void Map16Provider::setTranslucencyForSelectedTile(bool translucent) {
    if (m_currentSelected == SIZE_MAX) return;
    auto it = std::find_if(m_tiles[currentIndex].begin(), m_tiles[currentIndex].end(), [this](const auto& tile) {
        return tile.tid == m_currentSelected;
    });
    if (it == m_tiles[currentIndex].end()) return;
    else it->translucent = translucent;
    redraw();
}

void Map16Provider::redrawAll() {
    for (int i = 0; i < m_displays.size(); i++)
        redrawAt(i);
    if (currentIndex != -1)
        setPixmap(drawSelectedTile());
}

void Map16Provider::setCopiedTile(ClipboardTile& tile) {
    copiedTile = &tile;
}

ClipboardTile* Map16Provider::getCopiedTile() {
    return copiedTile;
}

QPixmap Map16Provider::createBase() {
    QPixmap pix{208, 208};
    pix.fill(Qt::transparent);
    return pix;
}

QPixmap Map16Provider::createGrid() {
    QImage img{208, 208, QImage::Format::Format_ARGB32};
    QPainter p{&img};
    QPen pen(Qt::lightGray, 0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    pen.setWidthF(0.5);
    p.setPen(pen);
    img.fill(qRgba(0, 0, 0, 0));
    int size = static_cast<int>(selectorSize);
    if (size != 8 && size != 16) {
        return QPixmap::fromImage(img);
    }
    for (int i = size; i < 208; i += size) {
        p.drawLine(0, i, 208, i);
        p.drawLine(i, 0, i, 208);
    }

    if (size == 16 || size == 8) {
        pen.setColor(Qt::blue);
        p.setPen(pen);
        constexpr int center = (208 / 2) - (16 / 2);
        QRect box_rect{center, center, 16, 16};
        p.drawRect(box_rect);
    }
    return QPixmap::fromImage(img);
}

QPixmap Map16Provider::overlay() {
    QPixmap pix{208, 208};
    QPainter p{&pix};
    p.fillRect(pix.rect(), QBrush(QGradient(QGradient::AmourAmour)));
    p.drawImage(pix.rect(), tileGrid.toImage());
    if (currentIndex < 0 || currentIndex >= m_displays.size()) {
        p.drawImage(pix.rect(), base.toImage());
    } else if (usesText[currentIndex]) {
        drawLetters(p);
    } else {
        p.drawImage(pix.rect(), m_displays[currentIndex].toImage());
    }
    p.end();
    return pix;
}

void Map16Provider::drawLetters(QPainter& p) {
    constexpr int cpl = 24; // 26 is a whole line
    auto slines = m_descriptions[currentIndex].split("\n", Qt::SkipEmptyParts);
    for (qsizetype i = 0; i < slines.length(); i++) {
        QString str{slines[i].trimmed()};
        int max = str.length();
        if (max > cpl) {
            slines.removeAt(i);
            int curr = 0;
            int j = 0;
            while (max - curr > cpl) {
                auto slice = str.sliced(curr, (curr + cpl >= max) ? (max - curr) : cpl).trimmed();
                auto space = slice.lastIndexOf(' ');
                if (space == -1) {
                    slines.insert(i + j, slice);
                    curr += cpl;
                }
                else {
                    slines.insert(i + j, slice.sliced(0, space));
                    curr += (space + 1);
                }
                j++;
            }
            slines.insert(i + j, str.sliced(curr));
        }
    }
    auto str = m_descriptions[currentIndex].toStdString();
    int lines = slines.length();
    int vmargin = (208 - (lines * 8)) / 2;
    for (int row = 0; row < lines; row++) {
        auto str = slines[row].toStdString();
        int hmargin = static_cast<int>((208 - (str.length() * 8)) / 2);
        for (int col = 0; col < (int)str.length(); col++) {
            char c = str[col];
            if (table.find(c) == table.cend()) {
                p.drawImage(QRect{hmargin + (col * 8), vmargin + (row * 8), 8, 8}, m_letters[(*table.find(' ')).second]);
            } else {
                p.drawImage(QRect{hmargin + (col * 8), vmargin + (row * 8), 8, 8}, m_letters[(*table.find(c)).second]);
            }
        }
    }
}

const Map16Provider::DisplayTiles& Map16Provider::Tiles() {
    return m_tiles[currentIndex];
}

void Map16Provider::setUseText(bool enabled) {
    if (currentIndex == -1)
        return;
    qDebug() << "Change use text: " << currentIndex << " " << enabled;
    usesText[currentIndex] = enabled;
    if (enabled)
        m_tiles[currentIndex].clear();
}

void Map16Provider::addDisplay(int index) {
    qDebug() << "Index is " << index;
    index++;
    m_tiles.insert(index, DisplayTiles());
    m_displays.insert(index, createBase());
    usesText.insert(index, false);
    m_descriptions.insert(index, "");
    currentIndex = index;
    setPixmap(overlay());
}
void Map16Provider::removeDisplay(int index) {
    if (index < 0)
        index = currentIndex;
    if (index < 0 || index >= m_displays.size())
        return;
    m_displays.removeAt(index);
    m_tiles.removeAt(index);
    usesText.removeAt(index);
    m_descriptions.removeAt(index);
    setCurrentlySelected(SIZE_MAX);
    if (m_displays.isEmpty())
        currentIndex = -1;
    else
        currentIndex = std::min(index, static_cast<int>(m_displays.size()) - 1);
    setPixmap(overlay());
}
void Map16Provider::changeDisplay(int index) {
    currentIndex = index;
    setCurrentlySelected(SIZE_MAX);
    setPixmap(overlay());
}

void Map16Provider::cloneDisplay(int index) {
    if (currentIndex < 0 || currentIndex >= m_displays.size())
        return;
    if (index < 0)
        index = currentIndex + 1;
    QPixmap pix = m_displays[currentIndex];
    DisplayTiles tiles = m_tiles[currentIndex];
    bool ut = usesText[currentIndex];
    QString desc = m_descriptions[currentIndex];
    for (auto& t : tiles)
        t.tid = TiledPosition::unique_index++;
    m_displays.insert(index, pix);
    m_tiles.insert(index, tiles);
    usesText.insert(index, ut);
    m_descriptions.insert(index, desc);
    currentIndex = index;
    setCurrentlySelected(SIZE_MAX);
    setPixmap(overlay());
}
SizeSelector Map16Provider::getSelectorSize() {
    return selectorSize;
}
void Map16Provider::setSelectorSize(SizeSelector size) {
    selectorSize = size;
    tileGrid = createGrid();
    setPixmap(overlay());
}

void Map16Provider::serializeDisplays(QVector<DisplayData>& data) {
    qDebug() << "data : " << data.length() << " tiles: " << m_tiles.length();
    Q_ASSERT(data.length() == m_tiles.length());
    for (int i = 0; i < m_tiles.length(); i++) {
        if (usesText[i]) {
            data[i].setUseText(true);
            data[i].setDisplayText(m_descriptions[i]);
            data[i].addTile({0, 0, 0, false});
        }
        else {
            data[i].setUseText(false);
            data[i].setDisplayText("");
            data[i].clearTiles();
            for (auto& t : m_tiles[i]) {
                QPoint newpos = t.pos - QPoint(96, 96);
                data[i].addTile({newpos.x(), newpos.y(), t.map16tileno, t.tile.translucent || t.translucent});
            }
        }
    }
}

void Map16Provider::focusOutEvent(QFocusEvent *event) {
    qDebug() << "Focus out";
    event->accept();
    releaseKeyboard();
}

void Map16Provider::keyPressEvent(QKeyEvent *event) {
    if (m_currentSelected == SIZE_MAX)
        return;
    qDebug() << "Key press event received";
    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        if (event->keyCombination().keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
            m_tiles[currentIndex].clear();
        } else {
            auto index = m_tiles[currentIndex].indexOf(m_currentSelected);
            m_tiles[currentIndex].removeAt(index);
        }
        setCurrentlySelected(SIZE_MAX);
        redrawNoSort();
    } else if (event->key() == Qt::Key_Escape) {
        setCurrentlySelected(SIZE_MAX);
        redrawNoSort();
    }
    releaseKeyboard();
    event->accept();
}

void Map16Provider::deserializeDisplays(const QVector<JSONDisplay>& displays, Map16GraphicsView* view) {

    // clear all to prepare for new displays
    m_displays.clear();
    m_tiles.clear();
    usesText.clear();
    m_descriptions.clear();

    // insert new displays
    setCurrentlySelected(SIZE_MAX);
    currentIndex = 0;
    auto len = displays.length();
    m_displays.reserve(len);
    m_tiles.reserve(len);
    usesText.reserve(len);
    m_descriptions.reserve(len);

    for (auto& d : displays) {
        usesText.append(d.useText);
        m_descriptions.append(d.displaytext);
        QVector<TiledPosition> tiles;
        tiles.reserve(d.tiles.length());
        for (auto& t : d.tiles) {
            QPoint align = QPoint(t.xoff, t.yoff) + QPoint(96, 96);
            int i = t.tilenumber / 16;
            int j = t.tilenumber % 16;
            qDebug() << "Drawing tile " << i << j << t.translucent;
            tiles.append({view->tiles[i][j], align, 0, TiledPosition::unique_index++, t.tilenumber, t.translucent});
        }
        m_tiles.append(tiles);
        m_displays.append(createBase());
        redrawNoSort();
        currentIndex++;
    }
    currentIndex = -1;
}

void Map16Provider::reset() {
    currentIndex = -1;
    setCurrentlySelected(SIZE_MAX);
    usesText.clear();
    m_descriptions.clear();
    m_tiles.clear();
    m_displays.clear();
    tileGrid = createGrid();
    base = createBase();
    setPixmap(overlay());
    currentlyPressed = false;
}
