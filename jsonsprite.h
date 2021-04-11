#ifndef JSONSPRITE_H
#define JSONSPRITE_H
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include "tweak_bytes.h"

struct Tile {
    int xoff;
    int yoff;
    int tilenumber;
    QString text;
    Tile(const QJsonObject& d);
    QJsonObject toJson() const;
};

struct Display {
    QString description;
    QVector<Tile> tiles;
    bool extrabit;
    int x;
    int y;
    QString displayText;
    bool useText;
    Display(const QJsonObject& t);
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
    QString to_text();
    void to_file(const QString& name = QString());
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
