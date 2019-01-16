//添加一个模型更新方法时，写出它的实现（总接口、线程控制、更新方法），并在switchModel中添加的它的开始与结束需要的动作

#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <cstddef>
#include <QMutex>
#include <QtConcurrent>
#include <vector>
#include <set>
#include <utility>

#ifndef M_NO_DEBUG
#include <QDebug>
#endif

using std::set;
using std::pair;
using std::vector;
using MatrixSize = vector<int>::size_type;
using UnitPoint = pair<MatrixSize, MatrixSize>;

class MatrixModel
{
public:
    enum ModelPattern{ EmptyPattern, LifeGameT, LifeGame };

    MatrixModel(unsigned widht = 0, ModelPattern pattern = EmptyPattern);
    ~MatrixModel();

    int getUnitValue(MatrixSize x, MatrixSize y) const;//获取单元值
    unsigned getModelSize() const;
    ModelPattern getCurrentPattern() const;
    void (MatrixModel::*updateModel)();//指向当前模式的更新数据方法
    void changeModelValue(MatrixSize x, MatrixSize y, int value);//修改单元的值
    void clearUnit(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height);//清空单元值
    void clearAllUnit();
    ModelPattern switchModel(ModelPattern after);
    
private:
    //Thread
    void beginUpdate();//开启更新状态
    MatrixSize getUpdateLine();//获取一个未被其它线程获取的行号，全部行号获取完毕则结束更新状态
    MatrixSize getUpdateLine(MatrixSize interval, MatrixSize pos);
    bool currentStatus() const;//查询当前更新状态是否开启
    QMutex lineMutex;

    //LifeGame模型迁变Transfer
    //依据当前模型四周的状态计算下一状态，储存在临时模型中，结束后把该临时模型指定为当前模型
    void LFTransferModelThread();//启用Transfer线程
    void transferModelLine(MatrixSize line, MatrixSize row);//使用Transfer一次更新一行
    void startTransfer();//送进线程中的控制函数
    int getAroundValue(MatrixSize x, MatrixSize y);
    vector<vector<int> >tTempModel;

    //LifeGame模型演变Calculus
    //把当前模型对下次更新的影响记录下来，再依据记录修改当前模型
    void LFCalculusModelThread();//启用Calculus线程
    void changLineAroundValue(MatrixSize line);//记录当前行对四周的影响
    void startCalculus();//控制记录线程
    QMutex changeMutex;
    QWaitCondition synchroThread;
    vector<vector<int> >cTempModel;

    unsigned THREADS;
    vector<QFuture<void> > future;
    vector<vector<int> >currentModel;
    ModelPattern modelPattern;
    unsigned modelSize;

    //追踪unit
    void tracedUnit(MatrixSize column, MatrixSize row);//开启追踪时才会记录坐标
    UnitPoint popTracedUnit();
    set<UnitPoint> traceUnit;
    set<UnitPoint> traceUnit2;
    bool traceOnOff;
    
    bool updateStatus;

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

inline MatrixModel::ModelPattern MatrixModel::getCurrentPattern() const
{
    return modelPattern;
}

inline void MatrixModel::changeModelValue(MatrixSize x, MatrixSize y, int value)
{
#ifndef M_NO_DEBUG
    if(x > modelSize || y > modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif

    currentModel[x][y] = value;
    tracedUnit(x, y);
}

inline void MatrixModel::beginUpdate()
{
    updateStatus = true;
}

inline bool MatrixModel::currentStatus() const
{
    return updateStatus;
}

inline void MatrixModel::tracedUnit(MatrixSize column, MatrixSize row)
{
    if(traceOnOff)
        traceUnit.insert({column, row});
}

#endif // MATRIXMODEL_H
