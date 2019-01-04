//添加一个模型更新方法时，写出它的实现（总接口、线程控制、更新方法），并在switchModel中添加的它的开始与结束需要的动作

#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <cstddef>
#include <QMutex>
#include <QtConcurrent>
#include <vector>

#ifndef M_NO_DEBUG
#include <QDebug>
#endif

using MatrixSize = std::vector<int>::size_type;
using std::vector;

class MatrixModel
{
public:
    enum ModelPattern{ EmptyPattern, LifeGameT, LifeGame };

    MatrixModel(unsigned widht = 0, ModelPattern pattern = EmptyPattern);
    ~MatrixModel();

    int getUnitValue(MatrixSize x, MatrixSize y) const;		//获取单元值
    unsigned getModelSize() const;
    void (MatrixModel::*updateModel)();									//更新数据
    void changeModelValue(MatrixSize x, MatrixSize y, int value);	//修改单元的值
    void clearUnit(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height);	//清空单元值
    void clearAllUnit();
    ModelPattern switchModel(ModelPattern after);
    
private:
    //Thread
    void beginUpdate();//开启更新状态
    MatrixSize getUpdateLine();//获取一个未被其它线程获取的行号，全部行号获取完毕则结束更新状态
    bool currentStatus() const;//查询当前更新状态是否开启

    //LifeGame模型迁变Transfer
    //依据当前模型四周的状态计算下一状态，储存在临时模型中，结束后把该临时模型指定为当前模型
    void LFTransferModelThread();//启用Transfer线程
    void transferModelLine(MatrixSize line);//使用Transfer一次更新一行
    void startTransfer();//送进线程中的控制函数
    int getAroundValue(MatrixSize x, MatrixSize y);
    int **tempModel;

    //LifeGame模型演变Calculus
    //把当前模型对下次更新的影响记录下来，再依据记录修改当前模型
    void LFCalculusModelThread();//启用Calculus线程
    void changLineAroundValue(MatrixSize line);//记录当前行对四周的影响
    void calculusModelLine(MatrixSize line);//依据记录修改当前行
    void startCalculus1();//控制记录线程
    void startCalculus2();//控制修改线程

    unsigned THREADS;
    vector<QFuture<void> > future;
    //vector<int> currentModel;
    int **currentModel;
    ModelPattern modelPattern;
    unsigned modelSize;
    
    bool updateStatus;
    MatrixSize updateLine;
    QMutex lineMutex;
    QMutex changeMutex;

#ifndef M_NO_DEBUG
    //测试用，记录线程中函数是否运行足够的次数
    MatrixSize debug = 0;
    MatrixSize debugValue = 0;
#endif
};

inline int MatrixModel::getUnitValue(MatrixSize x, MatrixSize y) const
{
#ifndef M_NO_DEBUG
    if(x > modelSize || y > modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!" ;
#endif

    return currentModel[x][y];
}

inline unsigned MatrixModel::getModelSize() const
{
    return modelSize;
}

inline void MatrixModel::changeModelValue(MatrixSize x, MatrixSize y, int value)
{
#ifndef M_NO_DEBUG
    if(x > modelSize || y > modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif

    currentModel[x][y] = value;
}

inline void MatrixModel::beginUpdate()
{
    updateStatus = true;
}

inline bool MatrixModel::currentStatus() const
{
    return updateStatus;
}

#endif // MATRIXMODEL_H
