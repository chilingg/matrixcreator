#include "matrixcontroller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MatrixController w;
    w.show();

    return a.exec();
}
