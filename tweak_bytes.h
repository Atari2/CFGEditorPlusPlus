#ifndef TWEAK_BYTES_H
#define TWEAK_BYTES_H
#include <QJsonObject>

struct J1656 {
    uint8_t objclip;
    bool canbejumped;
    bool diesjumped;
    bool hopin;
    bool disapp;
    J1656() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
};

struct J1662 {
    uint8_t sprclip;
    bool deathframe;
    bool strdown;
    J1662() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
};

struct J166E {
    bool secondpage;
    uint8_t palette;
    bool fireball;
    bool cape;
    bool splash;
    bool lay2;
    J166E() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
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
    J167A() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
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
    J1686() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
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
    J190F() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
};

#endif // TWEAK_BYTES_H
