#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include "matrixmodel.h"

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    MatrixView(MatrixModel *model, QWidget *parent = 0);
    bool toModelPoint(int &clickedX, int &y);//返回view坐标中对应的model坐标
    void zoomView(int x, int y, bool zoomView);

protected:
    void paintEvent(QPaintEvent *);

private:
    void drawBaseUnit(int x, int y, QRgb color, QImage &image);//按baseUnitSize绘制基础单元
    void referenceLine(QPainter &painter);//绘制参考线

    MatrixModel *model;
    int baseUnitSize; //基础单位的大小

    //基本状态
    const int died;
    const int lived;
    //基本状态对应的颜色
    const QRgb dieColor;
    const QRgb liveColer;


    //绘制的矩阵与客户区坐标偏移量
    int viewOffsetX;
    int viewOffsetY;

    //视图与模型坐标偏移量
    int modelOffsetX;
    int modelOffsetY;

    int viewColumn;
    int viewRow;

    const int zoomList[9];
};

namespace VIEW
{
    //定义了一组五分三等的明度
    const QColor LUMINOSITY_0_0(0, 0, 0);//Black

    const QColor LUMINOSITY_1_17(17, 17, 17);
    const QColor LUMINOSITY_1_34(34, 34, 34);
    const QColor LUMINOSITY_1_51(51, 51, 51);

    const QColor LUMINOSITY_2_68(68, 68, 68);
    const QColor LUMINOSITY_2_85(85, 85, 85);
    const QColor LUMINOSITY_2_102(102, 102, 102);

    const QColor LUMINOSITY_3_119(119, 119, 119);
    const QColor LUMINOSITY_3_136(136, 136, 136);
    const QColor LUMINOSITY_3_153(153, 153, 153);

    const QColor LUMINOSITY_4_170(170, 170, 170);
    const QColor LUMINOSITY_4_187(187, 187, 187);
    const QColor LUMINOSITY_4_204(204, 204, 204);

    const QColor LUMINOSITY_5_221(221, 221, 221);
    const QColor LUMINOSITY_5_238(238, 238, 238);
    const QColor LUMINOSITY_5_255(255, 255, 255);//White

    const QColor WARNING(255, 0, 0);//Red
}

#endif // MATRIXVIEW_H
