#include "updatethread.h"
#include <QDebug>

UpdateThread::UpdateThread(MatrixModel *m, QObject *parent) :
    QThread(parent),
    model(m)
{

}

void UpdateThread::run()
{
    while (1)
    {
        int line = model->getUpdateLine();
        //qDebug() << "In thread run..." << line;

        if(line < 0)
            break;
        if(line >= WORLDSIZE)
            qDebug() << "Overflow line" << line;

        model->updateModelLine(line);
    }
}
