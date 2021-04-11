#include "cfgeditor.h"
#include "./ui_cfgeditor.h"

CFGEditor::CFGEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CFGEditor)
    , sprite(new JsonSprite)
    , hexValidator(new QRegularExpressionValidator{QRegularExpression(R"([A-Fa-f0-9]{0,2})")})
    , hexNumberList(new QStringList(0x100))
{
    SpritePaletteCreator::ReadPaletteFile(0, 22);
    paletteImages.reserve(SpritePaletteCreator::npalettes());
    for (int i = 0; i < SpritePaletteCreator::npalettes(); i++) {
        paletteImages.append(SpritePaletteCreator::MakePalette(i));
    }
    this->setFixedSize(802, 600);
    ui->setupUi(this);
    QMenuBar* mb = menuBar();
    initCompleter();
    setUpMenuBar(mb);
    setUpTweak();
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
                sprite->to_file(QFileDialog::getSaveFileName());
            } else if (res == QMessageBox::Abort) {
                return;
            }
        }
        auto file = QFileDialog::getOpenFileName();
        sprite->from_file(file);
        resetTweaks();
    }, Qt::CTRL | Qt::Key_O);

    file->addAction("&Save", qApp, [&]() { sprite->to_file(); }, Qt::CTRL | Qt::Key_S);

    file->addAction("&Save As", qApp, [&]() {
        auto name = QFileDialog::getSaveFileName();
        sprite->to_file(name);
    }, Qt::CTRL | Qt::ALT | Qt::Key_S);

    display->addAction("&Load Custom Map16", qApp, DefaultMissingImpl("Load Custom Map16"));
    display->addAction("&Load Custom GFX33", qApp, DefaultMissingImpl("Load Custom GFX33"));
    display->addAction("&Palette", qApp, DefaultMissingImpl("Palette"));
    display->addAction("&8x8 Tile Selector", qApp, DefaultMissingImpl("8x8 Tile Selector"));

    mb->addMenu(file);
    mb->addMenu(display);
}

void CFGEditor::resetTweaks() {
    ui->lineEdit1656->setText(QString::asprintf("%02X", sprite->t1656.to_byte()));
    emit ui->lineEdit1656->editingFinished();
    ui->lineEdit166E->setText(QString::asprintf("%02X", sprite->t166e.to_byte()));
    emit ui->lineEdit166E->editingFinished();
}

void CFGEditor::setUpTweak() {
    // 1656
    ui->lineEdit1656->setMaxLength(2);
    ui->lineEdit1656->setValidator(hexValidator);
    ui->lineEdit1656->setCompleter(hexCompleter);
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
        sprite->t1656.objclip = index;
        ui->lineEdit1656->setText(QString::asprintf("%02X", sprite->t1656.to_byte()));
    });

    // 166E
    ui->lineEdit166E->setMaxLength(2);
    ui->lineEdit166E->setValidator(hexValidator);
    ui->lineEdit166E->setCompleter(hexCompleter);
    ui->paletteComboBox->setFixedSize(36, 24);
    ui->label->setFixedSize(24 * 8, 24);
    ui->label->setPixmap(paletteImages[0]);
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
        ui->label->setPixmap(paletteImages[index]);
        sprite->t166e.palette = index;
        ui->lineEdit166E->setText(QString::asprintf("%02X", sprite->t166e.to_byte()));
    });
}

CFGEditor::~CFGEditor()
{
    delete hexValidator;
    delete hexCompleter;
    delete hexNumberList;
    delete ui;
}

