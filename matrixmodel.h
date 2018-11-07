#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"

class MatrixModel
{
public:
    MatrixModel(int seed = 0);
    int getModelValue(int x, int y, int offsetX, int offsetY); //依据坐标返回单个数据
    void updateModel(); //更新数据

private:
    int model[WORLDSIZE][WORLDSIZE];
};

#endif // MATRIXMODEL_H
