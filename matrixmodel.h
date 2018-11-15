#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"

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

protected:
    int getAroundValue(int x, int y);

private:
    int(* currentModel)[WORLDSIZE];
    int(* tempModel)[WORLDSIZE];
};

#endif // MATRIXMODEL_H
