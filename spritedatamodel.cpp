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

void TileData::setXOffset(int x) {
    m_x_offset = x;
}
void TileData::setYOffset(int y) {
    m_y_offset = y;
}
void TileData::setOffset(int x, int y) {
    m_x_offset = x;
    m_y_offset = y;
}
void TileData::setOffset(QPoint point) {
    m_x_offset = point.x();
    m_y_offset = point.y();
}
void TileData::setTileNumber(int number) {
    m_tile_number = number;
}
int TileData::XOffset() {
    return m_x_offset;
}
int TileData::YOffset() {
    return m_y_offset;
}
QPoint TileData::Offset() {
    return QPoint(m_x_offset, m_y_offset);
}
int TileData::TileNumber() {
    return m_tile_number;
}

TileData& TileData::operator=(const TileData& other) {
    m_tile_number = other.m_tile_number;
    m_x_offset = other.m_x_offset;
    m_y_offset = other.m_y_offset;
    return *this;
}

TileData::TileData(const TileData& other) {
    this->operator=(other);
}

void DisplayData::setUseText(bool enabled) {
    m_use_text = enabled;
}
void DisplayData::setExtraBit(bool enabled) {
    m_extra_bit = enabled;
}
void DisplayData::setDescription(const QString& description) {
    m_description = description;
}
void DisplayData::setX(int x) {
    m_x = x;
}
void DisplayData::setY(int y) {
    m_y = y;
}
void DisplayData::setPos(QPoint point) {
    m_x = point.x();
    m_y = point.y();
}
void DisplayData::setPos(int x, int y) {
    m_x = x;
    m_y = y;
}
void DisplayData::setDisplayText(const QString& displayText) {
    qDebug() << "Trying setting text " << (m_use_text ? "True" : "False") << " " << displayText;
    if (m_use_text)
        m_display_text = displayText;
}
void DisplayData::addTile(const TileData& data) {
    m_tiles.append(data);
}
void DisplayData::removeTile(int index) {
    m_tiles.removeAt(index);
}
void DisplayData::addTiles(const QVector<TileData>& tiles) {
    m_tiles.append(tiles);
}

bool DisplayData::UseText() const {
    return m_use_text;
}
bool DisplayData::ExtraBit() const {
    return m_extra_bit;
}
const QVector<TileData>& DisplayData::Tiles() const {
    return m_tiles;
}
const QString& DisplayData::Description() const {
    return m_description;
}
const QString& DisplayData::DisplayText() const {
    return m_display_text;
}
int DisplayData::X() const {
    return m_x;
}
int DisplayData::Y() const {
    return m_y;
}
QPoint DisplayData::Pos() const {
    return QPoint(m_x, m_y);
}


DisplayData::DisplayData(const DisplayData& other) {
    this->operator=(other);
}

DisplayData& DisplayData::operator=(const DisplayData& other) {
    m_extra_bit = other.m_extra_bit;
    m_x = other.m_x;
    m_y = other.m_y;
    m_tiles.reserve(other.m_tiles.size());
    std::for_each(other.m_tiles.cbegin(), other.m_tiles.cend(), [&](const TileData& tile) {
        m_tiles.append(tile);
    });
    m_description = other.m_description;
    m_use_text = other.m_use_text;
    m_display_text = other.m_display_text;
    return *this;
}

DisplayData::DisplayData() {

}

DisplayData DisplayData::blankData() {
    DisplayData data;
    data.m_extra_bit = false;
    data.m_x = 0;
    data.m_y = 0;
    data.m_tiles = QVector<TileData>();
    data.m_use_text = false;
    data.m_display_text = "";
    data.m_description = "";
    return data;
}
DisplayData DisplayData::cloneData(QStandardItemModel* model, const QString& description, int row, const QString& display_text) {
    DisplayData data;
    data.m_extra_bit = model->item(row, 0)->data().toString() == "True";
    data.m_x = model->item(row, 1)->data().toInt();
    data.m_y = model->item(row, 2)->data().toInt();
    data.m_description = description;
    data.m_use_text = display_text.length() != 0;
    data.m_display_text = display_text;
    return data;
}

QVector<QStandardItem*> DisplayData::itemsFromDisplay() {
    QVector<QStandardItem*> vec;
    vec.append(new QStandardItem(m_extra_bit ? "True" : "False"));
    vec.append(new QStandardItem(QString::asprintf("%d", m_x)));
    vec.append(new QStandardItem(QString::asprintf("%d", m_y)));
    return vec;
}
