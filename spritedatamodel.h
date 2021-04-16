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

class TileData : QObject {
    Q_OBJECT
private:
    int m_x_offset;
    int m_y_offset;
    int m_tile_number;
public:
    TileData(const TileData& other);
    void setXOffset(int x);
    void setYOffset(int y);
    void setOffset(int x, int y);
    void setOffset(QPoint point);
    void setTileNumber(int number);
    int XOffset();
    int YOffset();
    QPoint Offset();
    int TileNumber();
    TileData& operator=(const TileData& other);
signals:
};

class DisplayData : QObject {
    Q_OBJECT

private:
    bool m_use_text;
    bool m_extra_bit;
    QVector<TileData> m_tiles;
    QString m_description;
    QString m_display_text;
    int m_x;
    int m_y;
    DisplayData();
public:
    DisplayData& operator=(const DisplayData& other);
    DisplayData(const DisplayData& other);
    void setUseText(bool enabled);
    void setExtraBit(bool enabled);
    void setDescription(const QString& description);
    void setX(int x);
    void setY(int y);
    void setPos(QPoint point);
    void setPos(int x, int y);
    void setDisplayText(const QString& displayText);
    void addTile(const TileData& data);
    void removeTile(int index);
    void addTiles(const QVector<TileData>& tiles);

    bool UseText() const;
    bool ExtraBit() const;
    const QVector<TileData>& Tiles() const;
    const QString& Description() const;
    const QString& DisplayText() const;
    int X() const;
    int Y() const;
    QPoint Pos() const;

    static DisplayData blankData();
    static DisplayData cloneData(QStandardItemModel* model, const QString& description, int row, const QString& display_text);
    QVector<QStandardItem*> itemsFromDisplay();
signals:


};

#endif // COLLECTIONDATAMODEL_H
