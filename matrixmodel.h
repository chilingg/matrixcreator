#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

class MatrixModel
{
public:
    MatrixModel();
    ~MatrixModel();

    virtual int getUnitValue(unsigned x, unsigned y) = 0;		//获取单元值
    virtual void updateModel() = 0;								//更新数据
    virtual void changeModelValue(unsigned x, unsigned y) = 0;	//修改单元的值
    virtual void clearModel(unsigned x, unsigned y, unsigned widht, unsigned height) = 0;	//清空单元值
};

#endif // MATRIXMODEL_H
