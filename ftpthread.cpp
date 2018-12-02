#include "ftpthread.h"
#include <QTime>

ftpThread::ftpThread(double &f, QObject *parent) :
    QThread(parent),
    ftp(f)
{
    onOff = true;
}

void ftpThread::run()
{
    QTime
    while (onOff)
    {

    }
}

void ftpThread::finished()
{
    onOff = false;
}
