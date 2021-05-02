#ifndef JSONSPRITE_H
#define JSONSPRITE_H
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableView>
#include <QFileDialog>
#include "tweak_bytes.h"

enum DisplayType {
    XY,
    ExtraByte
};

struct GFXFiles {
    bool separate;
    int sp0;
    int sp1;
    int sp2;
    int sp3;
    GFXFiles(bool sep, int s0, int s1, int s2, int s3);
    GFXFiles(const QJsonObject& g);
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

struct Display {
    QString description;
    QVector<Tile> tiles;
    bool extrabit;
    DisplayType disp_type;
    int x_or_index;
    int y_or_value;
    bool useText;
    QString displaytext;
    Display(const QJsonObject& t, DisplayType type);
    Display(const QString& d, const QVector<Tile>& ts, bool bit, int xx, int yy, bool text, const QString& disp);
    QJsonObject toJson() const;
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
    void serialize();
    void addCollections(QTableView* view);
    void addDisplay(const Display& display);
    void addGfxList(bool sep, int sp0, int sp1, int sp2, int sp3);
    void setMap16(const QString& mapdata);
    QString to_text();
    void to_file(const QString& name = "");
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
    QVector<Display> displays;
    QVector<Collection> collections;
    QVector<GFXFiles> gfxfiles;
    QString m_name;
    QJsonObject obj;
    DisplayType dispType;
};


#endif // JSONSPRITE_H
