#include "matrixmodel.h"

MatrixModel::MatrixModel(int seed)
{

}

int &MatrixModel::getModel(int x, int y)
{
    return (model[x][y]);

}

void MatrixModel::initModel()
{
    for(int i = 0; i < WORLDSIZE; ++i)
    {
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            if (j % 2 == 0)
                model[i][j] = 0;
            else
                model[i][j] = 1;
        }
    }

}
