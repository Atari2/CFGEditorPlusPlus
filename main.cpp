#include "cfgeditor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CFGEditor w;
    w.show();
    return a.exec();
}
