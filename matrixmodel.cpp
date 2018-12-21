#ifndef M_NO_DEBUG
#include <QDebug>
#endif

#include "matrixmodel.h"

MatrixModel::MatrixModel(MatrixSize x, MatrixSize y)
{
    currentModelP = new vector<int>(x*y, 0);
}

MatrixModel::~MatrixModel()
{
    currentModelP->clear();
}
