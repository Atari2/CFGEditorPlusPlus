#ifndef JSONSPRITE_H
#define JSONSPRITE_H
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableView>
#include <QFileDialog>
#include <QMessageBox>
#include "tweak_bytes.h"

enum DisplayType {
    XY,
    ExtraByte
};

struct SingleGFXFile {
    bool separate{false};
    int value{0x7f};
    SingleGFXFile() = default;
    SingleGFXFile(bool separate, int value);
    SingleGFXFile(const QJsonObject& g);
    QJsonObject toJson() const;
};

struct GFXInfo {
    SingleGFXFile sp0{};
    SingleGFXFile sp1{};
    SingleGFXFile sp2{};
    SingleGFXFile sp3{};
    GFXInfo() = default;
    GFXInfo(SingleGFXFile s0, SingleGFXFile s1, SingleGFXFile s2, SingleGFXFile s3);
    GFXInfo(const QJsonObject& g);
    QJsonObject toJson() const;
};

struct Tile {
    int xoff;
    int yoff;
    int tilenumber;
    Tile(int x, int y, int tileno);
    Tile(const QJsonObject& d);
    QJsonObject toJson() const;
};

struct JSONDisplay {
    QString description;
    QVector<Tile> tiles;
    bool extrabit;
    int x_or_index;
    int y_or_value;
    bool useText;
    QString displaytext;
    GFXInfo gfxinfo{};
    JSONDisplay(const QJsonObject& t, DisplayType type);
    JSONDisplay(const QString& d, const QVector<Tile>& ts, bool bit, int xx, int yy, bool text, const QString& disp, const GFXInfo& info);
    QJsonObject toJson(DisplayType type) const;
};

struct Collection {
    QString name;
    bool extrabit;
    uint8_t prop[12];
    Collection(const QJsonObject& c);
    QJsonObject toJson() const;
};

class JsonSprite {
public:
    JsonSprite();
    void reset();
    void from_file(const QString& name);
    void deserialize();
    void deserialize_cfg(QFile& file);
    void serialize();
    QByteArray serialize_cfg();
    void addCollections(QTableView* view);
    void addDisplay(const JSONDisplay& display);
    void setMap16(const QString& mapdata);
    QByteArray to_text(const QString& filename);
    void to_file(QString name = "");
    QString& name();
    J1656 t1656;
    J1662 t1662;
    J166E t166e;
    J167A t167a;
    J1686 t1686;
    J190F t190f;
    QString asmfile;
    uint8_t actlike;
    uint8_t type;
    uint8_t extraProp1;
    uint8_t extraProp2;
    int addbcountclear;
    int addbcountset;
    QString map16;
    QVector<JSONDisplay> displays;
    QVector<Collection> collections;
    QString m_name;
    QJsonObject obj;
    DisplayType dispType;
};


#endif // JSONSPRITE_H
