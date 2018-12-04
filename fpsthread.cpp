#include "fpsthread.h"
#include <QTime>
#include <QDebug>

FPSThread::FPSThread(int &sum, int &fps, QObject *parent) :
    QThread(parent),
    sum(sum),
    fps(fps)
{
    onOff = true;
}

void FPSThread::run()
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
            //qDebug() << "In fps thread." << now << sum;
            before = now;
            fps = sum;
            sum = 0;
        }
    }
}

void FPSThread::finished()
{
    onOff = false;
}
