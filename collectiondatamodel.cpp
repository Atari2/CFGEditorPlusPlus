#include "collectiondatamodel.h"

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
