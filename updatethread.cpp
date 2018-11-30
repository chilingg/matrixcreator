#include "updatethread.h"

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

        if(line < 0)
            break;

        model->updateModelLine(line);
    }
}
