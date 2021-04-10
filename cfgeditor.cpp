#include "cfgeditor.h"
#include "./ui_cfgeditor.h"
CFGEditor::CFGEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CFGEditor)
    , dialog(new QFileDialog)
{
    this->setFixedSize(701, 511);

    ui->setupUi(this);
    QMenuBar* mb = menuBar();
    setUpMenuBar(mb);
    mb->show();
    setMenuBar(mb);
}

DefaultMissingImpl::DefaultMissingImpl(const QString& impl_name) : name(impl_name) {
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
    file->addAction("&New", qApp, DefaultMissingImpl("New"), Qt::CTRL + Qt::Key_N);

    file->addSeparator();

    file->addAction("&Open File", qApp, [&]() {
        dialog->setAcceptMode(QFileDialog::AcceptOpen);
        dialog->setVisible(true);
    }, Qt::CTRL + Qt::Key_O);

    file->addAction("&Save", qApp, DefaultMissingImpl("Save"), Qt::CTRL + Qt::Key_S);

    file->addAction("&Save As", qApp, [&]() {
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setVisible(true);
    }, Qt::CTRL + Qt::ALT + Qt::Key_S);

    display->addAction("&Load Custom Map16", qApp, DefaultMissingImpl("Load Custom Map16"));
    display->addAction("&Load Custom GFX33", qApp, DefaultMissingImpl("Load Custom GFX33"));
    display->addAction("&Palette", qApp, DefaultMissingImpl("Palette"));
    display->addAction("&8x8 Tile Selector", qApp, DefaultMissingImpl("8x8 Tile Selector"));

    mb->addMenu(file);
    mb->addMenu(display);
}

CFGEditor::~CFGEditor()
{
    delete dialog;
    delete ui;
}

