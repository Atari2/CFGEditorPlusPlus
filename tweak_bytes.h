#ifndef TWEAK_BYTES_H
#define TWEAK_BYTES_H
#include <QJsonObject>

#define CFGPPTWEAKPROP(name, type, size) \
type name : size; \
void set_##name(type value) { this->name = value; }

struct J1656 {
    uint8_t objclip : 4;
    CFGPPTWEAKPROP(canbejumped, bool, 1);
    CFGPPTWEAKPROP(diesjumped, bool, 1);
    CFGPPTWEAKPROP(hopin, bool, 1);
    CFGPPTWEAKPROP(disapp, bool, 1);
    J1656() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
    constexpr bool operator==(const J1656&) const = default;
};

static_assert(sizeof(J1656) == 1, "Size of tweak byte must be 1 byte");

struct J1662 {
    uint8_t sprclip : 6;
    CFGPPTWEAKPROP(deathframe, bool, 1);
    CFGPPTWEAKPROP(strdown, bool, 1);
    J1662() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
    constexpr bool operator==(const J1662&) const = default;
};

static_assert(sizeof(J1662) == 1, "Size of tweak byte must be 1 byte");

struct J166E {
    CFGPPTWEAKPROP(secondpage, bool, 1);
    uint8_t palette : 3;
    CFGPPTWEAKPROP(fireball, bool, 1);
    CFGPPTWEAKPROP(cape, bool, 1);
    CFGPPTWEAKPROP(splash, bool, 1);
    CFGPPTWEAKPROP(lay2, bool, 1);
    J166E() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
    constexpr bool operator==(const J166E&) const = default;
};

static_assert(sizeof(J166E) == 1, "Size of tweak byte must be 1 byte");

struct J167A {
    CFGPPTWEAKPROP(star, bool, 1);
    CFGPPTWEAKPROP(blk, bool, 1);
    CFGPPTWEAKPROP(offscr, bool, 1);
    CFGPPTWEAKPROP(stunn, bool, 1);
    CFGPPTWEAKPROP(kick, bool, 1);
    CFGPPTWEAKPROP(everyframe, bool, 1);
    CFGPPTWEAKPROP(powerup, bool, 1);
    CFGPPTWEAKPROP(defaultint, bool, 1);
    J167A() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
    constexpr bool operator==(const J167A&) const = default;
};

static_assert(sizeof(J167A) == 1, "Size of tweak byte must be 1 byte");

struct J1686 {
    CFGPPTWEAKPROP(inedible, bool, 1);
    CFGPPTWEAKPROP(mouth, bool, 1);
    CFGPPTWEAKPROP(ground, bool, 1);
    CFGPPTWEAKPROP(nosprint, bool, 1);
    CFGPPTWEAKPROP(direc, bool, 1);
    CFGPPTWEAKPROP(goalpass, bool, 1);
    CFGPPTWEAKPROP(newspr, bool, 1);
    CFGPPTWEAKPROP(noobjint, bool, 1);
    J1686() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
    constexpr bool operator==(const J1686&) const = default;
};

static_assert(sizeof(J1686) == 1, "Size of tweak byte must be 1 byte");

struct J190F {
    CFGPPTWEAKPROP(below, bool, 1);
    CFGPPTWEAKPROP(goal, bool, 1);
    CFGPPTWEAKPROP(slidekill, bool, 1);
    CFGPPTWEAKPROP(fivefire, bool, 1);
    CFGPPTWEAKPROP(yupsp, bool, 1);
    CFGPPTWEAKPROP(deathframe, bool, 1);
    CFGPPTWEAKPROP(nosilver, bool, 1);
    CFGPPTWEAKPROP(nostuck, bool, 1);
    J190F() = default;
    void from_json(const QJsonObject& byte);
    void from_byte(uint8_t byte);
    uint8_t to_byte() const;
    QJsonObject to_json() const;
    constexpr bool operator==(const J190F&) const = default;
};

static_assert(sizeof(J190F) == 1, "Size of tweak byte must be 1 byte");

#endif // TWEAK_BYTES_H
