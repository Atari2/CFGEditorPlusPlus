#include "cfgeditor.h"
#include "./ui_cfgeditor.h"

CFGEditor::CFGEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CFGEditor)
    , sprite(new JsonSprite)
    , hexValidator(new QRegularExpressionValidator{QRegularExpression(R"([A-Fa-f0-9]+)")})
    , hexNumberList(new QStringList(0x100))
    , models()
    , copiedTile()
    , displays()
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    setUpImages();
    view8x8 = new EightByEightView(new QGraphicsScene);
    viewPalette = new PaletteView(new QGraphicsScene);
    loadFullbitmap();
    ui->map16GraphicsView->setControllingLabel(ui->labelTileNo);
    QMenuBar* mb = menuBar();
    initCompleter();
    setUpMenuBar(mb);
    bindSpriteProp();
    setCollectionModel();
    setDisplayModel();
    bindCollectionButtons();
    bindDisplayButtons();
    bindGFXFilesButtons();
    bindGFXSelector();
    ui->Default->setAutoFillBackground(true);
    mb->show();
    setMenuBar(mb);
}

void CFGEditor::closeEvent(QCloseEvent *event) {
    view8x8->close();
    viewPalette->close();
    QMainWindow::closeEvent(event);
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

void CFGEditor::loadFullbitmap(int index) {
    if (index == -1)
        index = ui->paletteComboBox->currentIndex();
    QVector<QString> gfxFiles{ui->lineEditGFXSp0->text(), ui->lineEditGFXSp1->text(), ui->lineEditGFXSp2->text(), ui->lineEditGFXSp3->text()};
    if (full8x8Bitmap) {
        full8x8Bitmap->~QImage();
        delete full8x8Bitmap;
    }
    full8x8Bitmap = new QImage{128, 256, QImage::Format_RGB32};
    QPainter p{full8x8Bitmap};
    p.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
    int i = 0;
    SnesGFXConverter::populateFullMap16Data(gfxFiles);
    for (auto& file : gfxFiles) {
        QImage img;
        if (!QDir(file).isAbsolute())
            img = SnesGFXConverter::fromResource(":/Resources/Graphics/" + file + ".bin", SpritePaletteCreator::getPalette(index + 8));
        else
            img = SnesGFXConverter::fromResource(file, SpritePaletteCreator::getPalette(index + 8));
        p.drawImage(QRect{0, i, 128, 64}, img, QRect{0, 0, img.width(), img.height()});
        i += 64;
    }
    ui->map16GraphicsView->readInternalMap16File();
    view8x8->updateForChange(full8x8Bitmap);
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
            // TODO: clear displays, etc
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
                for (auto& disp : displays)
                    sprite->addDisplay(createDisplay(disp));
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
        ui->map16GraphicsView->setMap16(sprite->map16);
        ui->labelDisplayTilesGrid->deserializeDisplays(sprite->displays, ui->map16GraphicsView);
        populateDisplays();
        populateGFXFiles();
    }, Qt::CTRL | Qt::Key_O);

    file->addAction("&Save", qApp, [&]() {
        ui->labelDisplayTilesGrid->serializeDisplays(displays);
        sprite->displays.clear();
        sprite->collections.clear();
        for (auto& disp : displays)
            sprite->addDisplay(createDisplay(disp));
        sprite->setMap16(ui->map16GraphicsView->getMap16());
        sprite->addCollections(ui->tableView);
        sprite->to_file();
    }, Qt::CTRL | Qt::Key_S);

    file->addAction("&Save As", qApp, [&]() {
        ui->labelDisplayTilesGrid->serializeDisplays(displays);
        sprite->displays.clear();
        sprite->collections.clear();
        for (auto& disp : displays)
            sprite->addDisplay(createDisplay(disp));
        sprite->setMap16(ui->map16GraphicsView->getMap16());
        sprite->addCollections(ui->tableView);
        sprite->to_file(QFileDialog::getSaveFileName());
    }, Qt::CTRL | Qt::ALT | Qt::Key_S);

    display->addAction("&Load Custom Map16", qApp, [&]() {
        QString name = QFileDialog::getOpenFileName(this);
        if (name.length() == 0)
            return;
        ui->map16GraphicsView->readExternalMap16File(name);
    });
    display->addAction("&Load Custom GFX33", qApp, [&]() {
        QString name = QFileDialog::getOpenFileName(this);
        if (name.length() == 0)
            return;
        SnesGFXConverter::setCustomExanimation(name);
        ui->map16GraphicsView->drawInternalMap16File();
    });
    display->addAction("&Palette", qApp, [&]() {
        qDebug() << "Opening palette viewer";
        viewPalette->updateForChange(SpritePaletteCreator::MakeFullPalette());
        viewPalette->open();
    });
    display->addAction("&8x8 Tile Selector", qApp, [&]() {
        qDebug() << "Opening 8x8 tile selector";
        view8x8->updateForChange(full8x8Bitmap);
        view8x8->open();
    });

    mb->addMenu(file);
    mb->addMenu(display);
}

void CFGEditor::populateDisplays() {
    currentDisplayIndex = -1;
    QSignalBlocker block{ui->tableViewDisplays};
    for (auto& d : sprite->displays) {
        DisplayData display{d};
        ((QStandardItemModel*)ui->tableViewDisplays->model())->appendRow(display.itemsFromDisplay());
        displays.append(display);
        ui->checkBoxDisplayExtraBit->setChecked(display.ExtraBit());
        ui->checkBoxUseText->setChecked(display.UseText());
        ui->spinBoxXPos->setValue(display.X());
        ui->spinBoxYPos->setValue(display.Y());
        ui->textEditLMDescription->setText(display.Description());
        if (display.UseText())
           ui->textEditDisplayText->setText(display.DisplayText());
    }
}

void CFGEditor::populateGFXFiles() {
    currentGFXFileIndex = 0;
    auto len = sprite->gfxfiles.size();
    for (auto i = 0; i < len; i++) {
        ui->comboBoxGFXIndex->addItem(QString::asprintf("%d", i));
    }
    if (len > 0) {
        ui->comboBoxGFXIndex->setCurrentIndex(0);
        currentGFXFileIndex = 0;
    } else {
        currentGFXFileIndex = -1;
    }
}

Display CFGEditor::createDisplay(const DisplayData& data) {
    QVector<Tile> tiles;
    tiles.reserve(displays.length());
    for (auto& t : data.Tiles()) {
        tiles.append({t.XOffset(), t.YOffset(), t.TileNumber()});
    }
    return {data.Description(), tiles, data.ExtraBit(), data.X(), data.Y(), data.UseText(), data.DisplayText()};
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
        // this is horrific
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

void CFGEditor::setDisplayModel() {
    ui->textEditDisplayText->setReadOnly(true);
    QStandardItemModel* model = new QStandardItemModel;
    models.append(model);
    QStringList labelList{"ExtraBit", "X", "Y"};
    model->setHorizontalHeaderLabels(labelList);
    ui->tableViewDisplays->setFixedSize(ui->tableViewDisplays->size());
    ui->tableViewDisplays->setModel(model);
    ui->tableViewDisplays->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewDisplays->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableViewDisplays->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->tableViewDisplays->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
}

void CFGEditor::setCollectionModel() {
    QStandardItemModel* model = new QStandardItemModel;
    models.append(model);
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

void CFGEditor::addLunarMagicIcons() {
    QFile first{":/Resources/ButtonIcons/8x8t.png"};
    first.open(QFile::OpenModeFlag::ReadOnly);
    QFile second{":/Resources/ButtonIcons/8x8.png"};
    second.open(QFile::OpenModeFlag::ReadOnly);
    QFile third{":/Resources/ButtonIcons/grid.png"};
    third.open(QFile::OpenModeFlag::ReadOnly);
    QFile fourth{":/Resources/ButtonIcons/page.png"};
    fourth.open(QFile::OpenModeFlag::ReadOnly);
    QFile fifth{":/Resources/ButtonIcons/palette.png"};
    fifth.open(QFile::OpenModeFlag::ReadOnly);
    ui->toolButton8x8Edit->setIcon(QIcon(QPixmap::fromImage(QImage::fromData(first.readAll()))));
    ui->toolButton8x8Edit->setIconSize(QSize(32, 32));
    ui->toolButton8x8Mode->setIcon(QIcon(QPixmap::fromImage(QImage::fromData(second.readAll()))));
    ui->toolButton8x8Mode->setIconSize(QSize(32, 32));
    ui->toolButtonGrid->setIcon(QIcon(QPixmap::fromImage(QImage::fromData(third.readAll()))));
    ui->toolButtonGrid->setIconSize(QSize(32, 32));
    ui->toolButtonBorders->setIcon(QIcon(QPixmap::fromImage(QImage::fromData(fourth.readAll()))));
    ui->toolButtonBorders->setIconSize(QSize(32, 32));
    ui->toolButtonPalette->setIcon(QIcon(QPixmap::fromImage(QImage::fromData(fifth.readAll()))));
    ui->toolButtonPalette->setIconSize(QSize(32, 32));
    ui->toolButton8x8Edit->setToolTip("Switch to 8x8 mode");
    ui->toolButton8x8Mode->setToolTip("Open 8x8 Selector");
    ui->toolButtonGrid->setToolTip("Show grid");
    ui->toolButtonBorders->setToolTip("Show page borders");
    ui->toolButtonPalette->setToolTip("Open Palette Viewer");
}

void CFGEditor::bindGFXSelector() {
    addLunarMagicIcons();
    auto splitSetGFx = [&]() {
        QString gfxStr[4];
        auto strNums = ui->comboBoxGFXSet->currentText().split(" ");
        std::transform(strNums.cbegin(), strNums.cbegin() + 4, std::begin(gfxStr), [&](const QString& str) -> QString {
            return "GFX" + str;
        });
        ui->lineEditGFXSp0->setText(gfxStr[0]);
        ui->lineEditGFXSp1->setText(gfxStr[1]);
        ui->lineEditGFXSp2->setText(gfxStr[2]);
        ui->lineEditGFXSp3->setText(gfxStr[3]);
    };
    QObject::connect(ui->toolButton8x8Edit, &QToolButton::clicked, this, [&]() {
        qDebug() << "Map8x8 edit button clicked";
        ui->map16GraphicsView->switchCurrSelectionType();
    });
    QObject::connect(ui->toolButton8x8Mode, &QToolButton::clicked, this, [&]() {
        qDebug() << "Map8x8 button clicked";
        view8x8->updateForChange(full8x8Bitmap);
        view8x8->open();
    });
    QObject::connect(ui->toolButtonPalette, &QToolButton::clicked, this, [&]() {
        qDebug() << "Palette button clicked";
        viewPalette->updateForChange(SpritePaletteCreator::MakeFullPalette());
        viewPalette->open();
    });
    splitSetGFx();
    QObject::connect(ui->comboBoxGFXSet, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&, splitSetGFx](int _) {
        qDebug() << "Index of GFX set changed";
        splitSetGFx();
        loadFullbitmap();
        if (currentDisplayIndex != -1)
            ui->labelDisplayTilesGrid->redrawNoSort();
    });
    changeTilePropGroupState(true);
    QObject::connect(viewPalette, &PaletteView::paletteChanged, this, [&](){
        qDebug() << "Custom signal change palette received";
        loadFullbitmap();
        for (int i = 0; i < SpritePaletteCreator::nSpritePalettes(); i++) {
            paletteImages[i] = SpritePaletteCreator::MakePalette(i);
        }
        ui->label->setPixmap(paletteImages[ui->paletteComboBox->currentIndex()]);
    });

    QObject::connect(ui->toolButtonGFXSp0, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName();
        if (filename.length() == 0)
            return;
        ui->lineEditGFXSp0->setText(filename);
        loadFullbitmap();
    });

    QObject::connect(ui->toolButtonGFXSp1, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName();
        if (filename.length() == 0)
            return;
        ui->lineEditGFXSp1->setText(filename);
        loadFullbitmap();
    });

    QObject::connect(ui->toolButtonGFXSp2, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName();
        if (filename.length() == 0)
            return;
        ui->lineEditGFXSp2->setText(filename);
        loadFullbitmap();
    });

    QObject::connect(ui->toolButtonGFXSp3, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName();
        if (filename.length() == 0)
            return;
        ui->lineEditGFXSp3->setText(filename);
        loadFullbitmap();
    });
}

void CFGEditor::addCloneRow() {
    DisplayData display(displays[currentDisplayIndex]);
    ((QStandardItemModel*)ui->tableViewDisplays->model())->appendRow(display.itemsFromDisplay());
    advanceDisplayIndex();
    displays.insert(currentDisplayIndex, display);
    ui->checkBoxDisplayExtraBit->setChecked(display.ExtraBit());
    ui->checkBoxUseText->setChecked(display.UseText());
    ui->spinBoxXPos->setValue(display.X());
    ui->spinBoxYPos->setValue(display.Y());
    ui->textEditLMDescription->setText(display.Description());
    if (display.UseText())
       ui->textEditDisplayText->setText(display.DisplayText());
}

void CFGEditor::addBlankRow() {
    DisplayData display = DisplayData::blankData();
    displays.insert(currentDisplayIndex + 1, display);
    advanceDisplayIndex();
    ((QStandardItemModel*)ui->tableViewDisplays->model())->appendRow(display.itemsFromDisplay());
    qDebug() << displays.length();
    ui->checkBoxDisplayExtraBit->setChecked(false);
    ui->checkBoxUseText->setChecked(false);
    ui->spinBoxXPos->setValue(0);
    ui->spinBoxYPos->setValue(0);
    ui->textEditLMDescription->setText("");
    ui->textEditDisplayText->setText("");
}

void CFGEditor::removeExistingRow() {
    displays.removeAt(currentDisplayIndex);
    currentDisplayIndex--;
    qDebug() << currentDisplayIndex;
    ui->tableViewDisplays->model()->removeRow(ui->tableViewDisplays->currentIndex().row());
}

void CFGEditor::advanceDisplayIndex() {
    currentDisplayIndex++;
    qDebug() << "Row count " << ui->tableViewDisplays->model()->rowCount();
    qDebug() << "Row got added at " << currentDisplayIndex << " and at " << ui->tableViewDisplays->currentIndex().row();
}

void CFGEditor::bindGFXFilesButtons() {
    ui->lineEditSp0->setMaxLength(3);
    ui->lineEditSp0->setValidator(hexValidator);
    ui->lineEditSp1->setMaxLength(3);
    ui->lineEditSp1->setValidator(hexValidator);
    ui->lineEditSp2->setMaxLength(3);
    ui->lineEditSp2->setValidator(hexValidator);
    ui->lineEditSp3->setMaxLength(3);
    ui->lineEditSp3->setValidator(hexValidator);

    QObject::connect(ui->pushButtonGFXNew, &QPushButton::clicked, this, [&]() {
        sprite->gfxfiles.insert(++currentGFXFileIndex, {false, 0x7F, 0x7F, 0x7F, 0x7F});
        ui->comboBoxGFXIndex->addItem(QString::asprintf("%d", ui->comboBoxGFXIndex->count()));
        ui->comboBoxGFXIndex->setCurrentIndex(currentGFXFileIndex);
    });
    QObject::connect(ui->pushButtonGFXDelete, &QPushButton::clicked, this, [&]() {
        if (currentGFXFileIndex == -1)
            return;
        sprite->gfxfiles.removeAt(currentGFXFileIndex);
        if (sprite->gfxfiles.empty())
            currentGFXFileIndex = -1;
        ui->comboBoxGFXIndex->removeItem(ui->comboBoxGFXIndex->count() - 1);
        if (ui->comboBoxGFXIndex->count() > 0)
            emit ui->comboBoxGFXIndex->currentIndexChanged(currentGFXFileIndex);

    });
    QObject::connect(ui->pushButtonGFXClone, &QPushButton::clicked, this, [&]() {
        if (currentGFXFileIndex == -1)
            return;
        auto gfx = sprite->gfxfiles[currentGFXFileIndex++];
        sprite->gfxfiles.insert(currentGFXFileIndex, gfx);
        ui->comboBoxGFXIndex->addItem(QString::asprintf("%d", ui->comboBoxGFXIndex->count()));
        ui->comboBoxGFXIndex->setCurrentIndex(currentGFXFileIndex);
    });
    QObject::connect(ui->comboBoxGFXIndex, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        currentGFXFileIndex = index;
        if (currentGFXFileIndex == -1) {
            ui->lineEditSp0->setText(QString::asprintf("%03X", 0x7F));
            ui->lineEditSp1->setText(QString::asprintf("%03X", 0x7F));
            ui->lineEditSp2->setText(QString::asprintf("%03X", 0x7F));
            ui->lineEditSp3->setText(QString::asprintf("%03X", 0x7F));
            ui->checkBoxGFXSeparate->setCheckState(Qt::CheckState::Unchecked);
            return;
        }
        ui->lineEditSp0->setText(QString::asprintf("%03X", sprite->gfxfiles[index].sp0));
        ui->lineEditSp1->setText(QString::asprintf("%03X", sprite->gfxfiles[index].sp1));
        ui->lineEditSp2->setText(QString::asprintf("%03X", sprite->gfxfiles[index].sp2));
        ui->lineEditSp3->setText(QString::asprintf("%03X", sprite->gfxfiles[index].sp3));
        ui->checkBoxGFXSeparate->setCheckState(sprite->gfxfiles[index].separate ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    });
    QObject::connect(ui->lineEditSp0, &QLineEdit::editingFinished, this, [&]() {
        if (sprite->gfxfiles.empty())
            return;
        sprite->gfxfiles[currentGFXFileIndex].sp0 = ui->lineEditSp0->text().toInt(nullptr, 16);
    });
    QObject::connect(ui->lineEditSp1, &QLineEdit::editingFinished, this, [&]() {
        if (sprite->gfxfiles.empty())
            return;
        sprite->gfxfiles[currentGFXFileIndex].sp1 = ui->lineEditSp1->text().toInt(nullptr, 16);
    });
    QObject::connect(ui->lineEditSp2, &QLineEdit::editingFinished, this, [&]() {
        if (sprite->gfxfiles.empty())
            return;
        sprite->gfxfiles[currentGFXFileIndex].sp2 = ui->lineEditSp2->text().toInt(nullptr, 16);
    });
    QObject::connect(ui->lineEditSp3, &QLineEdit::editingFinished, this, [&]() {
        if (sprite->gfxfiles.empty())
            return;
        sprite->gfxfiles[currentGFXFileIndex].sp3 = ui->lineEditSp3->text().toInt(nullptr, 16);
    });
    QObject::connect(ui->checkBoxGFXSeparate, &QCheckBox::stateChanged, this, [&]() {
        if (sprite->gfxfiles.empty())
            return;
       sprite->gfxfiles[currentGFXFileIndex].separate = ui->checkBoxGFXSeparate->isChecked();
    });
}

void CFGEditor::bindDisplayButtons() {
    ui->map16GraphicsView->setCopiedTile(copiedTile);
    ui->labelDisplayTilesGrid->setCopiedTile(copiedTile);
    QObject::connect(ui->toolButtonGrid, &QToolButton::clicked, this, [&]() {
        ui->map16GraphicsView->useGridChanged();
    });
    QObject::connect(ui->toolButtonBorders, &QToolButton::clicked, this, [&]() {
        ui->map16GraphicsView->usePageSepChanged();
    });
    QObject::connect(ui->tableViewDisplays->model(), &QAbstractItemModel::rowsInserted, this, [&]() {
        qDebug() << "Rows have been inserted " << currentDisplayIndex;
        if (ui->tableViewDisplays->currentIndex().isValid())
            ui->tableViewDisplays->setCurrentIndex(ui->tableViewDisplays->model()->index(ui->tableViewDisplays->currentIndex().row() + 1, ui->tableViewDisplays->currentIndex().column()));
        else
            ui->tableViewDisplays->setCurrentIndex(ui->tableViewDisplays->model()->index(currentDisplayIndex, 0));
    });
    // new, delete, clone
    QObject::connect(ui->pushButtonNewDisplay, &QPushButton::clicked, this, [&]() {
        qDebug() << "New row button pushed";
        ui->labelDisplayTilesGrid->addDisplay(currentDisplayIndex);
        addBlankRow();
    });
    QObject::connect(ui->pushButtonCloneDisplay, &QPushButton::clicked, this, [&]() {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            DefaultAlertImpl("Select a row before cloning")();
            return;
        }
        qDebug() << "Clone display button clicked";
        addCloneRow();
        ui->labelDisplayTilesGrid->cloneDisplay(currentDisplayIndex);
    });
    QObject::connect(ui->pushButtonDeleteDisplay, &QPushButton::clicked, this, [&]() {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            DefaultAlertImpl("Select a row before deleting")();
            return;
        }
        qDebug() << "Delete display button clicked";
        ui->labelDisplayTilesGrid->removeDisplay(currentDisplayIndex);
        removeExistingRow();
    });

    // index gets changed
    QObject::connect(ui->tableViewDisplays->selectionModel(),
                     QOverload<const QModelIndex&, const QModelIndex&>::of(&QItemSelectionModel::currentRowChanged), this, [&](const QModelIndex& now, const QModelIndex& pre) {
        qDebug() << "current row changed called";
        int len = displays.length();
        if (ui->tableViewDisplays->model()->rowCount() == 0)
            return;
        if (currentDisplayIndex == -1 && len == 0)
            return;
        if (now.row() == -1)
            currentDisplayIndex = pre.row();
        else
            currentDisplayIndex = now.row();
        if (currentDisplayIndex > len - 1)
            currentDisplayIndex = 0;
        qDebug() << currentDisplayIndex << " " << ui->tableViewDisplays->model()->rowCount();
        const DisplayData& d = len == 0 ? DisplayData::blankData() : displays[currentDisplayIndex];
        ui->textEditLMDescription->setText(d.Description());
        ui->labelDisplayTilesGrid->changeDisplay(currentDisplayIndex);
        ui->checkBoxDisplayExtraBit->setChecked(d.ExtraBit());
        ui->spinBoxXPos->setValue(d.X());
        ui->spinBoxYPos->setValue(d.Y());
        ui->checkBoxUseText->setChecked(d.UseText());
        qDebug() << "Use text? " << (d.UseText() ? "True" : "False");
        if (d.UseText()) {
            ui->textEditDisplayText->setText(d.DisplayText());
        } else {
            ui->textEditDisplayText->setText("");
        }
    });

    // useText
    QPalette readOnlyPalette = palette();
    readOnlyPalette.setColor(QPalette::Base, readOnlyPalette.color(QPalette::Window));
    ui->textEditDisplayText->setPalette(readOnlyPalette);
    QObject::connect(ui->checkBoxUseText, &QCheckBox::stateChanged, this, [&]() {
        ui->textEditDisplayText->setReadOnly(!ui->checkBoxUseText->isChecked());
        ui->labelDisplayTilesGrid->setUseText(ui->checkBoxUseText->isChecked());
        QPalette readOnlyPalette = palette();
        if (ui->textEditDisplayText->isReadOnly()) {
            ui->textEditDisplayText->setText("");
            readOnlyPalette.setColor(QPalette::Base, readOnlyPalette.color(QPalette::Window));
        } else {
            readOnlyPalette.setColor(QPalette::Base, readOnlyPalette.color(QPalette::BrightText));
        }
        ui->textEditDisplayText->setPalette(readOnlyPalette);
        if (currentDisplayIndex == -1)
            return;
        displays[currentDisplayIndex].setUseText(ui->checkBoxUseText->isChecked());
        if (!ui->checkBoxUseText->isChecked())
            displays[currentDisplayIndex].setDisplayText("");
    });

    // checkbox or spinner get updated
    QObject::connect(ui->checkBoxDisplayExtraByte, &QCheckBox::stateChanged, this, [&]() {
        if (ui->checkBoxDisplayExtraByte->checkState() == Qt::CheckState::Checked) {
            sprite->dispType = DisplayType::ExtraByte;
            ui->labelDisplayX->setText("ExByte Index:");
            ui->labelDisplayY->setText("Value:");
            ui->spinBoxXPos->setMaximum(12);
            ui->spinBoxYPos->setMaximum(0xFF);
        } else {
            sprite->dispType = DisplayType::XY;
            ui->labelDisplayX->setText("X");
            ui->labelDisplayY->setText("Y");
            ui->spinBoxXPos->setMaximum(15);
            ui->spinBoxYPos->setMaximum(15);
        }
    });
    QObject::connect(ui->checkBoxDisplayExtraBit, &QCheckBox::stateChanged, this, [&]() {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            return;
        }
        auto realIndex = ui->tableViewDisplays->model()->index(ui->tableViewDisplays->currentIndex().row(), 0);
        ui->tableViewDisplays->model()->setData(realIndex, ui->checkBoxDisplayExtraBit->isChecked() ? "True" : "False");
        displays[currentDisplayIndex].setExtraBit(ui->checkBoxDisplayExtraBit->isChecked());
    });
    QObject::connect(ui->spinBoxXPos, QOverload<const QString&>::of(&QSpinBox::textChanged), this, [&](const QString& text) {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            return;
        }
        auto realIndex = ui->tableViewDisplays->model()->index(ui->tableViewDisplays->currentIndex().row(), 1);
        ui->tableViewDisplays->model()->setData(realIndex, text);
        displays[currentDisplayIndex].setX(text.toInt());
    });
    QObject::connect(ui->spinBoxYPos, QOverload<const QString&>::of(&QSpinBox::textChanged), this, [&](const QString& text) {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            return;
        }
        auto realIndex = ui->tableViewDisplays->model()->index(ui->tableViewDisplays->currentIndex().row(), 2);
        ui->tableViewDisplays->model()->setData(realIndex, text);
        displays[currentDisplayIndex].setY(text.toInt());
    });

    // description or displaytext get updated
    QObject::connect(ui->textEditLMDescription, &QTextEdit::textChanged, this, [&]() {
        if (currentDisplayIndex == -1)
            return;
        displays[currentDisplayIndex].setDescription(ui->textEditLMDescription->toPlainText());
    });
    QObject::connect(ui->textEditDisplayText, &QTextEdit::textChanged, this, [&]() {
        if (currentDisplayIndex == -1)
            return;
        qDebug() << currentDisplayIndex << " " << displays.length();
        displays[currentDisplayIndex].setDisplayText(ui->textEditDisplayText->toPlainText());
        ui->labelDisplayTilesGrid->insertText(ui->textEditDisplayText->toPlainText());
    });

    ui->map16GraphicsView->registerMouseClickCallback([&](FullTile tileInfo, int tileNo, SelectorType type) {
        qDebug() << QString::asprintf("Tile number selected is: 0x%03X", tileNo);
        if ((type == SelectorType::Sixteen && tileNo >= 0x300) || (type == SelectorType::Eight && tileNo >= 0xC00)) {
            changeTilePropGroupState(false, ui->map16GraphicsView->getChangeType());
        } else {
            changeTilePropGroupState(true);
        }
        setTilePropGroupState(tileInfo);
        ui->labelDisplayTilesGrid->getCopiedTile()->update(ui->map16GraphicsView->getCopiedTile());
    });

    connect(ui->lineEditTileBL, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "bottom left tile updated";
        ui->map16GraphicsView->tileChanged(ui->lineEditTileBL, TileChangeAction::Number, TileChangeType::BottomLeft, ui->lineEditTileBL->text().toInt(nullptr, 16));
    });
    connect(ui->lineEditTileBR, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "bottom right tile updated";
        ui->map16GraphicsView->tileChanged(ui->lineEditTileBR, TileChangeAction::Number, TileChangeType::BottomRight, ui->lineEditTileBR->text().toInt(nullptr, 16));
    });
    connect(ui->lineEditTileTL, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "top left tile updated";
        ui->map16GraphicsView->tileChanged(ui->lineEditTileTL,TileChangeAction::Number, TileChangeType::TopLeft, ui->lineEditTileTL->text().toInt(nullptr, 16));
    });
    connect(ui->lineEditTileTR, &QLineEdit::editingFinished, this, [&]() {
        qDebug() << "top right tile updated";
        ui->map16GraphicsView->tileChanged(ui->lineEditTileTR,TileChangeAction::Number, TileChangeType::TopRight, ui->lineEditTileTR->text().toInt(nullptr, 16));
    });

    connect(ui->comboBoxTilePalette, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        qDebug() << "palette for tile changed";
        ui->map16GraphicsView->tileChanged(ui->comboBoxTilePalette, TileChangeAction::Palette, ui->map16GraphicsView->getChangeType(), index);
    });

    connect(ui->pushButtonFlipX, &QPushButton::clicked, this, [&](){
       qDebug() << "flip x for tile clicked";
       ui->map16GraphicsView->tileChanged(ui->pushButtonFlipX, TileChangeAction::FlipX, ui->map16GraphicsView->getChangeType());
    });


    connect(ui->pushButtonFlipY, &QPushButton::clicked, this, [&](){
       qDebug() << "flip x for tile clicked";
       ui->map16GraphicsView->tileChanged(ui->pushButtonFlipY, TileChangeAction::FlipY, ui->map16GraphicsView->getChangeType());
    });

    connect(ui->comboBoxSizeType, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, [&](const QString& text) {
        qDebug() << "combo box for size of tile clicked";
        ui->labelDisplayTilesGrid->setSelectorSize(static_cast<SizeSelector>(text.split("x").takeFirst().toInt()));
    });
}

void CFGEditor::changeTilePropGroupState(bool disabled, TileChangeType type) {
    ui->comboBoxTilePalette->setDisabled(disabled);
    ui->pushButtonFlipX->setDisabled(disabled);
    ui->pushButtonFlipY->setDisabled(disabled);
    if (type != TileChangeType::All) {
        ui->lineEditTileBL->setDisabled(true);
        ui->lineEditTileBR->setDisabled(true);
        ui->lineEditTileTR->setDisabled(true);
        ui->lineEditTileTL->setDisabled(true);
        switch (type) {
        case TileChangeType::BottomLeft:
            ui->lineEditTileBL->setDisabled(disabled);
            break;
        case TileChangeType::TopLeft:
            ui->lineEditTileTL->setDisabled(disabled);
            break;
        case TileChangeType::BottomRight:
            ui->lineEditTileBR->setDisabled(disabled);
            break;
        case TileChangeType::TopRight:
            ui->lineEditTileTR->setDisabled(disabled);
            break;
        default:
            Q_ASSERT(false);
        }
    } else {
        ui->lineEditTileBL->setDisabled(disabled);
        ui->lineEditTileBR->setDisabled(disabled);
        ui->lineEditTileTR->setDisabled(disabled);
        ui->lineEditTileTL->setDisabled(disabled);
    }
}

void CFGEditor::setTilePropGroupState(FullTile tileInfo) {
    ui->map16GraphicsView->noSignals = true;
    ui->map16GraphicsView->changePaletteIndex(ui->comboBoxTilePalette, tileInfo);
    qDebug() << QString::asprintf("%d %d %d %d", tileInfo.bottomleft.pal, tileInfo.bottomright.pal, tileInfo.topleft.pal, tileInfo.topright.pal);
    ui->lineEditTileBL->setText(QString::asprintf("%03X", tileInfo.bottomleft.tilenum));
    ui->lineEditTileBR->setText(QString::asprintf("%03X", tileInfo.bottomright.tilenum));
    ui->lineEditTileTR->setText(QString::asprintf("%03X", tileInfo.topright.tilenum));
    ui->lineEditTileTL->setText(QString::asprintf("%03X", tileInfo.topleft.tilenum));
    ui->map16GraphicsView->noSignals = false;
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
    SpritePaletteCreator::ReadPaletteFile(0, 16);
    paletteImages.reserve(SpritePaletteCreator::nSpritePalettes());
    for (int i = 0; i < SpritePaletteCreator::nSpritePalettes(); i++) {
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

void CFGEditor::setupForNormal() {
    changeAllCheckBoxState(false);

    ui->lineEditAsmFile->setEnabled(false);
    ui->lineEditExtraProp1->setEnabled(false);
    ui->lineEditExtraProp2->setEnabled(false);
    ui->spinBoxextraBitClear->setEnabled(false);
    ui->spinBoxextraBitSet->setEnabled(false);
}

void CFGEditor::setupForCustom() {
    changeAllCheckBoxState(false);

    ui->lineEditAsmFile->setEnabled(true);
    ui->lineEditExtraProp1->setEnabled(true);
    ui->lineEditExtraProp2->setEnabled(true);
    ui->spinBoxextraBitClear->setEnabled(true);
    ui->spinBoxextraBitSet->setEnabled(true);

}

void CFGEditor::setupForGenShoot() {
    changeAllCheckBoxState(true);

    ui->lineEditAsmFile->setEnabled(true);
    ui->lineEditExtraProp1->setEnabled(true);
    ui->lineEditExtraProp2->setEnabled(true);
    ui->spinBoxextraBitClear->setEnabled(true);
    ui->spinBoxextraBitSet->setEnabled(true);
}

void CFGEditor::changeAllCheckBoxState(bool state) {
    ui->lineEdit1656->setDisabled(state);
    ui->checkBox1656DiesJumped->setDisabled(state);
    ui->checkBox1656Hopin->setDisabled(state);
    ui->checkBox1656JumpedOn->setDisabled(state);
    ui->checkBox1656Smoke->setDisabled(state);
    ui->objClipCmbBox->setDisabled(state);

    ui->lineEdit1662->setDisabled(state);
    ui->checkBox1662deathframe->setDisabled(state);
    ui->checkBox1662strdown->setDisabled(state);
    ui->sprClipCmbBox->setDisabled(state);

    ui->lineEdit166E->setDisabled(state);
    ui->checkBox166ecape->setDisabled(state);
    ui->checkBox166efireball->setDisabled(state);
    ui->checkBox166esecondpage->setDisabled(state);
    ui->checkBox166esplash->setDisabled(state);
    ui->checkBox166elay2->setDisabled(state);
    ui->paletteComboBox->setDisabled(state);

    ui->lineEdit167a->setDisabled(state);
    ui->checkBox167astar->setDisabled(state);
    ui->checkBox167ablk->setDisabled(state);
    ui->checkBox167aoffscr->setDisabled(state);
    ui->checkBox167astunned->setDisabled(state);
    ui->checkBox167akick->setDisabled(state);
    ui->checkBox167aeveryframe->setDisabled(state);
    ui->checkBox167apowerup->setDisabled(state);
    ui->checkBox167adefaultint->setDisabled(state);

    ui->lineEdit1686->setDisabled(state);
    ui->checkBox1686Inedible->setDisabled(state);
    ui->checkBox1686mouth->setDisabled(state);
    ui->checkBox1686ground->setDisabled(state);
    ui->checkBox1686sprint->setDisabled(state);
    ui->checkBox1686dir->setDisabled(state);
    ui->checkBox1686goalcoin->setDisabled(state);
    ui->checkBox1686spawnSpr->setDisabled(state);
    ui->checkBox1686noObjInt->setDisabled(state);

    ui->lineEdit190f->setDisabled(state);
    ui->checkBox190fbelow->setDisabled(state);
    ui->checkBox190fgoalpass->setDisabled(state);
    ui->checkBox190fsliding->setDisabled(state);
    ui->checkBox190ffivefire->setDisabled(state);
    ui->checkBox190fupysp->setDisabled(state);
    ui->checkBox190fdeathframe->setDisabled(state);
    ui->checkBox190fnosilver->setDisabled(state);
    ui->checkBox190fwallstuck->setDisabled(state);
}

void CFGEditor::bindSpriteProp() {
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
        // disable stuff
        sprite->type = index;
        switch (index) {
        case 0:
            setupForNormal();
            break;
        case 1:
            setupForCustom();
            break;
        case 2:
            setupForGenShoot();
            break;
        default:
            Q_ASSERT(false);
        }
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
    setupForNormal();
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
    displays.clear();
    for (auto p : models)
        delete p;
    delete full8x8Bitmap;
    delete view8x8;
    delete viewPalette;
    delete hexValidator;
    delete hexCompleter;
    delete hexNumberList;
    delete ui;
}

