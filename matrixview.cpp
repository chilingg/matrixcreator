#include "matrixview.h"

#include <QDebug>
#include <QPainter>

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

    baseUnitSize = 12;
    viewOffsetX = 0;
    viewOffsetY = 0;
    modelOffsetX = WORLDSIZE / 2;
    modelOffsetY = WORLDSIZE / 2;
    modelColumn = 0;
    modelRow = 0;
}

int MatrixView::getModelColumn() const
{
    return modelColumn;
}

int MatrixView::getModelRow() const
{
    return modelRow;
}

int MatrixView::getBaseUnitSize() const
{
    return baseUnitSize;
}

bool MatrixView::pointViewToModel(int &x, int &y)
{
    //判断点击是否发生在view范围内
    if (x > 0 && x < modelColumn * baseUnitSize)
    {
        if (y > 0 && y < modelColumn * baseUnitSize)
        {
            x = x / baseUnitSize + modelOffsetX;
            y = y / baseUnitSize + modelOffsetY;

            return true;
        }
    }

    qDebug() << "Point over!";
    qDebug() << x / baseUnitSize << x << "-->pos().x";
    qDebug() << y / baseUnitSize << y << "-->pos().y";
    return false;
}

int MatrixView::getViewOffsetX() const
{
    return viewOffsetX;
}

int MatrixView::getViewOffsetY() const
{
    return viewOffsetY;
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    //计算视图中的模型行列
    modelColumn = 0;
    modelRow = 0;
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
            int value = model->getModelValue(i + modelOffsetX, j + modelOffsetY);
            if(value == died)
                color = dieColor;
            else if(value == lived)
                color = liveColer;
            else
                color = qRgb(255, 0, 0);

            //qDebug() << i << j << value << "-->This is value()";
            drawBaseUnit(i * baseUnitSize, j * baseUnitSize, color, image);
        }
    }

    //添加坐标偏移，使模型居中于视图
    viewOffsetX = (width() - modelColumn * baseUnitSize) / 2;
    viewOffsetY = (height() - modelRow * baseUnitSize) / 2;
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

    painter.drawImage(0, 0, image);//绘制View
    referenceLine(painter); //绘制参考线

    //qDebug() << size() << "-->This is size()";
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
    for(int i = baseUnitSize; i < modelColumn * baseUnitSize; i += baseUnitSize)
    {
        if(i / baseUnitSize % 10 != 0)
            painter.setPen(QColor(17, 17, 17));
        else
            painter.setPen(QColor(34, 34, 34));

        painter.drawLine(i, 0, i, modelRow * baseUnitSize);//绘制列
    }

    for(int j = baseUnitSize; j < modelRow * baseUnitSize; j += baseUnitSize)
    {
        if(j / baseUnitSize % 10 != 0)
            painter.setPen(QColor(17, 17, 17));
        else
            painter.setPen(QColor(34, 34, 34));

        painter.drawLine(0, j, modelColumn * baseUnitSize, j);//绘制行
        //qDebug() << i << j - baseUnitSize << modelColumn * baseUnitSize;
    }
}
