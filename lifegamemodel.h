#ifndef LIFEGAMEMODEL_H
#define LIFEGAMEMODEL_H

#include "matrixmodel.h"
#include <cstddef>
#include <QMutex>
#include <array>

using std::array;

class LifeGameModel : public MatrixModel
{
public:
    LifeGameModel(size_t x, size_t y);
    virtual bool getUnitValue(size_t width, size_t height);		//获取单元值
    virtual void updateModel();							//更新数据
    virtual void changeModelValue(size_t x, size_t y);	//修改单元的值
    virtual void clearModel(size_t x, size_t y, size_t widht, size_t height);	//清空单元值

private:
    bool getAroundValue(size_t x, size_t y);

    array::iterator *currentModelP;
    array::iterator *endModelP;
    size_t modelWidth;
    size_t modelSize;

    //Thread update
    bool currentStatus;
    size_t updateLine;
    QMutex updateMutex;
};

inline bool LifeGameModel::getUnitValue(size_t x, size_t y)
{
    return *(currentModelP + (x + y*modelWidth));
}

void LifeGameModel::clearModel(size_t x, size_t y, size_t widht, size_t height)
{
    for(array::iterator *begin = currentModelP; begin != endModelP; ++begin)
    {
        *begin = 0;
        //重写，需要一个记录有值单元位置的容器
    }
}

#endif // LIFEGAMEMODEL_H
