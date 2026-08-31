#include "cfgeditor.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CFGEditorCommandLineOptions options = CFGEditor::parseCommandLineOptions(a);
    CFGEditor w{};
    w.show();
    w.applyCommandLineOptions(options);
    return a.exec();
}