#include "jsonsprite.h"

Display::Display(const QJsonObject& d, DisplayType type) {
    description = d["Description"].toString();
    extrabit = d["ExtraBit"].toBool();
    if (auto it = d.find("GFXInfo"); it != d.constEnd()) {
        gfxinfo = GFXInfo{it->toObject()};
    }
    if (type == DisplayType::XY) {
        x_or_index = d["X"].toInt();
        y_or_value = d["Y"].toInt();
    } else {
        x_or_index = d["Index"].toInt();
        y_or_value = d["Value"].toInt();
    }
    useText = d["UseText"].toBool();
    auto tilesArr = d["Tiles"].toArray();
    if (useText) {
        tiles.reserve(1);
        tiles.append(Tile(tilesArr[0].toObject()));
        displaytext = d["DisplayText"].toString();
    } else {
        tiles.reserve(tilesArr.size());
        std::for_each(tilesArr.cbegin(), tilesArr.cend(), [&](auto& t) {
            tiles.append(Tile(t.toObject()));
        });
    }
}

Display::Display(const QString& d, const QVector<Tile>& ts, bool bit, int xx, int yy, bool text, const QString& disp, const GFXInfo& info) :
    description(d),
    extrabit(bit),
    x_or_index(xx),
    y_or_value(yy),
    useText(text),
    displaytext(disp),
    gfxinfo(info)
{
   tiles.append(ts);
}

QJsonObject Display::toJson(DisplayType type) const {
    QJsonObject obj{};
    obj["Description"] = description;
    obj["ExtraBit"] = extrabit;
    if (type == DisplayType::XY) {
        obj["X"] = x_or_index;
        obj["Y"] = y_or_value;
    } else {
        obj["Index"] = x_or_index;
        obj["Value"] = y_or_value;
    }
    obj["UseText"] = useText;
    QJsonArray tilesArr{};
    if (useText) {
        obj["DisplayText"] = displaytext;
        tilesArr.append(tiles[0].toJson());
    } else {
        obj["DisplayText"] = "";
        std::for_each(tiles.cbegin(), tiles.cend(), [&](auto& t) {
            tilesArr.append(t.toJson());
        });
    }
    obj["Tiles"] = tilesArr;
    obj["GFXInfo"] = gfxinfo.toJson();
    return obj;
}

SingleGFXFile::SingleGFXFile(bool separate, int value) : separate{separate}, value{value} {

}
SingleGFXFile::SingleGFXFile(const QJsonObject& g) {
    separate = g["Separate"].toBool();
    value = g["Value"].toInt();
}
QJsonObject SingleGFXFile::toJson() const {
    QJsonObject obj{};
    obj["Separate"] = separate;
    obj["Value"] = value;
    return obj;
}

GFXInfo::GFXInfo(SingleGFXFile s0, SingleGFXFile s1, SingleGFXFile s2, SingleGFXFile s3) :
    sp0(s0),
    sp1(s1),
    sp2(s2),
    sp3(s3)
{

}

GFXInfo::GFXInfo(const QJsonObject& g) : sp0{false, 0x7F}, sp1{false, 0x7F}, sp2{false, 0x7F}, sp3{false, 0x7F}  {
    if (auto it = g.find("0"); it != g.constEnd()) {
        sp0 = SingleGFXFile{it->toObject()};
    }
    if (auto it = g.find("1"); it != g.constEnd()) {
        sp1 = SingleGFXFile{it->toObject()};
    }
    if (auto it = g.find("2"); it != g.constEnd()) {
        sp2 = SingleGFXFile{it->toObject()};
    }
    if (auto it = g.find("3"); it != g.constEnd()) {
        sp3 = SingleGFXFile{it->toObject()};
    }
}

QJsonObject GFXInfo::toJson() const {
    QJsonObject obj{};
    if (sp0.value != 0x7F)
        obj["0"] = sp0.toJson();
    if (sp1.value != 0x7F)
        obj["1"] = sp1.toJson();
    if (sp2.value != 0x7F)
        obj["2"] = sp2.toJson();
    if (sp3.value != 0x7F)
        obj["3"] = sp3.toJson();
    return obj;
}


Tile::Tile(const QJsonObject& t) {
    xoff = t["X offset"].toInt();
    yoff = t["Y offset"].toInt();
    tilenumber = t["map16 tile"].toInt();
}

Tile::Tile(int x, int y, int tileno) : xoff(x), yoff(y), tilenumber(tileno) {

}

QJsonObject Tile::toJson() const {
    QJsonObject obj{};
    obj["X offset"] = xoff;
    obj["Y offset"] = yoff;
    obj["map16 tile"] = tilenumber;
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

void JsonSprite::from_file(const QString& name) {
    if (name.length() == 0)
        return;
    qDebug() << "Reading from " << name;
    m_name = name;
    QFile file{m_name};
    file.open(QFile::OpenModeFlag::ReadOnly);
    if (name.endsWith(".json")) {
        auto doc = QJsonDocument::fromJson(file.readAll());
        obj = doc.object();
        deserialize();
    }
    else if (name.endsWith(".cfg")) {
        deserialize_cfg(file);
    }
    else {
        QMessageBox::warning(nullptr, "Error", "Unrecognized file extension, valid extensions are: .cfg, .json", QMessageBox::Ok );
        return;
    }

}

void JsonSprite::deserialize_cfg(QFile& file) {
    type = QString{file.readLine()}.toInt(nullptr, 16);
    actlike = QString{file.readLine()}.toInt(nullptr, 16);
    auto tweaks = QString{file.readLine()}.split(" ", Qt::SplitBehaviorFlags::SkipEmptyParts);
    if (tweaks.length() != 6) {
        QMessageBox::warning(nullptr, "Error", "CFG has unrecognized format", QMessageBox::Ok);
        return;
    }
    t1656.from_byte(tweaks[0].toInt(nullptr, 16));
    t1662.from_byte(tweaks[1].toInt(nullptr, 16));
    t166e.from_byte(tweaks[2].toInt(nullptr, 16));
    t167a.from_byte(tweaks[3].toInt(nullptr, 16));
    t1686.from_byte(tweaks[4].toInt(nullptr, 16));
    t190f.from_byte(tweaks[5].toInt(nullptr, 16));
    auto props = QString{file.readLine()}.split(" ", Qt::SplitBehaviorFlags::SkipEmptyParts);
    if (props.length() != 2) {
        QMessageBox::warning(nullptr, "Error", "CFG has unrecognized format", QMessageBox::Ok);
        return;
    }
    extraProp1 = props[0].toInt(nullptr, 16);
    extraProp2 = props[1].toInt(nullptr, 16);
    asmfile = QString{file.readLine()};
    auto bytecount = QString{file.readLine()}.split(QRegularExpression(R"(\s|:)"), Qt::SplitBehaviorFlags::SkipEmptyParts);
    if (bytecount.length() != 2) {
        addbcountclear = 0;
        addbcountset = 0;
    } else {
        addbcountclear = bytecount[0].toInt(nullptr, 16);
        addbcountset = bytecount[1].toInt(nullptr, 16);
    }
}

QByteArray JsonSprite::serialize_cfg() {
    QByteArray text{};
    QTextStream stream{&text};
    stream << QString::asprintf("%02X\n%02X\n", type, actlike);
    stream << QString::asprintf("%02X %02X %02X %02X %02X %02X\n", t1656.to_byte(), t1662.to_byte(), t166e.to_byte(), t167a.to_byte(), t1686.to_byte(), t190f.to_byte());
    stream << QString::asprintf("%02X %02X\n", extraProp1, extraProp2);
    stream << asmfile.trimmed() << '\n';
    stream << QString::asprintf("%02X:%02X", addbcountclear, addbcountset);
    stream.flush();
    return text;
}

JsonSprite::JsonSprite() {
    t1656 = J1656();
    t1662 = J1662();
    t166e = J166E();
    t167a = J167A();
    t1686 = J1686();
    t190f = J190F();
    asmfile = QString();
    actlike = 0;
    type = 0;
    extraProp1 = 0;
    extraProp2 = 0;
    addbcountclear = 0;
    addbcountset = 0;
    map16 = QString();
    displays = QVector<Display>();
    collections = QVector<Collection>();
	dispType = DisplayType::XY;
}

void JsonSprite::reset() {
    m_name.clear();
    t1656.from_byte(0);
    t1662.from_byte(0);
    t166e.from_byte(0);
    t167a.from_byte(0);
    t1686.from_byte(0);
    t190f.from_byte(0);
    asmfile.clear();
    actlike = 0;
    type = 0;
    extraProp1 = 0;
    extraProp2 = 0;
    addbcountclear = 0;
    addbcountset = 0;
    map16.clear();
    displays.clear();
    collections.clear();
	dispType = DisplayType::XY;
}

void JsonSprite::deserialize() {
    t1656.from_json(obj["$1656"].toObject());
    t1662.from_json(obj["$1662"].toObject());
    t166e.from_json(obj["$166E"].toObject());
    t167a.from_json(obj["$167A"].toObject());
    t1686.from_json(obj["$1686"].toObject());
    t190f.from_json(obj["$190F"].toObject());
    asmfile = obj["AsmFile"].toString();
    actlike = obj["ActLike"].toInt();
    type = obj["Type"].toInt();
    extraProp1 = obj["Extra Property Byte 1"].toInt();
    extraProp2 = obj["Extra Property Byte 2"].toInt();
    addbcountclear = obj["Additional Byte Count (extra bit clear)"].toInt();
    addbcountset = obj["Additional Byte Count (extra bit set)"].toInt();
    map16 = obj["Map16"].toString();
    if (obj.find("DisplayType") != obj.end())
        dispType = obj["DisplayType"].toString() == "ExByte" ? DisplayType::ExtraByte : DisplayType::XY;
    else
        dispType = DisplayType::XY;
    auto dispArr = obj["Displays"].toArray();
    auto collArr = obj["Collection"].toArray();
    displays.reserve(dispArr.size());
    collections.reserve(collArr.size());
    std::for_each(dispArr.cbegin(), dispArr.cend(), [&](auto& d) {
        displays.push_back(Display(d.toObject(), dispType));
    });
    std::for_each(collArr.cbegin(), collArr.cend(), [&](auto& c) {
        collections.push_back(Collection(c.toObject()));
    });
}

void JsonSprite::serialize() {
    obj["AsmFile"] = asmfile.trimmed();
    obj["ActLike"] = actlike;
    obj["Type"] = type;
    obj["Additional Byte Count (extra bit clear)"] = addbcountclear;
    obj["Additional Byte Count (extra bit set)"] = addbcountset;
    obj["Extra Property Byte 1"] = extraProp1;
    obj["Extra Property Byte 2"] = extraProp2;
    obj["Map16"] = map16;
    QJsonArray dispArr{};
    QJsonArray collArr{};
    obj["DisplayType"] = dispType == DisplayType::XY ? "XY" : "ExByte";
    std::for_each(displays.cbegin(), displays.cend(), [&](auto& d) {
        dispArr.append(d.toJson(dispType));
    });
    std::for_each(collections.cbegin(), collections.cend(), [&](auto& c) {
        collArr.append(c.toJson());
    });
    obj["Displays"] = dispArr;
    obj["Collection"] = collArr;
    obj["$1656"] = t1656.to_json();
    obj["$1662"] = t1662.to_json();
    obj["$166E"] = t166e.to_json();
    obj["$167A"] = t167a.to_json();
    obj["$1686"] = t1686.to_json();
    obj["$190F"] = t190f.to_json();
}

QByteArray JsonSprite::to_text(const QString& filename) {
    if (filename.endsWith(".cfg")) {
        return serialize_cfg();
    } else {
        serialize();
        QJsonDocument doc{obj};
        return doc.toJson();
    }
}

QString& JsonSprite::name() {
    return m_name;
}

void JsonSprite::to_file(QString name) {
    if (name.length() == 0) {
        if (m_name.length() == 0)
            name = QFileDialog::getSaveFileName(nullptr, "Save file", "", "JSON (*.json);;CFG (*.cfg)");
        else
            name = m_name;
        if (name.length() == 0)
            return;
        QFile outFile{name};
        outFile.open(QFile::OpenModeFlag::Truncate | QFile::OpenModeFlag::Text | QFile::OpenModeFlag::WriteOnly);
        outFile.write(to_text(name));
    } else {
        QFile outFile{name};
        outFile.open(QFile::OpenModeFlag::Truncate | QFile::OpenModeFlag::Text | QFile::OpenModeFlag::WriteOnly);
        outFile.write(to_text(name));
    }
}

void JsonSprite::addCollections(QTableView* view) {
    auto model = view->model();
    for (int i = 0; i < model->rowCount(); i++) {
        QJsonObject obj;
        obj["Name"] = model->index(i, 0).data().toString();
        obj["ExtraBit"] = model->index(i, 1).data().toBool();
        for (int j = 0; j < 12; j++) {
            obj[QString::asprintf("Extra Property Byte %d", j + 1)] = model->index(i, j + 2).data().toString().toInt(nullptr, 16);
        }
        collections.append(Collection{obj});
    }
}

void JsonSprite::addDisplay(const Display& display) {
    displays.append(display);
}

void JsonSprite::setMap16(const QString& mapdata) {
    map16 = mapdata;
}
