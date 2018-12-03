#include "ftpthread.h"
#include <QTime>
#include <QDebug>

FTPThread::FTPThread(int &sum, int &ftp, QObject *parent) :
    QThread(parent),
    sum(sum),
    ftp(ftp)
{
    onOff = true;
}

void FTPThread::run()
{
    QTime timer;
    timer.start();
    int before = 0;
    int now = 0;

    while (onOff)
    {
        now = timer.elapsed() / 1000;
        if(now != before)
        {
            //qDebug() << "In ftp thread." << now << sum;
            before = now;
            ftp = sum;
            sum = 0;
        }
    }
}

void FTPThread::finished()
{
    onOff = false;
}
