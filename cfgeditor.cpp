#include "cfgeditor.h"
#include "./ui_cfgeditor.h"
CFGEditor::CFGEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CFGEditor)
{
    sprite = nullptr;
    this->setFixedSize(802, 600);
    ui->setupUi(this);
    QMenuBar* mb = menuBar();

    // placeholder test
    QObject::connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, DefaultMissingImpl("TextChanged"));
    setUpMenuBar(mb);
    mb->show();
    setMenuBar(mb);
}

DefaultMissingImpl::DefaultMissingImpl(const QString& impl_name) : name(impl_name) {
    // yes, we construct this here, and we do not call delete on it
    // for some weird reason if I put delete messageBox in the destructor of DefaultMissingImpl
    // the app will segfault with invalid memory access and clang tidy tells me that
    // I'm trying to delete release memory ¯\_(ツ)_/¯, I'll just leave it be
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


void CFGEditor::setUpMenuBar(QMenuBar* mb) {
    QMenu* file = new QMenu("&File");
    QMenu* display = new QMenu("&Display");
    file->addAction("&New", qApp, DefaultMissingImpl("New"), Qt::CTRL | Qt::Key_N);

    file->addSeparator();

    file->addAction("&Open File", qApp, [&]() {
        if (sprite != nullptr) {
            delete sprite;
            DefaultMissingImpl("Opening file while another is already open")();
        }
        auto file = QFileDialog::getOpenFileName();
        sprite = new JsonSprite(file);
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

CFGEditor::~CFGEditor()
{
    delete ui;
}

