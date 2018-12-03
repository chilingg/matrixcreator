#ifndef FTPTHREAD_H
#define FTPTHREAD_H

#include <QThread>

class FTPThread : public QThread
{
    Q_OBJECT
public:
    explicit FTPThread(int &sum, int &ftp, QObject *parent = nullptr);
    void finished();

protected:
    void run();

private:
    int &sum;
    int &ftp;
    bool onOff;

signals:

public slots:
};

#endif // FTPTHREAD_H
