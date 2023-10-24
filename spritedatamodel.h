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
    TileData(int x_off, int y_off, int tile_num);
    TileData(const TileData& other);
    void setXOffset(int x);
    void setYOffset(int y);
    void setOffset(int x, int y);
    void setOffset(QPoint point);
    void setTileNumber(int number);
    int XOffset() const;
    int YOffset() const;
    QPoint Offset() const;
    int TileNumber() const;
    TileData& operator=(const TileData& other);
signals:
};

class SingleGFXFileData : QObject {
    Q_OBJECT

private:
    int m_value{0x7f};
    bool m_separate{false};

public:
    SingleGFXFileData() = default;
    SingleGFXFileData(bool separate, int value);
    SingleGFXFileData(const SingleGFXFileData&);
    SingleGFXFileData& operator=(const SingleGFXFileData&);
    SingleGFXFileData& operator=(const SingleGFXFile& data);
    int Value() const;
    bool Separate() const;
    void setValue(int value);
    void setSeparate(bool separate);
    QStandardItem* separateItem() const;
    QStandardItem* valueItem() const;
};

class GFXInfoData : QObject {
    Q_OBJECT

private:
    SingleGFXFileData m_sp0{};
    SingleGFXFileData m_sp1{};
    SingleGFXFileData m_sp2{};
    SingleGFXFileData m_sp3{};
public:
    GFXInfoData() = default;
    GFXInfoData(const GFXInfoData&);
    GFXInfoData& operator=(const GFXInfoData&);
    GFXInfoData& operator=(const GFXInfo&);
    const SingleGFXFileData& sp0() const;
    const SingleGFXFileData& sp1() const;
    const SingleGFXFileData& sp2() const;
    const SingleGFXFileData& sp3() const;
    void setSp0(SingleGFXFileData gfxdata);
    void setSp1(SingleGFXFileData gfxdata);
    void setSp2(SingleGFXFileData gfxdata);
    void setSp3(SingleGFXFileData gfxdata);
    QVector<QStandardItem*> itemsFromGFXInfo() const;
    static GFXInfoData fromModel(QStandardItemModel* model, int row);
};

class DisplayData : QObject {
    Q_OBJECT

private:
    bool m_use_text;
    bool m_extra_bit;
    QVector<TileData> m_tiles;
    QString m_description;
    QString m_display_text;
    int m_x_or_index;
    int m_y_or_value;
    GFXInfoData m_gfxinfo;
    DisplayData();
public:
    DisplayData& operator=(const DisplayData& other);
    DisplayData(const DisplayData& other);
    DisplayData(const Display& other);
    void setUseText(bool enabled);
    void setExtraBit(bool enabled);
    void setDescription(const QString& description);
    void setXOrIndex(int x);
    void setYOrValue(int y);
    void setPosOrExtra(QPoint point);
    void setPosOrExtra(int x, int y);
    void setDisplayText(const QString& displayText);
    void setGFXInfo(const GFXInfoData& gfxInfo);
    void setSeparate(bool separate, int spnum);
    void setGfxInfoValue(int value, int spnum);
    void addTile(const TileData& data);
    void clearTiles();

    bool UseText() const;
    bool ExtraBit() const;
    const QVector<TileData>& Tiles() const;
    const GFXInfoData& GFXInfo() const;
    const QString& Description() const;
    const QString& DisplayText() const;
    int XOrIndex() const;
    int YOrValue() const;
    QPoint PosOrExtra() const;

    static DisplayData blankData();
    static DisplayData cloneData(QStandardItemModel* model, QStandardItemModel* gfxModel, const QString& description, int row, const QString& display_text);
    QVector<QStandardItem*> itemsFromDisplay();
signals:


};

#endif // COLLECTIONDATAMODEL_H
