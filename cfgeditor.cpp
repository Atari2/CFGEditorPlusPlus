#include "cfgeditor.h"
#include "./ui_cfgeditor.h"

CFGEditor::CFGEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CFGEditor)
    , sprite(new JsonSprite)
    , hexValidator(new QRegularExpressionValidator{QRegularExpression(R"([A-Fa-f0-9]{0,2})")})
    , hexNumberList(new QStringList(0x100))
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    setUpImages();
    QMenuBar* mb = menuBar();
    initCompleter();
    setUpMenuBar(mb);
    bindSpriteProp();
    setCollectionModel();
    bindCollectionButtons();
    ui->Default->setAutoFillBackground(true);
    mb->show();
    setMenuBar(mb);
}

DefaultMissingImpl::DefaultMissingImpl(const QString& impl_name) : name(impl_name) {
    // yes, we construct this here, and we do not call delete on it
    // for some weird reason if I put delete messageBox in the destructor of DefaultMissingImpl
    // the app will segfault with invalid memory access and clang tidy tells me that
    // I'm trying to delete released memory ¯\_(ツ)_/¯, I'll just leave it be
    // if it ever becomes a problem (e.g. memory leak), I'll fix it
    messageBox = new QMessageBox();
#ifdef QT_DEBUG
    messageBox->setText("Implement " + name);
#else
    messageBox->setText(name + " has not been implemented yet");
#endif
}

void DefaultMissingImpl::operator()() {
    if (messageBox)
        messageBox->exec();
};

DefaultAlertImpl::DefaultAlertImpl(const QString& impl_name) {
    messageBox = new QMessageBox();
    messageBox->setText(impl_name);
}

void DefaultAlertImpl::operator()() {
    if (messageBox)
        messageBox->exec();
};

void CFGEditor::initCompleter() {
    for (int i = 0; i <= 0xFF; i++) {
        hexNumberList->append(QString::asprintf("%02X", i));
    }
    hexCompleter = new QCompleter(*hexNumberList, this);
    hexCompleter->setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
}

void CFGEditor::setUpMenuBar(QMenuBar* mb) {
    QMenu* file = new QMenu("&File");
    QMenu* display = new QMenu("&Display");
    file->addAction("&New", qApp, [&]() {
        if (sprite->name().length() == 0) {
            sprite->reset();
            resetTweaks();
        } else {
            auto res = QMessageBox::question(this,
                                             "One file is already open",
                                             "Do you want to save it before opening a new one?",
                                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
            if (res == QMessageBox::Yes) {
                sprite->to_file(QFileDialog::getSaveFileName());
            } else if (res == QMessageBox::Abort) {
                return;
            } else {
                sprite->reset();
            }
            resetTweaks();
            ui->tableView->model()->removeRows(0, ui->tableView->model()->rowCount());
        }
    }, Qt::CTRL | Qt::Key_N);

    file->addSeparator();

    file->addAction("&Open File", qApp, [&]() {
        if (sprite->name().length() != 0) {
            auto res = QMessageBox::question(this,
                                  "One file is already open",
                                  "Do you want to save it before opening the other one?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
            if (res == QMessageBox::Yes) {
                sprite->addCollections(ui->tableView);
                sprite->to_file(QFileDialog::getSaveFileName());
            } else if (res == QMessageBox::Abort) {
                return;
            }
        }
        auto file = QFileDialog::getOpenFileName();
        sprite->from_file(file);
        resetTweaks();
        std::for_each(sprite->collections.cbegin(), sprite->collections.cend(), [&](auto& coll) {
            ((QStandardItemModel*)ui->tableView->model())->appendRow(CollectionDataModel::fromCollection(coll));
        });
    }, Qt::CTRL | Qt::Key_O);

    file->addAction("&Save", qApp, [&]() {
        sprite->addCollections(ui->tableView);
        sprite->to_file();
    }, Qt::CTRL | Qt::Key_S);

    file->addAction("&Save As", qApp, [&]() {
        sprite->addCollections(ui->tableView);
        sprite->to_file(QFileDialog::getSaveFileName());
    }, Qt::CTRL | Qt::ALT | Qt::Key_S);

    display->addAction("&Load Custom Map16", qApp, DefaultMissingImpl("Load Custom Map16"));
    display->addAction("&Load Custom GFX33", qApp, DefaultMissingImpl("Load Custom GFX33"));
    display->addAction("&Palette", qApp, DefaultMissingImpl("Palette"));
    display->addAction("&8x8 Tile Selector", qApp, DefaultMissingImpl("8x8 Tile Selector"));

    mb->addMenu(file);
    mb->addMenu(display);
}

QVector<QStandardItem*> CollectionDataModel::getRow(void* ui) {
    QVector<QStandardItem*> data{};
    if (ui == nullptr) {
        data.append(new QStandardItem(m_name));
        data.append(new QStandardItem(m_extrabit ? "True" : "False"));
        for (int i = 0; i < 12; i++) {
            data.append(new QStandardItem(QString::asprintf("%02X", m_bytes[i])));
        }
    } else {
        auto ed = (Ui::CFGEditor*)ui;
        data.append(new QStandardItem(ed->lineEditCollName->text()));
        data.append(new QStandardItem(ed->checkBoxCollExtrabit->isChecked() ? "True" : "False"));
        data.append(new QStandardItem(ed->lineEditCollExByte1->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte2->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte3->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte4->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte5->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte6->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte7->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte8->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte9->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte10->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte11->text()));
        data.append(new QStandardItem(ed->lineEditCollExByte12->text()));
    }
    return data;
}


void CFGEditor::setCollectionModel() {
    CollectionDataModel data{};
    QStandardItemModel* model = new QStandardItemModel;
    QStringList labelList{};
    labelList.append("Name");
    labelList.append("Extra bit");
    for (int i = 1; i <= 12; i++) {
        labelList.append(QString::asprintf("Ex%d", i));
    }
    model->setHorizontalHeaderLabels(labelList);
    ui->tableView->setFixedSize(ui->tableView->size());
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    ui->lineEditCollExByte1->setMaxLength(2);
    ui->lineEditCollExByte1->setValidator(hexValidator);
    ui->lineEditCollExByte1->setCompleter(hexCompleter);
    ui->lineEditCollExByte2->setMaxLength(2);
    ui->lineEditCollExByte2->setValidator(hexValidator);
    ui->lineEditCollExByte2->setCompleter(hexCompleter);
    ui->lineEditCollExByte3->setMaxLength(2);
    ui->lineEditCollExByte3->setValidator(hexValidator);
    ui->lineEditCollExByte3->setCompleter(hexCompleter);
    ui->lineEditCollExByte4->setMaxLength(2);
    ui->lineEditCollExByte4->setValidator(hexValidator);
    ui->lineEditCollExByte4->setCompleter(hexCompleter);
    ui->lineEditCollExByte5->setMaxLength(2);
    ui->lineEditCollExByte5->setValidator(hexValidator);
    ui->lineEditCollExByte5->setCompleter(hexCompleter);
    ui->lineEditCollExByte6->setMaxLength(2);
    ui->lineEditCollExByte6->setValidator(hexValidator);
    ui->lineEditCollExByte6->setCompleter(hexCompleter);
    ui->lineEditCollExByte7->setMaxLength(2);
    ui->lineEditCollExByte7->setValidator(hexValidator);
    ui->lineEditCollExByte7->setCompleter(hexCompleter);
    ui->lineEditCollExByte8->setMaxLength(2);
    ui->lineEditCollExByte8->setValidator(hexValidator);
    ui->lineEditCollExByte8->setCompleter(hexCompleter);
    ui->lineEditCollExByte9->setMaxLength(2);
    ui->lineEditCollExByte9->setValidator(hexValidator);
    ui->lineEditCollExByte9->setCompleter(hexCompleter);
    ui->lineEditCollExByte10->setMaxLength(2);
    ui->lineEditCollExByte10->setValidator(hexValidator);
    ui->lineEditCollExByte10->setCompleter(hexCompleter);
    ui->lineEditCollExByte11->setMaxLength(2);
    ui->lineEditCollExByte11->setValidator(hexValidator);
    ui->lineEditCollExByte11->setCompleter(hexCompleter);
    ui->lineEditCollExByte12->setMaxLength(2);
    ui->lineEditCollExByte12->setValidator(hexValidator);
    ui->lineEditCollExByte12->setCompleter(hexCompleter);

}

void CFGEditor::bindCollectionButtons() {
    QObject::connect(ui->newCollButton, &QPushButton::clicked, this, [&]() {
        qDebug() << "New collection button clicked";
        ((QStandardItemModel*)ui->tableView->model())->appendRow(CollectionDataModel().getRow(ui));
    });
    QObject::connect(ui->cloneCollButton, &QPushButton::clicked, this, [&]() {
        if (!ui->tableView->currentIndex().isValid()) {
            DefaultAlertImpl("Select a row before cloning")();
            return;
        }
        qDebug() << "Clone collection button clicked";
        CollectionDataModel model = CollectionDataModel::fromIndex(ui->tableView->currentIndex().row(), ui->tableView);
        ((QStandardItemModel*)ui->tableView->model())->appendRow(model.getRow());
    });
    QObject::connect(ui->deleteCollButton, &QPushButton::clicked, this, [&]() {
        if (!ui->tableView->currentIndex().isValid()) {
            DefaultAlertImpl("Select a row before deleting")();
            return;
        }
        qDebug() << "Delete collection button clicked";
        ui->tableView->model()->removeRow(ui->tableView->currentIndex().row());
    });
}

void CFGEditor::setUpImages() {
    SpritePaletteCreator::ReadPaletteFile(0, 22);
    paletteImages.reserve(SpritePaletteCreator::npalettes());
    for (int i = 0; i < SpritePaletteCreator::npalettes(); i++) {
        paletteImages.append(SpritePaletteCreator::MakePalette(i));
    }
    for (int i = 0; i <= 0x0F; i++) {
        QFile img{QString::asprintf(":/Resources/ObjClipping/%02X.png", i)};
        img.open(QFile::ReadOnly);
        QPixmap clip{};
        clip.loadFromData(img.readAll(), "png");
        if (clip.size().height() > clip.size().width())
            objClipImages.append(clip.scaledToHeight(100));
        else
            objClipImages.append(clip.scaledToWidth(100));
    }
    for (int i = 0; i <= 0x3F; i++) {
        QFile img{QString::asprintf(":/Resources/SprClipping/%02X.png", i)};
        img.open(QFile::ReadOnly);
        QPixmap clip{};
        clip.loadFromData(img.readAll(), "png");
        if (clip.size().height() > clip.size().width())
            sprClipImages.append(clip.scaledToHeight(100));
        else
            sprClipImages.append(clip.scaledToWidth(100));
    }
}

void CFGEditor::resetTweaks() {
    ui->lineEditExtraProp1->setText(QString::asprintf("%02X", sprite->extraProp1));
    emit ui->lineEditExtraProp1->editingFinished();
    ui->lineEditExtraProp2->setText(QString::asprintf("%02X", sprite->extraProp2));
    emit ui->lineEditExtraProp2->editingFinished();
    ui->comboBoxType->setCurrentIndex(sprite->type);
    ui->lineEditAsmFile->setText(sprite->asmfile);
    emit ui->lineEditAsmFile->editingFinished();
    ui->spinBoxextraBitClear->setValue(sprite->addbcountclear);
    ui->spinBoxextraBitSet->setValue(sprite->addbcountset);
    ui->lineEditActLike->setText(QString::asprintf("%02X", sprite->actlike));
    emit ui->lineEditActLike->editingFinished();
    ui->lineEdit1656->setText(QString::asprintf("%02X", sprite->t1656.to_byte()));
    emit ui->lineEdit1656->editingFinished();
    ui->lineEdit1662->setText(QString::asprintf("%02X", sprite->t1662.to_byte()));
    emit ui->lineEdit1662->editingFinished();
    ui->lineEdit166E->setText(QString::asprintf("%02X", sprite->t166e.to_byte()));
    emit ui->lineEdit166E->editingFinished();
    ui->lineEdit167a->setText(QString::asprintf("%02X", sprite->t167a.to_byte()));
    emit ui->lineEdit167a->editingFinished();
    ui->lineEdit1686->setText(QString::asprintf("%02X", sprite->t1686.to_byte()));
    emit ui->lineEdit1686->editingFinished();
    ui->lineEdit190f->setText(QString::asprintf("%02X", sprite->t190f.to_byte()));
    emit ui->lineEdit190f->editingFinished();
}

void CFGEditor::bindSpriteProp() {
    // Map16, Displays, Collection -> todo
    // FIXME: handle map16, displays and collection
    // Extra Prop Bytes
    ui->lineEditExtraProp1->setMaxLength(2);
    ui->lineEditExtraProp1->setValidator(hexValidator);
    ui->lineEditExtraProp1->setCompleter(hexCompleter);
    ui->lineEditExtraProp2->setMaxLength(2);
    ui->lineEditExtraProp2->setValidator(hexValidator);
    ui->lineEditExtraProp2->setCompleter(hexCompleter);
    QObject::connect(ui->lineEditExtraProp1, &QLineEdit::editingFinished, this, [&]() {
        sprite->extraProp1 = (uint8_t)ui->lineEditExtraProp1->text().toUInt(nullptr, 16);
    });
    QObject::connect(ui->lineEditExtraProp2, &QLineEdit::editingFinished, this, [&]() {
        sprite->extraProp2 = (uint8_t)ui->lineEditExtraProp2->text().toUInt(nullptr, 16);
    });
    // Type
    QObject::connect(ui->comboBoxType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        sprite->type = index;
    });
    // ActLike
    ui->lineEditActLike->setMaxLength(2);
    ui->lineEditActLike->setValidator(hexValidator);
    ui->lineEditActLike->setCompleter(hexCompleter);
    QObject::connect(ui->lineEditActLike, &QLineEdit::editingFinished, this, [&]() {
        sprite->actlike = ui->lineEditActLike->text().toUInt(nullptr, 16);
    });
    // AsmFile
    QObject::connect(ui->lineEditAsmFile, &QLineEdit::editingFinished, this, [&]() {
        sprite->asmfile = ui->lineEditAsmFile->text();
    });
    // Additional byte count
    QObject::connect(ui->spinBoxextraBitClear, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
        qDebug() << "Extra byte (clear) changed to " << value;
        sprite->addbcountclear = value;
    });
    QObject::connect(ui->spinBoxextraBitSet, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
        qDebug() << "Extra byte (set) changed to " << value;
        sprite->addbcountset = value;
    });
    // 1656
    bindTweak1656();
    // 1662
    bindTweak1662();
    // 166E
    bindTweak166E();
    // 167A
    bindTweak167A();
    // 1686
    bindTweak1686();
    // 190F
    bindTweak190F();
}

void CFGEditor::bindTweak1656() {
    ui->lineEdit1656->setMaxLength(2);
    ui->lineEdit1656->setValidator(hexValidator);
    ui->lineEdit1656->setCompleter(hexCompleter);
    ui->objClippingLabel->setPixmap(objClipImages[0]);
    QObject::connect(ui->lineEdit1656, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "Value changed";
        sprite->t1656.from_byte((uint8_t)ui->lineEdit1656->text().toUInt(nullptr, 16));
        ui->checkBox1656DiesJumped->setChecked(sprite->t1656.diesjumped);
        ui->checkBox1656Hopin->setChecked(sprite->t1656.hopin);
        ui->checkBox1656JumpedOn->setChecked(sprite->t1656.canbejumped);
        ui->checkBox1656Smoke->setChecked(sprite->t1656.disapp);
        ui->objClipCmbBox->setCurrentIndex(sprite->t1656.objclip);
    });
    connectCheckBox(ui->lineEdit1656, ui->checkBox1656DiesJumped, &sprite->t1656, sprite->t1656.diesjumped);
    connectCheckBox(ui->lineEdit1656, ui->checkBox1656JumpedOn, &sprite->t1656, sprite->t1656.canbejumped);
    connectCheckBox(ui->lineEdit1656, ui->checkBox1656Hopin, &sprite->t1656, sprite->t1656.hopin);
    connectCheckBox(ui->lineEdit1656, ui->checkBox1656Smoke, &sprite->t1656, sprite->t1656.disapp);
    QObject::connect(ui->objClipCmbBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        qDebug() << "Index changed";
        ui->objClippingLabel->setPixmap(objClipImages[index]);
        ui->objClippingLabel->setFixedSize(objClipImages[index].width(), objClipImages[index].height());
        sprite->t1656.objclip = index;
        ui->lineEdit1656->setText(QString::asprintf("%02X", sprite->t1656.to_byte()));
    });

}
void CFGEditor::bindTweak1662() {
    ui->lineEdit1662->setMaxLength(2);
    ui->lineEdit1662->setValidator(hexValidator);
    ui->lineEdit1662->setCompleter(hexCompleter);
    ui->sprClippingLabel->setPixmap(sprClipImages[0]);
    QObject::connect(ui->lineEdit1662, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "Value changed";
        sprite->t1662.from_byte((uint8_t)ui->lineEdit1662->text().toUInt(nullptr, 16));
        ui->checkBox1662deathframe->setChecked(sprite->t1662.deathframe);
        ui->checkBox1662strdown->setChecked(sprite->t1662.strdown);
        ui->sprClipCmbBox->setCurrentIndex(sprite->t1662.sprclip);
    });
    connectCheckBox(ui->lineEdit1662, ui->checkBox1662deathframe, &sprite->t1662, sprite->t1662.deathframe);
    connectCheckBox(ui->lineEdit1662, ui->checkBox1662strdown, &sprite->t1662, sprite->t1662.strdown);
    QObject::connect(ui->sprClipCmbBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        qDebug() << "Index changed";
        sprite->t1662.sprclip = index;
        ui->sprClippingLabel->setPixmap(sprClipImages[index]);
        ui->sprClippingLabel->setFixedSize(sprClipImages[index].width(), sprClipImages[index].height());
        ui->lineEdit1662->setText(QString::asprintf("%02X", sprite->t1662.to_byte()));
    });
}
void CFGEditor::bindTweak166E() {
    ui->lineEdit166E->setMaxLength(2);
    ui->lineEdit166E->setValidator(hexValidator);
    ui->lineEdit166E->setCompleter(hexCompleter);
    ui->label->setPixmap(paletteImages[0].scaled(ui->label->size(), Qt::AspectRatioMode::KeepAspectRatio));
    QObject::connect(ui->lineEdit166E, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "Value changed";
        sprite->t166e.from_byte((uint8_t)ui->lineEdit166E->text().toUInt(nullptr, 16));
        ui->checkBox166ecape->setChecked(sprite->t166e.cape);
        ui->checkBox166efireball->setChecked(sprite->t166e.fireball);
        ui->checkBox166esecondpage->setChecked(sprite->t166e.secondpage);
        ui->checkBox166esplash->setChecked(sprite->t166e.splash);
        ui->checkBox166elay2->setChecked(sprite->t166e.lay2);
        ui->paletteComboBox->setCurrentIndex(sprite->t166e.palette);
    });
    connectCheckBox(ui->lineEdit166E, ui->checkBox166ecape, &sprite->t166e, sprite->t166e.cape);
    connectCheckBox(ui->lineEdit166E, ui->checkBox166efireball, &sprite->t166e, sprite->t166e.fireball);
    connectCheckBox(ui->lineEdit166E, ui->checkBox166esplash, &sprite->t166e, sprite->t166e.splash);
    connectCheckBox(ui->lineEdit166E, ui->checkBox166esecondpage, &sprite->t166e, sprite->t166e.secondpage);
    connectCheckBox(ui->lineEdit166E, ui->checkBox166elay2, &sprite->t166e, sprite->t166e.lay2);
    QObject::connect(ui->paletteComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        qDebug() << "Index changed";
        ui->label->setPixmap(paletteImages[index].scaled(ui->label->size(), Qt::AspectRatioMode::KeepAspectRatio));
        sprite->t166e.palette = index;
        ui->lineEdit166E->setText(QString::asprintf("%02X", sprite->t166e.to_byte()));
    });
}
void CFGEditor::bindTweak167A() {
    ui->lineEdit167a->setMaxLength(2);
    ui->lineEdit167a->setValidator(hexValidator);
    ui->lineEdit167a->setCompleter(hexCompleter);
    QObject::connect(ui->lineEdit167a, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "Value changed";
        sprite->t167a.from_byte((uint8_t)ui->lineEdit167a->text().toUInt(nullptr, 16));
        ui->checkBox167astar->setChecked(sprite->t167a.star);
        ui->checkBox167ablk->setChecked(sprite->t167a.blk);
        ui->checkBox167aoffscr->setChecked(sprite->t167a.offscr);
        ui->checkBox167astunned->setChecked(sprite->t167a.stunn);
        ui->checkBox167akick->setChecked(sprite->t167a.kick);
        ui->checkBox167aeveryframe->setChecked(sprite->t167a.everyframe);
        ui->checkBox167apowerup->setChecked(sprite->t167a.powerup);
        ui->checkBox167adefaultint->setChecked(sprite->t167a.defaultint);
    });
    connectCheckBox(ui->lineEdit167a, ui->checkBox167astar, &sprite->t167a, sprite->t167a.star);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167ablk, &sprite->t167a, sprite->t167a.blk);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167aoffscr, &sprite->t167a, sprite->t167a.offscr);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167astunned, &sprite->t167a, sprite->t167a.stunn);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167akick, &sprite->t167a, sprite->t167a.kick);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167aeveryframe, &sprite->t167a, sprite->t167a.everyframe);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167apowerup, &sprite->t167a, sprite->t167a.powerup);
    connectCheckBox(ui->lineEdit167a, ui->checkBox167adefaultint, &sprite->t167a, sprite->t167a.defaultint);
}
void CFGEditor::bindTweak1686() {
    ui->lineEdit1686->setMaxLength(2);
    ui->lineEdit1686->setValidator(hexValidator);
    ui->lineEdit1686->setCompleter(hexCompleter);
    QObject::connect(ui->lineEdit1686, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "Value changed";
        sprite->t1686.from_byte((uint8_t)ui->lineEdit1686->text().toUInt(nullptr, 16));
        ui->checkBox1686Inedible->setChecked(sprite->t1686.inedible);
        ui->checkBox1686mouth->setChecked(sprite->t1686.mouth);
        ui->checkBox1686ground->setChecked(sprite->t1686.ground);
        ui->checkBox1686sprint->setChecked(sprite->t1686.nosprint);
        ui->checkBox1686dir->setChecked(sprite->t1686.direc);
        ui->checkBox1686goalcoin->setChecked(sprite->t1686.goalpass);
        ui->checkBox1686spawnSpr->setChecked(sprite->t1686.newspr);
        ui->checkBox1686noObjInt->setChecked(sprite->t1686.noobjint);
    });
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686Inedible, &sprite->t1686, sprite->t1686.inedible);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686mouth, &sprite->t1686, sprite->t1686.mouth);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686ground, &sprite->t1686, sprite->t1686.ground);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686sprint, &sprite->t1686, sprite->t1686.nosprint);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686dir, &sprite->t1686, sprite->t1686.direc);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686goalcoin, &sprite->t1686, sprite->t1686.goalpass);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686spawnSpr, &sprite->t1686, sprite->t1686.newspr);
    connectCheckBox(ui->lineEdit1686, ui->checkBox1686noObjInt, &sprite->t1686, sprite->t1686.noobjint);
}
void CFGEditor::bindTweak190F() {
    ui->lineEdit190f->setMaxLength(2);
    ui->lineEdit190f->setValidator(hexValidator);
    ui->lineEdit190f->setCompleter(hexCompleter);
    QObject::connect(ui->lineEdit190f, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "Value changed";
        sprite->t190f.from_byte((uint8_t)ui->lineEdit190f->text().toUInt(nullptr, 16));
        ui->checkBox190fbelow->setChecked(sprite->t190f.below);
        ui->checkBox190fgoalpass->setChecked(sprite->t190f.goal);
        ui->checkBox190fsliding->setChecked(sprite->t190f.slidekill);
        ui->checkBox190ffivefire->setChecked(sprite->t190f.fivefire);
        ui->checkBox190fupysp->setChecked(sprite->t190f.yupsp);
        ui->checkBox190fdeathframe->setChecked(sprite->t190f.deathframe);
        ui->checkBox190fnosilver->setChecked(sprite->t190f.nosilver);
        ui->checkBox190fwallstuck->setChecked(sprite->t190f.nostuck);
    });
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fbelow, &sprite->t190f, sprite->t190f.below);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fgoalpass, &sprite->t190f, sprite->t190f.goal);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fsliding, &sprite->t190f, sprite->t190f.slidekill);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190ffivefire, &sprite->t190f, sprite->t190f.fivefire);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fupysp, &sprite->t190f, sprite->t190f.yupsp);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fdeathframe, &sprite->t190f, sprite->t190f.deathframe);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fnosilver, &sprite->t190f, sprite->t190f.nosilver);
    connectCheckBox(ui->lineEdit190f, ui->checkBox190fwallstuck, &sprite->t190f, sprite->t190f.nostuck);
}

CFGEditor::~CFGEditor()
{
    delete hexValidator;
    delete hexCompleter;
    delete hexNumberList;
    delete ui;
}

