#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include "matrixmodel.h"

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    MatrixView(MatrixModel *model, QWidget *parent = 0);
    int getViewOffsetX() const;
    int getViewOffsetY() const;
    int getModelColumn() const;
    int getModelRow() const;
    int getBaseUnitSize() const;
    bool pointViewToModel(int &x, int &y);//返回view坐标中对应的model坐标

protected:
    void paintEvent(QPaintEvent *);

private:
    void drawBaseUnit(int x, int y, QRgb color, QImage &image);//按baseUnitSize绘制基础单元
    void referenceLine(QPainter &painter);//绘制参考线

    int baseUnitSize; //基础单位的大小

    //基本状态
    const int died;
    const int lived;
    //基本状态对应的颜色
    const QRgb dieColor;
    const QRgb liveColer;

    MatrixModel *model;

    //绘制的矩阵与客户区坐标偏移量
    int viewOffsetX;
    int viewOffsetY;

    //视图与模型坐标偏移量
    int modelOffsetX;
    int modelOffsetY;

    int modelColumn;
    int modelRow;
};

#endif // MATRIXVIEW_H
