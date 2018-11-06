#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include "matrixmodel.h"

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    MatrixView(MatrixModel *model, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

private:
    void drawBaseUnit(int x, int y, QRgb color, QImage &image);
    int baseUnitSize; //基础单位的大小
    //基本状态
    const int died;
    const int lived;
    const QRgb dieColor;
    const QRgb liveColer;

    MatrixModel *model;

    void referenceLine(QPainter &painter);
};

#endif // MATRIXVIEW_H
