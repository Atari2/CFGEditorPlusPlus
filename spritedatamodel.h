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

class TileDataModel
{
private:
    int m_xoffset = 0;
    int m_yoffset = 0;
    int m_tile_number = 0;
    QString m_text{};
public:
    TileDataModel();
    const QString& text();
    int xOffset();
    int yOffset();
    int tileNumber();

    void setText(const QString& text);
    void setXOffset(int xoff);
    void setYOffset(int yoff);
    void setOffset(int xoff, int yoff);
    void setTileNumber(int tilenum);
};

class DisplayDataModel
{
private:
    QString m_description{};
    QVector<TileDataModel> m_tiles{};
    bool m_extrabit = false;
    int m_x = 0;
    int m_y = 0;
public:
    DisplayDataModel();
    const QString& description();
    const QVector<TileDataModel>& tiles();
    bool extraBit();
    int X();
    int Y();

    void setDescription(const QString& description);
    void setTiles(const QVector<QStandardItem*>& data);
    void setExtraBit(bool extrabit);
    void setX(int x);
    void setY(int y);
    void setPos(int x, int y);
    QVector<QStandardItem*> getRow(void* ui = nullptr);
    static DisplayDataModel fromIndex(int index, QTableView* view);
    static QVector<QStandardItem*> fromDisplay(const Display& dis);
};

#endif // COLLECTIONDATAMODEL_H
