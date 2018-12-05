#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

//typedef const int (* PModle)[WORLDSIZE];

//MatrixModel的附属类，使用多线程更新模型
class UpdateThread;

class MatrixModel
{
public:
    MatrixModel();
    ~MatrixModel();
    int getModelValue(int x, int y); //依据坐标返回单个数据
    void updateModel(); //更新数据
    void changeModelValue(int x, int y); //修改模型的值
    void clearModel(int x, int y, int widht, int height);
    void clearAllModel();
    const int (*getModel())[WORLDSIZE];

    //Thread
    void updateModelThread();
    bool status() const;
    void beginUpdate();
    int getUpdateLine();
    void updateModelLine(int line);

protected:
    int getAroundValue(int x, int y);

private:
    int(* currentModel)[WORLDSIZE];
    int(* tempModel)[WORLDSIZE];

    //Thread
    bool currentStatus;
    int updateLine;
    QMutex mutex;
    UpdateThread *thread[THREADS];
};

//MatrixModel的附属类，使用多线程更新模型
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

#endif // MATRIXMODEL_H
