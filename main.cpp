#include "cfgeditor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList list;
    // skip the path of the executable
    for (auto i = 1; i < argc; ++i)
        list.append(argv[i]);
    CFGEditor w{list};
    w.show();
    return a.exec();
}
