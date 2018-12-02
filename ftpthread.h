#ifndef FTPTHREAD_H
#define FTPTHREAD_H

#include <QThread>

class ftpThread : public QThread
{
    Q_OBJECT
public:
    explicit ftpThread(double &ftp, QObject *parent = nullptr);
    void finished();

protected:
    void run();

private:
    double &ftp;
    bool onOff;

signals:

public slots:
};

#endif // FTPTHREAD_H
