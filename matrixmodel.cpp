#include "matrixmodel.h"

MatrixModel::MatrixModel(MatrixSize size, ModelPattern pattern):
    currentModel(size*size, 0),
    modelPattern(pattern),
    modelSize(size),
    updateStatus(false),
    updateLine(0)
{
}

MatrixModel::~MatrixModel()
{
    currentModel.clear();
}

void MatrixModel::clearUnit(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height)
{
#ifndef M_NO_DEBUG
    if(x > modelSize || y > modelSize)
        qDebug() << "log in " << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif
    for(MatrixSize i = 0; i < widht; ++i)
    {
        for(MatrixSize j = 0; j < height; ++j)
        {
            currentModel[x+i + (y+j)*modelSize] = 0;
        }
    }
}

void MatrixModel::clearAllUnit()
{
    clearUnit(0, 0, modelSize, modelSize);
}
