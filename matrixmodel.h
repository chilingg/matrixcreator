#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"
#include <QMutex>
#include <QMutexLocker>

//typedef const int (* PModle)[WORLDSIZE];

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
    void beginUpdate();
    int getUpdateLine();
    bool updateStatus() const;

    //模型迁变Transfer
    void transferModelThread();
    void transferModelLine(size_t line);
    void startTransfer();

    //模型演变Calculus
    void calculusModelThread();
    void changLineAroundValue(size_t line);
    void calculusModelLine(size_t line);
    void startCalculus1();
    void startCalculus2();

protected:
    int getAroundValue(int x, int y);

private:
    int(* currentModel)[WORLDSIZE];
    int(* tempModel)[WORLDSIZE];

    //Thread
    bool currentStatus;
    int updateLine;
    QMutex lineMutex;
    QMutex changeMutex;
    int debug = 0;
};

#endif // MATRIXMODEL_H
