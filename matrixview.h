#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include "matrixmodel.h"
#include "mpoint.h"
#include <QMainWindow>
#include <QPainter>
#include <array>

using std::array;

class MatrixView : public QMainWindow
{
    Q_OBJECT

public:
    enum Zoom { ZoomIn, ZoomOut };
    explicit MatrixView(MatrixModel &m, QWidget *parent = nullptr);
    //~MatrixView();
    void moveToCoordinate(unsigned column, unsigned row);//移动指定坐标至左上角
    void selectUnits(QRect selectBox);//显示选框
    void translationView(unsigned top, unsigned bottom, unsigned left, unsigned right);//移动视图至指定坐标
    void zoomView(MPoint cdt, Zoom zoom);//缩放视图
    void referenceLineOnOff();//参考线开关
    void gridOnOff();//网格开关
    void fpsOnOff();//帧率显示开关
    void noRedrawUnits();//不重绘单元视图
    MPoint inView(QPoint clicktedPos) const;//查看点击坐标是否发生在视图中

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void updateViewSize();//更新视图数据
    void moveViewCheckup();//检查视图显示的单元是否正常
    void FPSCount();//FPS计算
    void takePicture();//获取选区或屏幕照片
    void drawBaseUnits();//绘制一个基础单元格
    void drawReferenceLine();//绘制参考线
    void drawSelectBox();//绘制选框
    void drawFPSText();//绘制FPS数据

    const MatrixModel &model;
    const unsigned &MODELSIZE;
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
    bool unitsDspl;	//绘制模型单元
    bool gridDspl; 	//绘制网格参考线
    bool rflDspl; 	//绘制网格参考线
    bool fpsDspl;		//fps显示

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

inline void MatrixView::selectUnits(QRect selectBox)
{
    selectedUnitRect = selectBox;
}

inline MPoint MatrixView::inView(QPoint clicktedPos) const
{
    MPoint cdt {false,0,0,0,0,clicktedPos};
    int viewX = clicktedPos.x() - viewOffsetX;
    int viewY = clicktedPos.y() - viewOffsetY;

    if(viewX >= 0 && viewY >= 0)
    {
        cdt.viewX = static_cast<unsigned>(viewX);
        cdt.viewY = static_cast<unsigned>(viewY);

        if(cdt.viewX < viewColumn * unitSize && cdt.viewY < viewRow * unitSize)
        {
            cdt.valid = true;
            cdt.modelColumn = cdt.viewX / unitSize + modelOffsetX;
            cdt.modelRow = cdt.viewY / unitSize + modelOffsetY;

            return cdt;
        }
    }

    return cdt;
}

inline void MatrixView::noRedrawUnits()
{
    unitsDspl = false;
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

inline void MatrixView::moveViewCheckup()
{
    //检查模型单元显示
    if(modelOffsetX + viewColumn > MODELSIZE)
        modelOffsetX = MODELSIZE - viewColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
    if(modelOffsetY + viewRow > MODELSIZE)
        modelOffsetY = MODELSIZE - viewRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行
}

inline void MatrixView::translationView(unsigned top, unsigned bottom, unsigned left, unsigned right)
{
    if(top != 0 && modelOffsetY != 0)
        modelOffsetY -= top;
    if(bottom != 0 && modelOffsetY <= MODELSIZE - viewRow)
        modelOffsetY += bottom;
    if(left != 0 && modelOffsetX != 0)
        modelOffsetX -= left;
    if(right != 0 && modelOffsetX <= MODELSIZE - viewColumn)
        modelOffsetX += right;

    moveViewCheckup();
}

inline void MatrixView::referenceLineOnOff()
{
    rflDspl = !rflDspl;
}

inline void MatrixView::gridOnOff()
{
    gridDspl = !gridDspl;
}

inline void MatrixView::fpsOnOff()
{
    fpsDspl = !fpsDspl;
    frameSum = 0;
}


#endif // MATRIXVIEW_H
