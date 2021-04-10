#include "jsonsprite.h"

Display::Display(const QJsonObject& d) {
    description = d["Description"].toString();
    displayText = d["DisplayText"].toString();
    extrabit = d["ExtraBit"].toBool();
    x = d["X"].toInt();
    y = d["Y"].toInt();
    useText = d["UseText"].toBool();
    auto tilesArr = d["Tiles"].toArray();
    tiles.reserve(tilesArr.size());
    std::for_each(tilesArr.cbegin(), tilesArr.cend(), [&](auto& t) {
        tiles.push_back(Tile(t.toObject()));
    });
}

QJsonObject Display::toJson() const {
    QJsonObject obj{};
    obj["Description"] = description;
    obj["DisplayText"] = displayText;
    obj["ExtraBit"] = extrabit;
    obj["X"] = x;
    obj["Y"] = y;
    obj["UseText"] = useText;
    QJsonArray tilesArr{};
    std::for_each(tiles.cbegin(), tiles.cend(), [&](auto& t) {
        tilesArr.append(t.toJson());
    });
    obj["Tiles"] = tilesArr;
    return obj;
}

Tile::Tile(const QJsonObject& t) {
    xoff = t["X offset"].toInt();
    yoff = t["Y offset"].toInt();
    tilenumber = t["map16 tile"].toInt();
    if (t.contains("Text")) {
        text = t["Text"].toString();
    }
}

QJsonObject Tile::toJson() const {
    QJsonObject obj{};
    obj["X offset"] = xoff;
    obj["Y offset"] = yoff;
    obj["map16 tile"] = tilenumber;
    if (text.length() != 0)
        obj["Text"] = text;
    return obj;
}

Collection::Collection(const QJsonObject& c) {
    name = c["Name"].toString();
    extrabit = c["ExtraBit"].toBool();
    for (int i = 1; i <= 12; i++) {
        QString str = QString::asprintf("Extra Property Byte %d", i);
        if (c.contains(str))
            prop[i - 1] = c[str].toInt();
        else
            prop[i - 1] = 0;
    }
}

QJsonObject Collection::toJson() const {
    QJsonObject obj{};
    obj["Name"] = name;
    obj["ExtraBit"] = extrabit;
    for (int i = 1; i <= 12; i++) {
        QString str = QString::asprintf("Extra Property Byte %d", i);
        obj[str] = prop[i - 1];
    }
    return obj;
}

JsonSprite::JsonSprite(const QString& name) : m_name(name) {
    QFile file{m_name};
    file.open(QFile::OpenModeFlag::ReadOnly);
    auto doc = QJsonDocument::fromJson(file.readAll());
    obj = doc.object();
    deserialize();
}

void JsonSprite::deserialize() {
    asmfile = obj["AsmFile"].toString();
    actlike = obj["ActLike"].toInt();
    type = obj["Type"].toInt();
    extraProp1 = obj["Extra Property Byte 1"].toInt();
    extraProp2 = obj["Extra Property Byte 2"].toInt();
    addbcountclear = obj["Additional Byte Count (extra bit clear)"].toInt();
    addbcountset = obj["Additional Byte Count (extra bit set)"].toInt();
    map16 = obj["Map16"].toString();
    auto dispArr = obj["Displays"].toArray();
    auto collArr = obj["Collection"].toArray();
    displays.reserve(dispArr.size());
    collections.reserve(collArr.size());
    std::for_each(dispArr.cbegin(), dispArr.cend(), [&](auto& d) {
        displays.push_back(Display(d.toObject()));
    });
    std::for_each(collArr.cbegin(), collArr.cend(), [&](auto& c) {
        collections.push_back(Collection(c.toObject()));
    });
}

void JsonSprite::serialize() {
    obj["AsmFile"] = asmfile;
    obj["ActLike"] = actlike;
    obj["Type"] = type;
    obj["Extra Property Byte 1"] = extraProp1;
    obj["Extra Property Byte 2"] = extraProp2;
    obj["Map16"] = map16;
    QJsonArray dispArr{};
    QJsonArray collArr{};
    std::for_each(displays.cbegin(), displays.cend(), [&](auto& d) {
        dispArr.append(d.toJson());
    });
    std::for_each(collections.cbegin(), collections.cend(), [&](auto& c) {
        collArr.append(c.toJson());
    });
    obj["Displays"] = dispArr;
    obj["Collection"] = collArr;
}

QString JsonSprite::to_text() {
    serialize();
    QJsonDocument doc{obj};
    return QString(doc.toJson().toStdString().c_str());
}

QString& JsonSprite::name() {
    return m_name;
}

void JsonSprite::to_file(const QString& name) {
    // TODO: do something other than return when trying to print empty
    if (obj.isEmpty()) return;
    if (name.length() == 0) {
        QFile outFile{m_name};
        outFile.open(QFile::OpenModeFlag::Truncate | QFile::OpenModeFlag::Text | QFile::OpenModeFlag::WriteOnly);
        outFile.write(to_text().toStdString().c_str());
    } else {
        QFile outFile{name};
        outFile.open(QFile::OpenModeFlag::Truncate | QFile::OpenModeFlag::Text | QFile::OpenModeFlag::WriteOnly);
        outFile.write(to_text().toStdString().c_str());
    }
}
