#include "matrixview.h"
#include <QPainter>
//#include <QPalette>

MatrixView::MatrixView(MatrixModel &m, QWidget *parent) :
    QMainWindow(parent),
    model(m),
    modelSize(m.getModelSize()),
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

    //居中显示模型单元
    moveToCoordinate(modelSize/2, modelSize/2);

    fpsTime.start();
}

void MatrixView::resizeEvent(QResizeEvent *)
{
    updateViewSize();
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //添加坐标偏移，使视图居中于窗口
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

}

void MatrixView::updateViewSize()
{
    MatrixSize mWidth = static_cast<MatrixSize>(width());
    MatrixSize mHeight = static_cast<MatrixSize>(height());
    //计算视图显示的单元行列
    viewColumn = mWidth / unitSize;
    viewRow = mHeight / unitSize;
    //如果可显示单元大于模型最大单元
    if(viewColumn > modelSize)
        viewColumn = modelSize;
    if(viewRow > modelSize)
        viewRow = modelSize;

    //检查模型单元显示
    if(modelOffsetX + viewColumn > modelSize)
        modelOffsetX = modelSize - viewColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
    if(modelOffsetY + viewRow > modelSize)
        modelOffsetY = modelSize - viewRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行

    //计算视图偏移量
    viewOffsetX = (mWidth - viewColumn * modelSize) / 2;
    viewOffsetY = (mHeight - viewRow * modelSize) / 2;
}
