#include "cfgeditor.h"

#include <QApplication>
#include <QCommandLineParser>

static CFGEditorCommandLineOptions parseCommandLineOptions(const QCoreApplication &application);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CFGEditorCommandLineOptions options = parseCommandLineOptions(a);
    CFGEditor w{};
    w.show();
    w.applyCommandLineOptions(options);
    return a.exec();
}

static CFGEditorCommandLineOptions parseCommandLineOptions(const QCoreApplication &application) {
    QCommandLineParser parser;
    parser.setApplicationDescription("CFGEditorPlusPlus");
    parser.addHelpOption();

    QCommandLineOption paletteOption("palette", "Palette file.", "file");
    QCommandLineOption sp1Option("sp1", "SP1 file.", "file");
    QCommandLineOption sp2Option("sp2", "SP2 file.", "file");
    QCommandLineOption sp3Option("sp3", "SP3 file.", "file");
    QCommandLineOption sp4Option("sp4", "SP4 file.", "file");

    parser.addOption(paletteOption);
    parser.addOption(sp1Option);
    parser.addOption(sp2Option);
    parser.addOption(sp3Option);
    parser.addOption(sp4Option);

    parser.process(application);

    CFGEditorCommandLineOptions opts;
    const auto positional = parser.positionalArguments();
    if (!positional.isEmpty()) {
        opts.cfgFile = positional.first();
    }

    if (parser.isSet(paletteOption)) {
        opts.palette = QFileInfo(parser.value(paletteOption)).absoluteFilePath();
    }


    if (parser.isSet(sp1Option)) {
        opts.sp1 = QFileInfo(parser.value(sp1Option)).absoluteFilePath();
    }
    if (parser.isSet(sp2Option)) {
        opts.sp2 = QFileInfo(parser.value(sp2Option)).absoluteFilePath();
    }
    if (parser.isSet(sp3Option)) {
        opts.sp3 = QFileInfo(parser.value(sp3Option)).absoluteFilePath();
    }
    if (parser.isSet(sp4Option)) {
        opts.sp4 = QFileInfo(parser.value(sp4Option)).absoluteFilePath();
    }

    return opts;
}