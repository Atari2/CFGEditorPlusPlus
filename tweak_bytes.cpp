#include "tweak_bytes.h"

void J1656::from_json(const QJsonObject& byte) {
    objclip = byte["Object Clipping"].toInt() & 0x0F;
    canbejumped = byte["Can be jumped on"].toBool();
    diesjumped = byte["Dies when jumped on"].toBool();
    hopin = byte["Hop in/kick shells"].toBool();
    disapp = byte["Disappear in a cloud of smoke"].toBool();
}

void J1656::from_byte(uint8_t byte) {
    objclip = byte & 0x0F;
    canbejumped = (byte & 0x10) == 0x10;
    diesjumped = (byte & 0x20) == 0x20;
    hopin = (byte & 0x40) == 0x40;
    disapp = (byte & 0x80 ) == 0x80;
}

void J1662::from_json(const QJsonObject& byte) {
    sprclip = byte["Sprite Clipping"].toInt();
    deathframe = byte["Use shell as death frame"].toBool();
    strdown = byte["Fall straight down when killed"].toBool();
}

void J1662::from_byte(uint8_t byte) {
    sprclip = byte & 0x3F;
    deathframe = (byte & 0x40) == 0x40;
    strdown = (byte & 0x80) == 0x80;
}

void J166E::from_json(const QJsonObject& byte) {
    secondpage = byte["Use second graphics page"].toBool();
    palette = byte["Palette"].toInt();
    fireball = byte["Disable fireball killing"].toBool();
    cape = byte["Disable cape killing"].toBool();
    splash = byte["Disable water splash"].toBool();
    lay2 = byte["Don't interact with Layer 2"].toBool();
}

void J166E::from_byte(uint8_t byte) {
    secondpage = (byte & 0x01) == 1;
    palette = (byte & 0x0E) >> 1;
    fireball = (byte & 0x10) == 0x10;
    cape = (byte & 0x20) == 0x20;
    splash = (byte & 0x40) == 0x40;
    lay2 = (byte & 0x80) == 0x80;
}

void J167A::from_json(const QJsonObject& byte) {
    star = byte["Don't disable clipping when starkilled"].toBool();
    blk = byte["Invincible to star/cape/fire/bounce blk."].toBool();
    offscr = byte["Process when off screen"].toBool();
    stunn = byte["Don't change into shell when stunned"].toBool();
    kick = byte["Can't be kicked like shell"].toBool();
    everyframe = byte["Process interaction with Mario every frame"].toBool();
    powerup = byte["Gives power-up when eaten by yoshi"].toBool();
    defaultint = byte["Don't use default interaction with Mario"].toBool();
}

void J167A::from_byte(uint8_t byte) {
    star = (byte & 0x01) == 1;
    blk = (byte & 0x02) == 0x02;
    offscr = (byte & 0x04) == 0x04;
    stunn = (byte & 0x08) == 0x08;
    kick = (byte & 0x10) == 0x10;
    everyframe = (byte & 0x20) == 0x20;
    powerup = (byte & 0x40) == 0x40;
    defaultint = (byte & 0x80) == 0x80;
}

void J1686::from_json(const QJsonObject& byte) {
    inedible = byte["Inedible"].toBool();
    mouth = byte["Stay in Yoshi's mouth"].toBool();
    ground = byte["Weird ground behaviour"].toBool();
    nosprint = byte["Don't interact with other sprites"].toBool();
    direc = byte["Don't change direction if touched"].toBool();
    goalpass = byte["Don't turn into coin when goal passed"].toBool();
    newspr = byte["Spawn a new sprite"].toBool();
    noobjint = byte["Don't interact with objects"].toBool();
}

void J1686::from_byte(uint8_t byte) {
    inedible = (byte & 0x01) == 1;
    mouth = (byte & 0x02) == 0x02;
    ground = (byte & 0x04) == 0x04;
    nosprint = (byte & 0x08) == 0x08;
    direc = (byte & 0x10) == 0x10;
    goalpass = (byte & 0x20) == 0x20;
    newspr = (byte & 0x40) == 0x40;
    noobjint = (byte & 0x80) == 0x80;
}

void J190F::from_json(const QJsonObject& byte) {
    below = byte["Make platform passable from below"].toBool();
    goal = byte["Don't erase when goal passed"].toBool();
    slidekill = byte["Can't be killed by sliding"].toBool();
    fivefire = byte["Takes 5 fireballs to kill"].toBool();
    yupsp = byte["Can be jumped on with upwards Y speed"].toBool();
    deathframe = byte["Death frame two tiles high"].toBool();
    nosilver = byte["Don't turn into a coin with silver POW"].toBool();
    nostuck = byte["Don't get stuck in walls (carryable sprites)"].toBool();
}

void J190F::from_byte(uint8_t byte) {
    below = (byte & 0x01) == 1;
    goal = (byte & 0x02) == 0x02;
    slidekill = (byte & 0x04) == 0x04;
    fivefire = (byte & 0x08) == 0x08;
    yupsp = (byte & 0x10) == 0x10;
    deathframe = (byte & 0x20) == 0x20;
    nosilver = (byte & 0x40) == 0x40;
    nostuck = (byte & 0x80) == 0x80;
}

uint8_t J1656::to_byte() const {
    uint8_t val = 0;
    val |= objclip & 0x0F;
    val |= (canbejumped << 4);
    val |= (diesjumped << 5);
    val |= (hopin << 6);
    val |= (disapp << 7);
    return val;
}

uint8_t J1662::to_byte() const {
    uint8_t val = 0;
    val |= sprclip & 0x3F;
    val |= (deathframe << 6);
    val |= (strdown << 7);
    return val;
}

uint8_t J166E::to_byte() const {
    uint8_t val = 0;
    val |= (uint8_t)secondpage;
    val |= ((palette & 0x07) << 1);
    val |= (fireball << 4);
    val |= (cape << 5);
    val |= (splash << 6);
    val |= (lay2 << 7);
    return val;
}

uint8_t J167A::to_byte() const {
    uint8_t val = 0;
    val |= (uint8_t)star;
    val |= (blk << 1);
    val |= (offscr << 2);
    val |= (stunn << 3);
    val |= (kick << 4);
    val |= (everyframe << 5);
    val |= (powerup << 6);
    val |= (defaultint << 7);
    return val;
}

uint8_t J1686::to_byte() const {
    uint8_t val = 0;
    val |= (uint8_t)inedible;
    val |= (mouth << 1);
    val |= (ground << 2);
    val |= (nosprint << 3);
    val |= (direc << 4);
    val |= (goalpass << 5);
    val |= (newspr << 6);
    val |= (noobjint << 7);
    return val;
}

uint8_t J190F::to_byte() const {
    uint8_t val = 0;
    val |= (uint8_t)below;
    val |= (goal << 1);
    val |= (slidekill << 2);
    val |= (fivefire << 3);
    val |= (yupsp << 4);
    val |= (deathframe << 5);
    val |= (nosilver << 6);
    val |= (nostuck << 7);
    return val;
}

QJsonObject J1656::to_json() const {
    QJsonObject obj{};
    obj["Object Clipping"] = objclip;
    obj["Can be jumped on"] = canbejumped;
    obj["Dies when jumped on"] = diesjumped;
    obj["Hop in/kick shells"] = hopin;
    obj["Disappear in a cloud of smoke"] = disapp;
    return obj;
}

QJsonObject J1662::to_json() const {
    QJsonObject obj{};
    obj["Sprite Clipping"] = sprclip;
    obj["Use shell as death frame"] = deathframe;
    obj["Fall straight down when killed"] = strdown;
    return obj;
}

QJsonObject J166E::to_json() const {
    QJsonObject obj{};
    obj["Use second graphics page"] = secondpage;
    obj["Palette"] = palette;
    obj["Disable fireball killing"] = fireball;
    obj["Disable cape killing"] = cape;
    obj["Disable water splash"] = splash;
    obj["Don't interact with Layer 2"] = lay2;
    return obj;
}

QJsonObject J167A::to_json() const {
    QJsonObject obj{};
    obj["Don't disable clipping when starkilled"] = star;
    obj["Invincible to star/cape/fire/bounce blk."] = blk;
    obj["Process when off screen"] = offscr;
    obj["Don't change into shell when stunned"] = stunn;
    obj["Can't be kicked like shell"] = kick;
    obj["Process interaction with Mario ever frame"] = everyframe;
    obj["Gives power-up when eaten by yoshi"] = powerup;
    obj["Don't use default interaction with Mario"] = defaultint;
    return obj;
}

QJsonObject J1686::to_json() const {
    QJsonObject obj{};
    obj["Inedible"] = inedible;
    obj["Stay in Yoshi's mouth"] = mouth;
    obj["Weird ground behaviour"] = ground;
    obj["Don't interact with other sprites"] = nosprint;
    obj["Don't change direction if touched"] = direc;
    obj["Don't turn into coin when goal passed"] = goalpass;
    obj["Spawn a new sprite"] = newspr;
    obj["Don't interact with objects"] = noobjint;
    return obj;
}

QJsonObject J190F::to_json() const {
    QJsonObject obj{};
    obj["Make platform passable from below"] = below;
    obj["Don't erase when goal passed"] = goal;
    obj["Can't be killed by sliding"] = slidekill;
    obj["Takes 5 fireballs to kill"] = fivefire;
    obj["Can be jumped on with upwards Y speed"] = yupsp;
    obj["Death frame two tiles high"] = deathframe;
    obj["Don't turn into a coin with silver POW"] = nosilver;
    obj["Don't get stuck in walls (carryable sprites)"] = nostuck;
    return obj;
}
