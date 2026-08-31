#include "cfgeditor.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto options = CFGEditor::parseCommandLineOptions(a);
    if (!options.has_value()) {
        return EXIT_FAILURE;
    }
    CFGEditor w{};
    w.show();
    w.applyCommandLineOptions(*options);
    return a.exec();
}