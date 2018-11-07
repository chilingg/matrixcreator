#include "matrixview.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

MatrixView::MatrixView(MatrixModel *model, QWidget *parent)
    :QWidget(parent),
      died(0),
      lived(1),
      dieColor(qRgb(0, 0, 0)),
      liveColer(qRgb(255, 255, 255)),
      model(model)
{
    //Set window backgroundcolor
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(51, 51, 51));
    setAutoFillBackground(true);
    setPalette(pal);

    baseUnitSize = 64;
    viewOffsetX = 0;
    viewOffsetY = 0;
    modelOffsetX = WORLDSIZE / 2;
    modelOffsetY = WORLDSIZE / 2;
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    //计算视图中的模型行列
    int modelColumn = 0;
    int modelRow = 0;
    while (modelColumn * baseUnitSize < width() - baseUnitSize)
        ++modelColumn;
    while (modelRow * baseUnitSize < height() - baseUnitSize)
        ++modelRow;

    if(modelOffsetX + modelColumn > WORLDSIZE)
        modelOffsetX = WORLDSIZE - modelColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
    if(modelOffsetX < 0)
    {
        modelOffsetX = 0;
        modelColumn = WORLDSIZE;
    }//检查模型列是否小于视图列，若是则更改视图大小

    if(modelOffsetY + modelRow > WORLDSIZE)
        modelOffsetY = WORLDSIZE - modelRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行
    if(modelOffsetY < 0)
    {
        modelOffsetY = 0;
        modelRow = WORLDSIZE;
    }//检查模型行是否小于视图行，若是则更改视图大小

    QImage image(modelColumn * baseUnitSize, modelRow * baseUnitSize, QImage::Format_RGB32);

    for(int i = 0; i < modelColumn; ++i)
    {
        for(int j = 0; j < modelRow; ++j)
        {
            QRgb color;

            //Get modeldata and select color
            int value = model->getModelValue(i, j, modelOffsetX, modelOffsetY);
            if(value == died)
                color = dieColor;
            else if(value == lived)
                color = liveColer;
            else
                color = qRgb(255, 0, 0);

            //qDebug() << i << j << value << "-->This is color()";
            drawBaseUnit(i * baseUnitSize, j * baseUnitSize, color, image);
        }
    }

    //添加坐标偏移，使模型居中于视图
    viewOffsetX = (width() - modelColumn * baseUnitSize) / 2;
    viewOffsetY = (height() - modelRow * baseUnitSize) / 2;
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

    painter.drawImage(0, 0, image);//绘制View
    //referenceLine(painter); //绘制参考线

    qDebug() << size() << "-->This is size()";
}

void MatrixView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        //获取偏移修正后的点击处对应的视图中的矩形坐标
        int x = event->pos().x() - viewOffsetX;
        int y = event->pos().y() - viewOffsetY;
        qDebug() << x / baseUnitSize << "-->pos().x";
        qDebug() << y / baseUnitSize << "-->pos().y";
    }
}

void MatrixView::drawBaseUnit(int x, int y, QRgb color, QImage &image)
{
    for(int i = x; i < x + baseUnitSize; ++i)
    {
        for(int j = y; j < y + baseUnitSize; ++j)
        {
            image.setPixel(i, j, color); //以一个个像素点绘制基础单元
        }
    }
}

void MatrixView::referenceLine(QPainter &painter)
{
    for(int i = 0; i < width(); i += baseUnitSize)
    {
        for(int j = 0; j < width(); j += baseUnitSize)
        {
            painter.setPen(Qt::gray);
            painter.drawLine(i, j, i, height());
            painter.drawLine(i, j, width(), j);
        }
    }
}
