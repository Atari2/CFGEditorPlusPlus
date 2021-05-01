#ifndef JSONSPRITE_H
#define JSONSPRITE_H
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableView>
#include <QFileDialog>
#include "tweak_bytes.h"

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
    int x;
    int y;
    bool useText;
    QString displaytext;
    Display(const QJsonObject& t);
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
    QString m_name;
    QJsonObject obj;
};


#endif // JSONSPRITE_H
