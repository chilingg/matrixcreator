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
    void changeModelValue(size_t x, size_t y); //修改模型的值
    void clearModel(int x, int y, int widht, int height);
    void clearAllModel();
    auto getModel() -> const int(*)[WORLDSIZE];//用于查询对应像素

    //Thread
    void beginUpdate();//开启更新状态
    size_t getUpdateLine();//获取一个未被其它线程获取的行号，全部行号获取完毕则结束更新状态
    bool updateStatus() const;//查询当前更新状态是否开启

    //模型迁变Transfer
    //依据当前模型四周的状态计算下一状态，储存在临时模型中，结束后把该临时模型指定为当前模型
    void transferModelThread();//启用Transfer线程
    void transferModelLine(size_t line);//使用Transfer一次更新一行
    void startTransfer();//送进线程中的控制函数

    //模型演变Calculus
    //把当前模型对下次更新的影响记录下来，再依据记录修改当前模型
    void calculusModelThread();//启用Calculus线程
    void changLineAroundValue(size_t line);//记录当前行对四周的影响
    void calculusModelLine(size_t line);//依据记录修改当前行
    void startCalculus1();//控制记录线程
    void startCalculus2();//控制修改线程
    //可以把受影响的单元记录在链表中，随后依据链表进行修改

    //模型各数据不断取反，用于测试
    void testModelThread();
    void testModelLine(size_t line);
    void startTest();

protected:
    int getAroundValue(size_t x, size_t y);

private:
    int(* currentModel)[WORLDSIZE];
    int(* tempModel)[WORLDSIZE];

    //Thread
    bool currentStatus;
    size_t updateLine;
    QMutex lineMutex;
    QMutex changeMutex;

    //测试用，记录线程中函数是否运行足够的次数
    int debug = 0;
    int debugValue = 0;
};

#endif // MATRIXMODEL_H
