#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <cstddef>
#include <QMutex>
#include <vector>

#ifndef M_NO_DEBUG
#include <QDebug>
#endif

using MatrixSize = std::vector<int>::size_type;
using std::vector;

class MatrixModel
{
public:
    enum ModelPattern{ RedKernel, LifeGame };

    MatrixModel(MatrixSize widht, ModelPattern pattern);
    ~MatrixModel();

    int getUnitValue(MatrixSize x, MatrixSize y) const;		//获取单元值
    void (*updateModel)();									//更新数据
    void changeModelValue(MatrixSize x, MatrixSize y, int value);	//修改单元的值
    void clearUnit(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height);	//清空单元值
    void clearAllUnit();
    
private:
    vector<int> currentModel;
    ModelPattern modelPattern;
    MatrixSize modelSize;
    
    bool updateStatus;
    MatrixSize updateLine;
    QMutex lineMutex;
    QMutex changeMutex;
};

inline int MatrixModel::getUnitValue(MatrixSize x, MatrixSize y) const
{
#ifndef M_NO_DEBUG
    if(x > modelSize || y > modelSize)
        qDebug() << "log in " << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif
    return currentModel[x+y*modelSize];
}

inline void MatrixModel::changeModelValue(MatrixSize x, MatrixSize y, int value)
{
#ifndef M_NO_DEBUG
    if(x > modelSize || y > modelSize)
        qDebug() << "log in " << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif
    currentModel[x+y*modelSize] = value;
}

#endif // MATRIXMODEL_H
