#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include "matrixmodel.h"
#include <QMainWindow>
#include <QPainter>
#include <array>

using std::array;

class MatrixView : public QMainWindow
{
    Q_OBJECT

public:
    explicit MatrixView(MatrixModel &m, QWidget *parent = nullptr);
    //~MatrixView();
    void moveToCoordinate(unsigned column, unsigned row);//视图中显示的模型坐标是以模型中心为原点的坐标系
    bool InView(QPoint clicktedPos) const;//查看点击坐标是否发生在视图中

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void updateViewSize();
    void moveViewCheckup();
    void noRedrawUnit();
    void drawBaseUnit();
    void drawReferenceLine();
    bool drawTakePicture();
    void drawSelectBox();
    void drawFPSText();
    void FPSCount();

    const MatrixModel &model;
    const unsigned &modelSize;
    //绘制的矩阵与客户区坐标偏移量
    int viewOffsetX;
    int viewOffsetY;
    //视图与模型坐标偏移量
    unsigned modelOffsetX;
    unsigned modelOffsetY;
    //单元在视图中的行列
    unsigned viewColumn;
    unsigned viewRow;
    //缩放级别组
    array<const unsigned, 7> zoomList;
    array<const QColor, 4> lineColor;
    unsigned unitSize;	//基础单位大小（px）

    //一些开关
    bool unitsOnOff;	//绘制模型单元
    bool gridOnOff; 	//绘制网格参考线
    bool referencelineOnOff; 	//绘制网格参考线
    bool fpsOnOff;		//fps显示
    bool animationOnOff;//动画显示

    QRect selectedUnitRect;	//选框
    QImage unitImage;			//模型单元图像
    unsigned char *ppix;	//图像像素首指针
    unsigned imageWidth;	//图像宽度

    //fps计算
    double fpsCount;
    unsigned frameSum;
    QTime fpsTime;

    QPainter painter;

signals:

public slots:
};

namespace MatrixColor
{
    const QColor LUMINOSITY_0_0(0, 0, 0);	//Black

    //定义了一组五等三分的明度
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
    const QColor LUMINOSITY_5_255(255, 255, 255);	//White

    const QColor WARNING(255, 0, 0);	//Red
    const QColor SELECT(0, 255, 255);	//Cyan
}

inline void MatrixView::moveToCoordinate(unsigned column, unsigned row)
{
    modelOffsetX = column - viewColumn/2;
    modelOffsetY = row - viewRow/2;

    moveViewCheckup();
}

inline bool MatrixView::InView(QPoint clicktedPos) const
{
    if(clicktedPos.x() < viewOffsetX || clicktedPos.y() < viewOffsetY)
        return false;

    unsigned x = clicktedPos.x() - viewOffsetX;
    unsigned y = clicktedPos.y() - viewOffsetY;

    if(x < viewColumn * unitSize && y < viewRow * unitSize)
        return true;
    else
        return false;
}

inline void MatrixView::noRedrawUnit()
{
    unitsOnOff = false;
}

inline void MatrixView::drawSelectBox()
{
    painter.setPen(MatrixColor::SELECT);
    painter.drawRect(selectedUnitRect);
}

inline void MatrixView::FPSCount()
{
    static int interval = fpsTime.elapsed();

    //每N帧计算一次帧率
    if(frameSum == 8)
    {
        //qDebug() << "fps: " << (time.elapsed() - interval);
        fpsCount = 1000.0 / (fpsTime.elapsed() - interval) * 8.0;
        interval = fpsTime.elapsed();
        frameSum = 0;
    }
}


#endif // MATRIXVIEW_H
