#ifndef COLLECTIONDATAMODEL_H
#define COLLECTIONDATAMODEL_H

#include <QTableView>
#include <QStandardItemModel>
#include "jsonsprite.h"

class CollectionDataModel
{
private:
    uint8_t m_bytes[12];
    QString m_name;
    bool m_extrabit;
public:
    CollectionDataModel();
    const QString& name();
    uint8_t byte(int index);
    bool extrabit();
    void setName(const QString& name);
    void setByte(uint8_t data, int index);
    void setBytes(const uint8_t* data);
    void setExtraBit(bool bit);
    QVector<QStandardItem*> getRow(void* ui = nullptr);
    static CollectionDataModel fromRow(const QVector<QStandardItem*>& row);
    static CollectionDataModel fromIndex(int index, QTableView* view);
    static QVector<QStandardItem*> fromCollection(const Collection& coll);
};

#endif // COLLECTIONDATAMODEL_H
