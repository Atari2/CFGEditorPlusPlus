#include "cfgeditor.h"
#include "./ui_cfgeditor.h"
#include "eightbyeightview.h"
#include <QStyledItemDelegate>

CFGEditor::CFGEditor(const QStringList& argv, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CFGEditor)
    , sprite(new JsonSprite)
    , hexValidator(new QRegularExpressionValidator{QRegularExpression(R"([A-Fa-f0-9]+)")})
    , hexNumberList(new QStringList(0x100))
    , copiedTile()
    , displays()
{
	setWindowIcon(QIcon{":/VioletEgg.ico"});
    ui->setupUi(this);
    setFixedSize(size());
    setSizePolicy(QSizePolicy());
    statusBar()->setSizeGripEnabled(false);
    setUpImages();
    view8x8Container = new EightByEightViewContainer(new EightByEightView(new QGraphicsScene), this->ui->paletteComboBox);
	paletteContainer = new PaletteContainer(new PaletteView(new QGraphicsScene));
    ui->labelDisplayTilesGrid->attachMap16View(ui->map16GraphicsView);
    loadFullbitmap();
    ui->map16GraphicsView->setControllingLabel(ui->labelTileNo);
    QMenuBar* mb = menuBar();
    initCompleter();
    setUpMenuBar(mb);
    bindSpriteProp();
    setCollectionModel();
    setDisplayModel();
    setGFXInfoModel();
    bindCollectionButtons();
    bindDisplayButtons();
    bindGFXSelector();
    ui->Default->setAutoFillBackground(true);
    mb->show();
    setMenuBar(mb);
    deleteInstaller();
    if (argv.size() > 0) {
        sprite->from_file(argv[0]);
        resetTweaks();
        std::for_each(sprite->collections.cbegin(), sprite->collections.cend(), [&](auto& coll) {
            collectionModel->appendRow(CollectionDataModel::fromCollection(coll));
        });
        ui->map16GraphicsView->setMap16(sprite->map16);
        ui->labelDisplayTilesGrid->deserializeDisplays(sprite->displays, ui->map16GraphicsView);
        populateDisplays();
    }
}

void CFGEditor::deleteInstaller() {
    if (QFile::exists(QDir::currentPath() + "/ICFGEditor.exe")) {
        // if cfg editor has been just installed, just remove the installer, since it's a waste of space
        QFile file(QDir::currentPath() + "/ICFGEditor.exe");
        file.remove();
    }
}

void CFGEditor::closeEvent(QCloseEvent *event) {
	view8x8Container->close();
	paletteContainer->close();
    QMainWindow::closeEvent(event);
}


void CFGEditor::initCompleter() {
    for (int i = 0; i <= 0xFF; i++) {
        hexNumberList->append(QString::asprintf("%02X", i));
    }
    hexCompleter = new QCompleter(*hexNumberList, this);
    hexCompleter->setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
}

void CFGEditor::loadFullbitmap(int index, bool justPalette) {
    if (index == -1)
        index = ui->paletteComboBox->currentIndex();
    QVector<QString> gfxFiles{ui->lineEditGFXSp0->text(), ui->lineEditGFXSp1->text(), ui->lineEditGFXSp2->text(), ui->lineEditGFXSp3->text()};
    if (full8x8Bitmap) {
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
    if (!justPalette) {
        ui->map16GraphicsView->readInternalMap16File();
    }
	view8x8Container->updateForChange(full8x8Bitmap, true);
    if (!justPalette) {
        ui->labelDisplayTilesGrid->redraw();
    }
}

void CFGEditor::setUpMenuBar(QMenuBar* mb) {
    QMenu* file = new QMenu("&File");
    QMenu* display = new QMenu("&Display");
    file->addAction("&New", Qt::CTRL | Qt::Key_N, qApp, [&]() {
        if (sprite->name().length() != 0) {
            auto res = QMessageBox::question(this,
                                             "One file is already open",
                                             "Do you want to save it before opening a new one?",
                                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
            if (res == QMessageBox::Yes) {
                sprite->to_file(QFileDialog::getSaveFileName(this, tr("Save file"), sprite->name(), tr("JSON (*.json)")));
            } else if (res == QMessageBox::Abort) {
                return;
            }
        }
        resetAll();
        resetTweaks();
    });

    file->addSeparator();

    file->addAction("&Open File", Qt::CTRL | Qt::Key_O, qApp, [&]() {
        if (sprite->name().length() != 0) {
            auto res = QMessageBox::question(this,
                                  "One file is already open",
                                  "Do you want to save it before opening the other one?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
            if (res == QMessageBox::Yes) {
                saveSprite();
                sprite->to_file(QFileDialog::getSaveFileName(this, tr("Save file"), sprite->name(), tr("JSON (*.json)")));
            } else if (res == QMessageBox::Abort) {
                return;
            }
        }
        resetAll();
        auto file = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("JSON (*.json);;CFG (*.cfg)"));
        sprite->from_file(file);
        resetTweaks();
        std::for_each(sprite->collections.cbegin(), sprite->collections.cend(), [&](auto& coll) {
            collectionModel->appendRow(CollectionDataModel::fromCollection(coll));
        });
		ui->checkBoxDisplayExtraByte->setChecked(sprite->dispType == DisplayType::ExtraByte);
        ui->map16GraphicsView->setMap16(sprite->map16);
        ui->labelDisplayTilesGrid->deserializeDisplays(sprite->displays, ui->map16GraphicsView);
        populateDisplays();
    });

    file->addAction("&Save", Qt::CTRL | Qt::Key_S, qApp, [&]() {
        saveSprite();
        sprite->to_file();
    });

    file->addAction("&Save As", Qt::CTRL | Qt::ALT | Qt::Key_S, qApp, [&]() {
        saveSprite();
        sprite->to_file(QFileDialog::getSaveFileName(this, tr("Save file"), sprite->name(), tr("JSON (*.json);;CFG (*.cfg)")));
    });

    display->addAction("&Load Custom Map16", qApp, [&]() {
        QString name = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("M16 (*.m16);;Map16 (*.map16)"));
        if (name.length() == 0)
            return;
        if (name.endsWith(".m16") && !assert_filesize(name, kb(8)))
            return;
        ui->map16GraphicsView->readExternalMap16File(name);
    });
    display->addAction("&Load Custom GFX33", qApp, [&]() {
        QString name = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("BIN (*.bin"));
        if (name.length() == 0)
            return;
        if (!assert_filesize(name, kb(12)))
            return;
        SnesGFXConverter::setCustomExanimation(name);
        ui->map16GraphicsView->drawInternalMap16File();
    });
    display->addAction("&Palette", qApp, [&]() {
        qDebug() << "Opening palette viewer";
		paletteContainer->updateContainer(SpritePaletteCreator::MakeFullPalette());
    });
    display->addAction("&8x8 Tile Viewer", qApp, [&]() {
        qDebug() << "Opening 8x8 tile selector";
		view8x8Container->updateForChange(full8x8Bitmap);
    });
    display->addAction("&Load External GFX Files", qApp, [&]() {
        qDebug() << "Opening external gfx file loader";
        ui->map16GraphicsView->loadExternalGraphics();
    });

    mb->addMenu(file);
    mb->addMenu(display);
}

void CFGEditor::resetAll() {
    sprite->reset();
    ui->tableView->model()->removeRows(0, ui->tableView->model()->rowCount());
    ui->tableViewDisplays->model()->removeRows(0, ui->tableViewDisplays->model()->rowCount());
    displays.clear();
    currentDisplayIndex = -1;
    ui->labelDisplayTilesGrid->reset();
    ui->checkBoxDisplayExtraByte->setChecked(false);
}

void CFGEditor::saveSprite() {
    sprite->displays.clear();
    sprite->collections.clear();
    ui->labelDisplayTilesGrid->serializeDisplays(displays);
    for (auto& disp : displays)
        sprite->addDisplay(createDisplay(disp));
    sprite->setMap16(ui->map16GraphicsView->getMap16());
    sprite->addCollections(ui->tableView);
}

void CFGEditor::populateDisplays() {
    currentDisplayIndex = -1;
    QSignalBlocker block{ui->tableViewDisplays};
    for (auto& d : sprite->displays) {
        DisplayData display{d};
        displayModel->appendRow(display.itemsFromDisplay());
        gfxinfoModel->appendRow(display.GFXInfo().itemsFromGFXInfo());
        displays.append(display);
        ui->checkBoxDisplayExtraBit->setChecked(display.ExtraBit());
        ui->checkBoxUseText->setChecked(display.UseText());
        ui->spinBoxXPos->setValue(display.XOrIndex());
        ui->spinBoxYPos->setValue(display.YOrValue());
        ui->textEditLMDescription->setText(display.Description());
        if (display.UseText())
           ui->textEditDisplayText->setText(display.DisplayText());
    }
}

Display CFGEditor::createDisplay(const DisplayData& data) {
    QVector<Tile> tiles;
    tiles.reserve(data.Tiles().length());
    for (auto& t : data.Tiles()) {
        tiles.append({t.XOffset(), t.YOffset(), t.TileNumber()});
    }
    GFXInfo info{
        SingleGFXFile{data.GFXInfo().sp0().Separate(), data.GFXInfo().sp0().Value()},
        SingleGFXFile{data.GFXInfo().sp1().Separate(), data.GFXInfo().sp1().Value()},
        SingleGFXFile{data.GFXInfo().sp2().Separate(), data.GFXInfo().sp2().Value()},
        SingleGFXFile{data.GFXInfo().sp3().Separate(), data.GFXInfo().sp3().Value()}
    };
    return {data.Description(), tiles, data.ExtraBit(), data.XOrIndex(), data.YOrValue(), data.UseText(), data.DisplayText(), info};
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
    displayModel = new QStandardItemModel;
    QStringList labelList{"ExtraBit", "X", "Y"};
    displayModel->setHorizontalHeaderLabels(labelList);
    ui->tableViewDisplays->setFixedSize(ui->tableViewDisplays->size());
    ui->tableViewDisplays->setModel(displayModel);
    ui->tableViewDisplays->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewDisplays->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableViewDisplays->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->tableViewDisplays->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
}

class CustomItemDelegate : public QStyledItemDelegate {
    QValidator* m_validator = nullptr;
    QStandardItemModel* m_model = nullptr;
public:
    CustomItemDelegate(QStandardItemModel* model, QObject* parent = nullptr, QValidator* validator = nullptr) : QStyledItemDelegate(parent) {
        m_validator = validator;
        m_model = model;
    }
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
        if (index.column() % 2 == 1) {
            return static_cast<const QStyledItemDelegate*>(this)->createEditor(parent, option, index);
        } else {
            QLineEdit* lineEdit = new QLineEdit{index.data().toString(), parent};
            lineEdit->setValidator(m_validator);
            QObject::connect(lineEdit, &QLineEdit::editingFinished, this, [index, this, lineEdit]() {
                auto newString = "0x" + lineEdit->text().toUpper();
                m_model->item(index.row(), index.column())->setText(newString);
            });
            return lineEdit;
        }
    }
};

void CFGEditor::setGFXInfoModel() {
    gfxinfoModel = new QStandardItemModel;
    QStringList labelList{"Sp0", "Sep.", "Sp1", "Sep.", "Sp2", "Sep.", "Sp3", "Sep."};
    gfxinfoModel->setHorizontalHeaderLabels(labelList);
    ui->tableViewGfxInfo->setFixedSize(ui->tableViewGfxInfo->size());
    ui->tableViewGfxInfo->setModel(gfxinfoModel);
    ui->tableViewGfxInfo->setItemDelegate(new CustomItemDelegate(gfxinfoModel, ui->tableViewGfxInfo, hexValidator));
    ui->tableViewGfxInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewGfxInfo->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableViewGfxInfo->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    ui->tableViewGfxInfo->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    QObject::connect(gfxinfoModel, &QStandardItemModel::itemChanged, this, [this](QStandardItem* item){
        auto row = item->index().row();
        auto col = item->index().column();
        if (col % 2 == 1) {
            displays[row].setSeparate(item->checkState() == Qt::Checked, col / 2);
        } else {
            // chop 0x and convert to hex
            auto str = item->text();
            displays[row].setGfxInfoValue(QStringView{str}.mid(2).toInt(nullptr, 16), col / 2);
        }
    });
}

void CFGEditor::setCollectionModel() {
    collectionModel = new QStandardItemModel;
    QStringList labelList{};
    labelList.append("Name");
    labelList.append("Extra bit");
    for (int i = 1; i <= 12; i++) {
        labelList.append(QString::asprintf("Ex%d", i));
    }
    collectionModel->setHorizontalHeaderLabels(labelList);
    ui->tableView->setFixedSize(ui->tableView->size());
    ui->tableView->setModel(collectionModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->resizeSection(1, 60);
    for (int i = 2; i < 14; i++) {
        ui->tableView->horizontalHeader()->resizeSection(i, 30);
        ui->tableView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    }
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
    ui->toolButton8x8Mode->setToolTip("Open 8x8 Viewer");
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
		view8x8Container->updateForChange(full8x8Bitmap);
    });
    QObject::connect(ui->toolButtonPalette, &QToolButton::clicked, this, [&]() {
        qDebug() << "Palette button clicked";
		paletteContainer->updateContainer(SpritePaletteCreator::MakeFullPalette());
    });
    splitSetGFx();
	QObject::connect(ui->comboBoxGFXSet, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&, splitSetGFx](int) {
        qDebug() << "Index of GFX set changed";
        splitSetGFx();
        loadFullbitmap();
        if (currentDisplayIndex != -1)
            ui->labelDisplayTilesGrid->redrawNoSort();
    });
    changeTilePropGroupState(true);
	QObject::connect(paletteContainer, &PaletteContainer::paletteChanged, this, [&](){
        qDebug() << "Custom signal change palette received";
        loadFullbitmap();
        for (int i = 0; i < SpritePaletteCreator::nSpritePalettes(); i++) {
            paletteImages[i] = SpritePaletteCreator::MakePalette(i);
        }
        ui->label->setPixmap(paletteImages[ui->paletteComboBox->currentIndex()]);
    });

    QObject::connect(ui->toolButtonGFXSp0, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName(this, "Open GFX File", "", tr("GFX Files (*.bin)"));
        if (filename.length() == 0)
            return;
        if (!assert_filesize(filename, kb(4))) {
            return;
        }
        ui->lineEditGFXSp0->setText(filename);
        loadFullbitmap();
    });

    QObject::connect(ui->toolButtonGFXSp1, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName(this, "Open GFX File", "", tr("GFX Files (*.bin)"));
        if (filename.length() == 0)
            return;
        if (!assert_filesize(filename, kb(4)))
            return;
        ui->lineEditGFXSp1->setText(filename);
        loadFullbitmap();
    });

    QObject::connect(ui->toolButtonGFXSp2, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName(this, "Open GFX File", "", tr("GFX Files (*.bin)"));
        if (filename.length() == 0)
            return;
        if (!assert_filesize(filename, kb(4)))
            return;
        ui->lineEditGFXSp2->setText(filename);
        loadFullbitmap();
    });

    QObject::connect(ui->toolButtonGFXSp3, &QToolButton::clicked, this, [&]() {
        QString filename = QFileDialog::getOpenFileName(this, "Open GFX File", "", tr("GFX Files (*.bin)"));
        if (filename.length() == 0)
            return;
        if (!assert_filesize(filename, kb(4)))
            return;
        ui->lineEditGFXSp3->setText(filename);
        loadFullbitmap();
    });
}

void CFGEditor::addCloneRow() {
    DisplayData display(displays[currentDisplayIndex]);
    displayModel->appendRow(display.itemsFromDisplay());
    gfxinfoModel->appendRow(display.GFXInfo().itemsFromGFXInfo());
    advanceDisplayIndex();
    displays.insert(currentDisplayIndex, display);
    ui->checkBoxDisplayExtraBit->setChecked(display.ExtraBit());
    ui->checkBoxUseText->setChecked(display.UseText());
    ui->spinBoxXPos->setValue(display.XOrIndex());
    ui->spinBoxYPos->setValue(display.YOrValue());
    ui->textEditLMDescription->setText(display.Description());
    if (display.UseText())
       ui->textEditDisplayText->setText(display.DisplayText());

}

void CFGEditor::addBlankRow() {
    DisplayData display = DisplayData::blankData();
    displays.insert(currentDisplayIndex + 1, display);
    advanceDisplayIndex();
    displayModel->appendRow(display.itemsFromDisplay());
    qDebug() << displays.length();
    ui->checkBoxDisplayExtraBit->setChecked(false);
    ui->checkBoxUseText->setChecked(false);
    ui->spinBoxXPos->setValue(0);
    ui->spinBoxYPos->setValue(0);
    ui->textEditLMDescription->setText("");
    ui->textEditDisplayText->setText("");

    gfxinfoModel->appendRow(display.GFXInfo().itemsFromGFXInfo());
}

void CFGEditor::removeExistingRow() {
    displays.removeAt(currentDisplayIndex);
    currentDisplayIndex--;
    qDebug() << currentDisplayIndex;
    ui->tableViewDisplays->model()->removeRow(ui->tableViewDisplays->currentIndex().row());
    ui->tableViewGfxInfo->model()->removeRow(ui->tableViewGfxInfo->currentIndex().row());
}

void CFGEditor::advanceDisplayIndex() {
    currentDisplayIndex++;
    qDebug() << "Row count " << ui->tableViewDisplays->model()->rowCount();
    qDebug() << "Row got added at " << currentDisplayIndex << " and at " << ui->tableViewDisplays->currentIndex().row();
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
    QObject::connect(ui->tableViewGfxInfo->model(), &QAbstractItemModel::rowsInserted, this, [&]() {
        qDebug() << "Rows have been inserted " << currentDisplayIndex;
        if (ui->tableViewGfxInfo->currentIndex().isValid())
            ui->tableViewGfxInfo->setCurrentIndex(ui->tableViewGfxInfo->model()->index(ui->tableViewGfxInfo->currentIndex().row() + 1, ui->tableViewGfxInfo->currentIndex().column()));
        else
            ui->tableViewGfxInfo->setCurrentIndex(ui->tableViewGfxInfo->model()->index(currentDisplayIndex, 0));
    });
    // new, delete, clone
    QObject::connect(ui->pushButtonNewDisplay, &QPushButton::clicked, this, [&]() {
        qDebug() << "New row button pushed";
        ui->labelDisplayTilesGrid->addDisplay(currentDisplayIndex);
        addBlankRow();
    });
    QObject::connect(ui->pushButtonCloneDisplay, &QPushButton::clicked, this, [&]() {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            DefaultAlertImpl(this, "Select a row before cloning")();
            return;
        }
        qDebug() << "Clone display button clicked";
        addCloneRow();
        ui->labelDisplayTilesGrid->cloneDisplay(currentDisplayIndex);
    });
    QObject::connect(ui->pushButtonDeleteDisplay, &QPushButton::clicked, this, [&]() {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            DefaultAlertImpl(this, "Select a row before deleting")();
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
        {
            QSignalBlocker blocker{ui->tableViewGfxInfo};
            int column = 0;
            if (auto idx = ui->tableViewGfxInfo->currentIndex(); idx.isValid()) {
                column = ui->tableViewGfxInfo->currentIndex().column();
            }
            int row = currentDisplayIndex;
            ui->tableViewGfxInfo->setCurrentIndex(ui->tableViewGfxInfo->model()->index(row, column));
        }
        qDebug() << currentDisplayIndex << " " << ui->tableViewDisplays->model()->rowCount();
        const DisplayData& d = len == 0 ? DisplayData::blankData() : displays[currentDisplayIndex];
        ui->textEditLMDescription->setText(d.Description());
        ui->labelDisplayTilesGrid->changeDisplay(currentDisplayIndex);
        ui->checkBoxDisplayExtraBit->setChecked(d.ExtraBit());
        ui->spinBoxXPos->setValue(d.XOrIndex());
        ui->spinBoxYPos->setValue(d.YOrValue());
        ui->checkBoxUseText->setChecked(d.UseText());
        qDebug() << "Use text? " << (d.UseText() ? "True" : "False");
        if (d.UseText()) {
            ui->textEditDisplayText->setText(d.DisplayText());
        } else {
            ui->textEditDisplayText->setText("");
        }
    });

    QObject::connect(ui->tableViewGfxInfo->selectionModel(),
                     QOverload<const QModelIndex&, const QModelIndex&>::of(&QItemSelectionModel::currentRowChanged), this, [&](const QModelIndex& now, const QModelIndex& pre) {
        qDebug() << "current row changed called";
        int len = displays.length();
        if (ui->tableViewGfxInfo->model()->rowCount() == 0)
            return;
        if (currentDisplayIndex == -1 && len == 0)
            return;
        if (now.row() == -1)
            currentDisplayIndex = pre.row();
        else
            currentDisplayIndex = now.row();
        if (currentDisplayIndex > len - 1)
            currentDisplayIndex = 0;
        qDebug() << currentDisplayIndex << " " << ui->tableViewGfxInfo->model()->rowCount();
        {
            QSignalBlocker blocker{ui->tableViewDisplays};
            int column = 0;
            if (auto idx = ui->tableViewDisplays->currentIndex(); idx.isValid()) {
                column = ui->tableViewDisplays->currentIndex().column();
            }
            int row = currentDisplayIndex;
            ui->tableViewDisplays->setCurrentIndex(ui->tableViewDisplays->model()->index(row, column));
        }
        const DisplayData& d = len == 0 ? DisplayData::blankData() : displays[currentDisplayIndex];
        ui->textEditLMDescription->setText(d.Description());
        ui->labelDisplayTilesGrid->changeDisplay(currentDisplayIndex);
        ui->checkBoxDisplayExtraBit->setChecked(d.ExtraBit());
        ui->spinBoxXPos->setValue(d.XOrIndex());
        ui->spinBoxYPos->setValue(d.YOrValue());
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
            QStringList labelList{"ExtraBit", "Index", "Value"};
            displayModel->setHorizontalHeaderLabels(labelList);
            sprite->dispType = DisplayType::ExtraByte;
            ui->labelDisplayX->setText("ExByte Index:");
            ui->labelDisplayY->setText("Value:");
            ui->spinBoxXPos->setMaximum(12);
            ui->spinBoxYPos->setMaximum(0xFF);
        } else {
            QStringList labelList{"ExtraBit", "X", "Y"};
            displayModel->setHorizontalHeaderLabels(labelList);
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
        ui->tableViewDisplays->model()->setData(realIndex, ui->checkBoxDisplayExtraBit->isChecked() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        displays[currentDisplayIndex].setExtraBit(ui->checkBoxDisplayExtraBit->isChecked());
    });
    QObject::connect(ui->spinBoxXPos, QOverload<const QString&>::of(&QSpinBox::textChanged), this, [&](const QString& text) {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            return;
        }
        auto realIndex = ui->tableViewDisplays->model()->index(ui->tableViewDisplays->currentIndex().row(), 1);
        ui->tableViewDisplays->model()->setData(realIndex, text);
        displays[currentDisplayIndex].setXOrIndex(text.toInt());
    });
    QObject::connect(ui->spinBoxYPos, QOverload<const QString&>::of(&QSpinBox::textChanged), this, [&](const QString& text) {
        if (!ui->tableViewDisplays->currentIndex().isValid()) {
            return;
        }
        auto realIndex = ui->tableViewDisplays->model()->index(ui->tableViewDisplays->currentIndex().row(), 2);
        ui->tableViewDisplays->model()->setData(realIndex, text);
        displays[currentDisplayIndex].setYOrValue(text.toInt());
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
        if (index == 8) {
            DefaultAlertImpl(this, "This palette is not supposed to be selected by the user")();
            QSignalBlocker block{ui->comboBoxTilePalette};
            ui->comboBoxTilePalette->setCurrentIndex(7);
            return;
        }
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
    QSignalBlocker bl{ui->lineEditTileBL};
    QSignalBlocker br{ui->lineEditTileBR};
    QSignalBlocker tr{ui->lineEditTileTR};
    QSignalBlocker tl{ui->lineEditTileTL};
    ui->lineEditTileBL->setText(QString::asprintf("%03X", tileInfo.bottomleft.tilenum));
    ui->lineEditTileBR->setText(QString::asprintf("%03X", tileInfo.bottomright.tilenum));
    ui->lineEditTileTR->setText(QString::asprintf("%03X", tileInfo.topright.tilenum));
    ui->lineEditTileTL->setText(QString::asprintf("%03X", tileInfo.topleft.tilenum));
    ui->map16GraphicsView->noSignals = false;
}

void CFGEditor::bindCollectionButtons() {
    QObject::connect(ui->newCollButton, &QPushButton::clicked, this, [&]() {
        qDebug() << "New collection button clicked";
        collectionModel->appendRow(CollectionDataModel().getRow(ui));
    });
    QObject::connect(ui->cloneCollButton, &QPushButton::clicked, this, [&]() {
        if (!ui->tableView->currentIndex().isValid()) {
            DefaultAlertImpl(this, "Select a row before cloning")();
            return;
        }
        qDebug() << "Clone collection button clicked";
        CollectionDataModel model = CollectionDataModel::fromIndex(ui->tableView->currentIndex().row(), ui->tableView);
        collectionModel->appendRow(model.getRow());
    });
    QObject::connect(ui->deleteCollButton, &QPushButton::clicked, this, [&]() {
        if (!ui->tableView->currentIndex().isValid()) {
            DefaultAlertImpl(this, "Select a row before deleting")();
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

    ui->extraPropByte2Bit6CheckBox->setEnabled(false);
    ui->extraPropByte2Bit7CheckBox->setEnabled(false);
}

void CFGEditor::setupForCustom() {
    changeAllCheckBoxState(false);

    ui->lineEditAsmFile->setEnabled(true);
    ui->lineEditExtraProp1->setEnabled(true);
    ui->lineEditExtraProp2->setEnabled(true);
    ui->spinBoxextraBitClear->setEnabled(true);
    ui->spinBoxextraBitSet->setEnabled(true);

    ui->extraPropByte2Bit6CheckBox->setEnabled(true);
    ui->extraPropByte2Bit7CheckBox->setEnabled(true);

}

void CFGEditor::setupForGenShootOther() {
    changeAllCheckBoxState(true);

    ui->lineEditAsmFile->setEnabled(true);
    ui->lineEditExtraProp1->setEnabled(true);
    ui->lineEditExtraProp2->setEnabled(true);
    ui->spinBoxextraBitClear->setEnabled(true);
    ui->spinBoxextraBitSet->setEnabled(true);

    ui->extraPropByte2Bit6CheckBox->setEnabled(false);
    ui->extraPropByte2Bit7CheckBox->setEnabled(false);
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
        {
            QSignalBlocker blocker1{ui->extraPropByte2Bit6CheckBox};
            QSignalBlocker blocker2{ui->extraPropByte2Bit7CheckBox};
            ui->extraPropByte2Bit6CheckBox->setCheckState(sprite->extraProp2 & 0x40 ? Qt::Checked : Qt::Unchecked);
            ui->extraPropByte2Bit7CheckBox->setCheckState(sprite->extraProp2 & 0x80 ? Qt::Checked : Qt::Unchecked);
        }
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
		case 3:
			setupForGenShootOther();
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

    QObject::connect(ui->extraPropByte2Bit6CheckBox, &QCheckBox::stateChanged, this, [&](int state) {
        if (state == Qt::Checked) {
            sprite->extraProp2 |= 0x40;
        } else {
            sprite->extraProp2 &= ~0x40;
        }
        ui->lineEditExtraProp2->setText(QString::asprintf("%02X", sprite->extraProp2));
    });

    QObject::connect(ui->extraPropByte2Bit7CheckBox, &QCheckBox::stateChanged, this, [&](int state) {
        if (state == Qt::Checked) {
            sprite->extraProp2 |= 0x80;
        } else {
            sprite->extraProp2 &= ~0x80;
        }
        ui->lineEditExtraProp2->setText(QString::asprintf("%02X", sprite->extraProp2));
    });
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
        loadFullbitmap(-1, true);
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
    delete collectionModel;
    delete displayModel;
    delete full8x8Bitmap;
	delete view8x8Container;
	delete paletteContainer;
    delete hexValidator;
    delete hexCompleter;
    delete hexNumberList;
    delete ui;
}

