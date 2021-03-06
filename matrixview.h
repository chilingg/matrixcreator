#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include <QtConcurrent>
#include "matrixmodel.h"

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    MatrixView(MatrixModel *model, QWidget *parent = nullptr);
    ~MatrixView();

    bool isInView(int clickedX, int clickedY);//返回view坐标中对应的model坐标
    bool isInView(QPoint pos);

    QPoint getModelPoint(int clickedX, int clickedY);//点击坐标转换模型坐标
    QPoint getModelPoint(QPoint clickedPos);
    QPoint getUnitCentralPoint(int modelX, int modelY) const;//模型坐标转换为view中单元的中心点窗口坐标
    QPoint getUnitCentralPoint(QPoint modelPoint) const;
    QPoint getUnitPoint(QPoint modelPoint) const;//暂弃 模型坐标转换为view中单元的左上角坐标
    QRect getUnitRect(QPoint modelPoint) const;//获取模型在view中的坐标集

    QRect getSelectedModelRect() const;//获取选中的模型坐标集
    QRect getSelectedUnitRect() const;//获取选中的view单元坐标集

    QPoint getViewOffsetPoint() const;
    int getBaseUnitSize() const;

    void selectedUnits(QRect select);
    void moveView(int horizontal, int vertical);
    void zoomView(int clickedX, int clickedY, bool zoomView);//true缩小，false放大
    void referenceLineOnOff();
    void centerView();
    void notRedraw();//因历史遗留原因而增加的函数，若重写则在更改视图时直接把redraw改为true

    void FPSCount();
    void FPSDisplayOnOff();

    void takePicture();//获取选区或屏幕照片
    void startAnimation(){animationOnOff = true;}
    bool currentStatus(){ return animationOnOff; }

protected:
    void paintEvent(QPaintEvent *);
    void updateViewData();
    void drawBaseUnit(int x, int y, QRgb color);//按baseUnitSize绘制基础单元
    void drawBaseUnit(int x, int y, QRgb color, QImage &image);
    void drawReferenceLine(QPainter &painter);//绘制参考线
    void drawSelectBox(QPainter &painter);
    void drawFPSText(QPainter &painter);
    bool drawTakePicture(QPainter &painter);//绘制拍照动画

private:
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

    const int zoomList[7];
    const int ZOOMLEVEL = 7;

    QRect selectedUnitRect;
    bool referenceLine;
    bool centerOnOff;

    QImage image;
    bool redraw;
    unsigned char *ppix;
    int imageWidth;

    //fps计算
    double fps;
    int FrameSum;
    bool fpsOnOff;
    //fps计算2
    QElapsedTimer time;

    //动画
    bool animationOnOff = false;
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
    const QColor SELECT(0, 255, 255);//Cyan
}

#endif // MATRIXVIEW_H
