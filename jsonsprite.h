#ifndef JSONSPRITE_H
#define JSONSPRITE_H
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>

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

struct J1656 {
    uint8_t objclip;
    bool canbejumped;
    bool diesjumped;
    bool hopin;
    bool disapp;
};

struct J1662 {
    uint8_t sprclip;
    bool deathframe;
    bool strdown;
};

struct J166E {
    bool secondpage;
    uint8_t palette;
    bool fireball;
    bool cape;
    bool splash;
    bool lay2;
};

struct J167A {
    bool star;
    bool blk;
    bool offscr;
    bool stunn;
    bool kick;
    bool everyframe;
    bool powerup;
    bool defaultint;
};

struct J1686 {
    bool inedible;
    bool mouth;
    bool ground;
    bool nosprint;
    bool direc;
    bool goalpass;
    bool newspr;
    bool noobjint;
};

struct J190F {
    bool below;
    bool goal;
    bool slidekill;
    bool fivefire;
    bool yupsp;
    bool deathframe;
    bool nosilver;
    bool nostuck;
};

class JsonSprite {
public:
    JsonSprite(const QString& name);
    void deserialize();
    void serialize();
    QString to_text();
    void to_file(const QString& name = QString());
    QString& name();
private:
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
