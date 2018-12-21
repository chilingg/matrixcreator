#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <cstddef>
#include <QMutex>
#include <vector>

using MatrixSize = std::vector<int>::size_type;
using std::vector;

class MatrixModel
{
public:
    MatrixModel(MatrixSize x, MatrixSize y);
    ~MatrixModel();

    int getUnitValue(MatrixSize x, MatrixSize y);		//获取单元值
    void updateModel();									//更新数据
    void changeModelValue(MatrixSize x, MatrixSize y);	//修改单元的值
    void clearModel(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height);	//清空单元值
    
private:
    vector<int> *currentModelP;
    
    bool updateStatus;
    MatrixSize updateLine;
    QMutex lineMutex;
    QMutex changeMutex;
};

#endif // MATRIXMODEL_H
