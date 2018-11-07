#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"

class MatrixModel
{
public:
    MatrixModel(int seed = 0);
    int getModelValue(int x, int y); //依据坐标返回单个数据
    void updateModel(); //更新数据
    void changeModelValue(int x, int y, int value); //修改模型的值

private:
    int model[WORLDSIZE][WORLDSIZE];
};

#endif // MATRIXMODEL_H
