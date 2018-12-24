#include "matrixview.h"
//#include <QPalette>

MatrixView::MatrixView(MatrixModel &m, QWidget *parent) :
    QMainWindow(parent),
    model(m),
    viewOffsetX(0),
    viewOffsetY(0),
    modelOffsetX(0),
    modelOffsetY(0),
    viewColumn(0),
    viewRow(0),
    zoomList{ 1,2,4,8,16,32,64 },
    unitSize(zoomList[3]),
    unitsOnOff(true),
    gridOnOff(true),
    fpsOnOff(true),
    moveOnOff(),
    animationOnOff(false),
    selectedUnitRect(),
    image(),
    ppix(nullptr),
    imageWidth(0),
    fpsCount(0),
    frameSum(0)
{
    //设置窗口背景色
    QPalette pal(palette());
    pal.setColor(QPalette::Background, MatrixColor::LUMINOSITY_1_51);
    setAutoFillBackground(true);
    setPalette(pal);

    fpsTime.start();
}
