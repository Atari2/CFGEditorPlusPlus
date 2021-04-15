#include "spritedatamodel.h"

CollectionDataModel::CollectionDataModel()
{
    m_extrabit = false;
    m_name = "";
    memset(m_bytes, 0, 12);
}
const QString& CollectionDataModel::name() {
    return m_name;
}
uint8_t CollectionDataModel::byte(int index) {
    Q_ASSERT(index < 12);
    return m_bytes[index];
}
bool CollectionDataModel::extrabit() {
    return m_extrabit;
}
void CollectionDataModel::setName(const QString& name) {
    m_name = name;
}
void CollectionDataModel::setBytes(const uint8_t* data) {
    memcpy(m_bytes, data, 12);
}
void CollectionDataModel::setByte(uint8_t data, int index) {
    Q_ASSERT(index < 12);
    m_bytes[index] = data;
}
void CollectionDataModel::setExtraBit(bool bit) {
    m_extrabit = bit;
}

CollectionDataModel CollectionDataModel::fromRow(const QVector<QStandardItem*>& row) {
    CollectionDataModel model{};
    model.setName(row[0]->data().toString());
    model.setExtraBit(row[1]->data().toBool());
    for (int i = 0; i < 12; i++) {
        model.m_bytes[i] = (uint8_t)row[i + 2]->data().toUInt();
    }
    return model;
}

QVector<QStandardItem*> CollectionDataModel::fromCollection(const Collection& coll) {
    CollectionDataModel data{};
    data.setExtraBit(coll.extrabit);
    data.setName(coll.name);
    data.setBytes(coll.prop);
    return data.getRow();
}

CollectionDataModel CollectionDataModel::fromIndex(int index, QTableView* view) {
    CollectionDataModel data{};
    data.setName(view->model()->data(view->model()->index(index, 0)).toString());
    data.setExtraBit(view->model()->data(view->model()->index(index, 1)).toBool());
    for (int i = 0; i < 12; i++)
        data.setByte((uint8_t)view->model()->data(view->model()->index(index, i + 2)).toString().toUInt(nullptr, 16), i);
    return data;
}

TileDataModel::TileDataModel() {

}
const QString& TileDataModel::text() {
    return m_text;
}
int TileDataModel::xOffset() {
    return m_xoffset;
}
int TileDataModel::yOffset() {
    return m_yoffset;
}
int TileDataModel::tileNumber() {
    return m_tile_number;
}

void TileDataModel::setUseText(bool enabled) {
    m_use_text = enabled;
}

void TileDataModel::setText(const QString& text) {
    if (m_use_text)
        m_text = text;
}
void TileDataModel::setXOffset(int xoff) {
    m_xoffset = xoff;
}
void TileDataModel::setYOffset(int yoff) {
    m_yoffset = yoff;
}
void TileDataModel::setOffset(int xoff, int yoff) {
    m_xoffset = xoff;
    m_yoffset = yoff;
}
void TileDataModel::setTileNumber(int tilenum) {
    m_tile_number = tilenum;
}


DisplayDataModel::DisplayDataModel() {
}

const QString& DisplayDataModel::description() {
    return m_description;
}
const QVector<TileDataModel>& DisplayDataModel::tiles() {
    return m_tiles;
}
bool DisplayDataModel::extraBit() {
    return m_extrabit;
}
int DisplayDataModel::X() {
    return m_x;
}
int DisplayDataModel::Y() {
    return m_y;
}

void DisplayDataModel::setDescription(const QString& description) {
    m_description = description;
}
void DisplayDataModel::setTiles(const QVector<QMap<QString, QVariant>>& data) {
    m_tiles.clear();
    for (auto& tile : data) {
        TileDataModel t;
        if (tile.contains("UseText")) {
           t.setUseText(true);
           t.setOffset(0, 0);
           t.setTileNumber(0);
        } else {
           t.setUseText(false);
           t.setXOffset(tile["X"].toInt());
           t.setYOffset(tile["Y"].toInt());
           t.setTileNumber(tile["TileNumber"].toInt());
        }
        m_tiles.append(t);
    }
}
void DisplayDataModel::setExtraBit(bool extrabit) {
    m_extrabit = extrabit;
}
void DisplayDataModel::setX(int x) {
    m_x = x;
}
void DisplayDataModel::setY(int y) {
    m_y = y;
}
void DisplayDataModel::setPos(int x, int y) {
    m_x = x;
    m_y = y;

}
DisplayDataModel DisplayDataModel::fromIndex(int index, QTableView* view) {
    DisplayDataModel data{};
    data.setExtraBit(view->model()->data(view->model()->index(index, 0)).toBool());
    data.setX(view->model()->data(view->model()->index(index, 1)).toInt());
    data.setY(view->model()->data(view->model()->index(index, 2)).toInt());
    return data;
}

QVector<QStandardItem*> DisplayDataModel::fromDisplay(const Display& dis) {
    DisplayDataModel data{};
    data.setExtraBit(dis.extrabit);
    data.setDescription(dis.description);
    data.setPos(dis.x, dis.y);
    return data.getRow();
}
