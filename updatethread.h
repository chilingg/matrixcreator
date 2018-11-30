#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H

#include <QThread>
#include "matrixmodel.h"

class UpdateThread : public QThread
{
    Q_OBJECT
public:
    explicit UpdateThread(MatrixModel *m, QObject *parent = nullptr);

protected:
    void run();
private:
    MatrixModel *model;

signals:

public slots:
};

#endif // UPDATETHREAD_H
