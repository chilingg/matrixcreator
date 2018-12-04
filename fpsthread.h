#ifndef FPSTHREAD_H
#define FPSTHREAD_H

#include <QThread>

class FPSThread : public QThread
{
    Q_OBJECT
public:
    explicit FPSThread(int &sum, int &fps, QObject *parent = nullptr);
    void finished();

protected:
    void run();

private:
    int &sum;
    int &fps;
    bool onOff;

signals:

public slots:
};

#endif // FPSTHREAD_H
