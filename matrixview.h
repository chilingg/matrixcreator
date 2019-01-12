#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include "matrixmodel.h"
#include "mpoint.h"
#include <QWidget>
#include <QPainter>
#include <array>

#ifndef M_NO_DEBUG
#include <QDebug>
#endif

using std::array;

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    enum Zoom { ZoomIn, ZoomOut };
    explicit MatrixView(MatrixModel &m, QWidget *parent = nullptr);
    //~MatrixView();
    void moveToCoordinate(int column, int row);//移动指定坐标至左上角
    void moveToCoordinate();//移动指定坐标模型中间
    void selectUnits(QRect selectBox);//显示选框
    void translationView(int horizontal, int vertical);//移动视图至指定坐标
    void zoomView(MPoint cdt, Zoom zoom);//缩放视图
    void referenceLineOnOff();//参考线开关
    void gridOnOff();//网格开关
    void fpsOnOff();//帧率显示开关
    void overRangeLineOff();//取消越界线显示
    void noRedrawUnits();//不重绘单元视图
    void takePicture(QString path);//获取选区或屏幕照片
    QPoint getViewOffsetPoint() const;
    QRect getSelectViewRect() const;
    QRect getSelectUnitRect() const;
    MPoint inView(QPoint clicktedPos) const;//查看点击坐标是否发生在视图中
    void switchColorPattern(MatrixModel::ModelPattern pattern);

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void updateViewSize();//更新视图数据
    void moveViewCheckup();//检查视图显示的单元是否正常
    void moveViewCheckupDisplay();//若视图越界则修正并显示
    void FPSCount();//FPS计算
    void drawBaseUnits();//绘制所有基础单元格
    void drawBaseUnits(int left, int top, int mWidth, int mHeight, QImage &picture);
    void drawReferenceLine(QPainter &painter);//绘制参考线
    void drawSelectBox(QPainter &painter);//绘制选框
    void drawFPSText(QPainter &painter);//绘制FPS数据
    void drawOverRangeLine(QPainter &painter);//绘制越界提示线
    void viewOverRange(bool top, bool bottom, bool left, bool right);

    QRgb (*valueToColor)(int);
    static QRgb tValueToColor(int value);
    static QRgb cValueToColor(int value);

    const MatrixModel &model;
    const int MODELSIZE;
    //绘制的矩阵与客户区坐标偏移量
    int viewOffsetX;
    int viewOffsetY;
    //视图与模型坐标偏移量
    int modelOffsetX;
    int modelOffsetY;
    //单元在视图中的行列
    int viewColumn;
    int viewRow;
    //缩放级别组
    array<const int, 7> zoomList;
    array<const QColor, 4> lineColor;
    int unitSize;	//基础单位大小（px）

    //一些开关
    bool unitsDspl;	//绘制模型单元
    bool gridDspl; 	//绘制网格参考线
    bool rflDspl; 	//绘制网格参考线
    bool fpsDspl;	//fps显示
    array<bool, 5> overRange;//移动视图越界提示

    QRect selectedViewRect;	//选框
    QImage unitImage;			//模型单元图像

    //fps计算
    double fpsCount;
    unsigned frameSum;
    QTime fpsTime;

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

inline void MatrixView::moveToCoordinate(int column, int row)
{
    modelOffsetX = column;
    modelOffsetY = row;

    moveViewCheckup();
}

inline void MatrixView::moveToCoordinate()
{
    moveToCoordinate(MODELSIZE/2, MODELSIZE/2);
}

inline void MatrixView::selectUnits(QRect selectBox)
{
    selectedViewRect = selectBox;
}

inline void MatrixView::noRedrawUnits()
{
    unitsDspl = false;
}

inline QPoint MatrixView::getViewOffsetPoint() const
{
    return QPoint(viewOffsetX, viewOffsetY);
}

inline QRect MatrixView::getSelectViewRect() const
{
    return selectedViewRect;
}

inline QRect MatrixView::getSelectUnitRect() const
{
    return QRect(selectedViewRect.topLeft()/unitSize + QPoint(modelOffsetX,modelOffsetY),
                 selectedViewRect.size()/unitSize);
}

inline void MatrixView::drawSelectBox(QPainter &painter)
{
    painter.setPen(MatrixColor::SELECT);
    painter.drawRect(selectedViewRect);
}

inline void MatrixView::viewOverRange(bool top, bool bottom, bool left, bool right)
{
    overRange[0] = true;//是否越界
    overRange[1] = top;
    overRange[2] = bottom;
    overRange[3] = left;
    overRange[4] = right;
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
    {
        modelOffsetX = MODELSIZE - viewColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
    }
    else if(modelOffsetX < 0)
    {
        modelOffsetX = 0;
    }

    if(modelOffsetY + viewRow > MODELSIZE)
    {
        modelOffsetY = MODELSIZE - viewRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行
    }
    else if(modelOffsetY < 0)
    {
        modelOffsetY = 0;
    }
}

inline void MatrixView::translationView(int horizontal, int vertical)
{
    if(horizontal)
    {
        modelOffsetX += horizontal;
    }
    if(vertical)
    {
        modelOffsetY += vertical;
    }

    moveViewCheckupDisplay();
}

inline void MatrixView::referenceLineOnOff()
{
    rflDspl = !rflDspl;
}

inline void MatrixView::gridOnOff()
{
    gridDspl = !gridDspl;

    //显示网格需要重绘单元图像
    if(gridDspl)
    {
        unitImage = QImage(viewColumn * unitSize, viewRow * unitSize, QImage::Format_RGB32);
        unitImage.fill(MatrixColor::LUMINOSITY_1_17);
    }
}

inline void MatrixView::fpsOnOff()
{
    fpsDspl = !fpsDspl;
    frameSum = 0;
}

inline void MatrixView::overRangeLineOff()
{
    overRange[0] = false;
}

inline void MatrixView::drawBaseUnits()
{
    drawBaseUnits(0, 0, viewColumn, viewRow, unitImage);
}

#endif // MATRIXVIEW_H
