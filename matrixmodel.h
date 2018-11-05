#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"

class MatrixModel
{
public:
    MatrixModel(int seed = 0);
    int &getModel(int x, int y);
    void initModel();

private:
    int model[WORLDSIZE][WORLDSIZE];
};

#endif // MATRIXMODEL_H
