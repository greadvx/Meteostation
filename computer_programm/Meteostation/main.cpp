#include "desk.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Desk w;
    w.show();

    return a.exec();
}
