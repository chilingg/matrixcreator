#include "matrixview.h"

#include <QDebug>
#include <QPainter>

MatrixView::MatrixView(MatrixModel *model, QWidget *parent)
    :QWidget(parent),
      model(model),
      died(0),
      lived(1),
      dieColor(VIEW::LUMINOSITY_0_0.rgb()),
      liveColer(VIEW::LUMINOSITY_5_255.rgb())
{
    //Set window backgroundcolor
    QPalette pal = palette();
    pal.setColor(QPalette::Background, VIEW::LUMINOSITY_1_51);
    setAutoFillBackground(true);
    setPalette(pal);

    baseUnitSize = 50;//默认的一个单元大小

    viewOffsetX = 0;
    viewOffsetY = 0;
    modelOffsetX = WORLDSIZE / 2;
    modelOffsetY = WORLDSIZE / 2;
    viewColumn = 0;
    viewRow = 0;

}

int MatrixView::getModelColumn() const
{
    return viewColumn;
}

int MatrixView::getModelRow() const
{
    return viewRow;
}

int MatrixView::getBaseUnitSize() const
{
    return baseUnitSize;
}

bool MatrixView::pointViewToModel(int &x, int &y)
{
    //判断点击是否发生在view范围内
    if (x > 0 && x < viewColumn * baseUnitSize)
    {
        if (y > 0 && y < viewColumn * baseUnitSize)
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

void MatrixView::zoomView(bool zoom)
{
    if(zoom)
    {
        //放大
        if(baseUnitSize == 1)
            baseUnitSize = 2;
        else if(baseUnitSize == 2)
            baseUnitSize = 10;
        else if (baseUnitSize > 40)
            baseUnitSize = 50;
        else if (baseUnitSize > 1)
            baseUnitSize += 10;
        else
            qDebug() << baseUnitSize << "Zoom(In) value over!";
    }else
    {
        //缩小
        if(baseUnitSize <= 10)
            baseUnitSize = 1;
        else if (baseUnitSize <= 50)
            baseUnitSize -= 10;
        else
            qDebug() << baseUnitSize << "Zoom(Out) value over!";
    }
    qDebug() << baseUnitSize << "Test zoom";
    update();
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
    viewColumn = 0;
    viewRow = 0;
    while (viewColumn * baseUnitSize < width() - baseUnitSize)
        ++viewColumn;
    while (viewRow * baseUnitSize < height() - baseUnitSize)
        ++viewRow;

    if(modelOffsetX + viewColumn > WORLDSIZE)
        modelOffsetX = WORLDSIZE - viewColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
    if(modelOffsetX < 0)
    {
        modelOffsetX = 0;
        viewColumn = WORLDSIZE;
    }//检查模型列是否小于视图列，若是则更改视图大小

    if(modelOffsetY + viewRow > WORLDSIZE)
        modelOffsetY = WORLDSIZE - viewRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行
    if(modelOffsetY < 0)
    {
        modelOffsetY = 0;
        viewRow = WORLDSIZE;
    }//检查模型行是否小于视图行，若是则更改视图大小

    QImage image(viewColumn * baseUnitSize, viewRow * baseUnitSize, QImage::Format_RGB32);

    for(int i = 0; i < viewColumn; ++i)
    {
        for(int j = 0; j < viewRow; ++j)
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
    viewOffsetX = (width() - viewColumn * baseUnitSize) / 2;
    viewOffsetY = (height() - viewRow * baseUnitSize) / 2;
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
    //基础单元为一个像素时取消参考线
    if(baseUnitSize <= 1)
        return;

    int level = 0;//参考线明度等级
    const QColor lineColor[5] = {VIEW::LUMINOSITY_1_17.rgb(),
                                 VIEW::LUMINOSITY_1_34.rgb(),
                                 VIEW::LUMINOSITY_2_68.rgb(),
                                 VIEW::LUMINOSITY_2_85.rgb(),
                                 VIEW::WARNING.rgb()
                                };

    if(baseUnitSize >= 10)
    {
        //绘制一级参考线，相隔一个基础单元
        painter.setPen(lineColor[level]);
        for(int i = 1; i < viewColumn; ++i)
            painter.drawLine(i * baseUnitSize, 0,
                             i * baseUnitSize, viewRow * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
        for(int j = 1; j < viewRow; ++j)
            painter.drawLine(0, j * baseUnitSize,
                             viewColumn * baseUnitSize - 1, j * baseUnitSize);//绘制行
        ++level;
    }

    //绘制二级参考线，相隔十个基础单元
    painter.setPen(lineColor[level]);
    for(int i = 10 - (modelOffsetX % 10); i < viewColumn; i += 10)
        painter.drawLine(i * baseUnitSize, 0,
                         i * baseUnitSize, viewRow * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 10 - (modelOffsetY % 10); j < viewRow; j += 10)
        painter.drawLine(0, j * baseUnitSize,
                         viewColumn * baseUnitSize - 1, j * baseUnitSize);//绘制行
    ++level;

    //绘制三级参考线，相隔百个基础单元
    painter.setPen(lineColor[level]);
    for(int i = 100 - (modelOffsetX % 100); i < viewColumn; i += 100)
        painter.drawLine(i * baseUnitSize, 0,
                         i * baseUnitSize, viewRow * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 100 - (modelOffsetY % 100); j < viewRow; j += 100)
        painter.drawLine(0, j * baseUnitSize,
                         viewColumn * baseUnitSize - 1, j * baseUnitSize);//绘制行
    ++level;

    if(WORLDSIZE > 1000)
    {
        //绘制三级参考线，相隔百个基础单元
        painter.setPen(lineColor[level]);
        for(int i = 1000 - (modelOffsetX % 1000); i < viewColumn; i += 1000)
            painter.drawLine(i * baseUnitSize, 0,
                             i * baseUnitSize, viewRow * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
        for(int j = 1000 - (modelOffsetY % 1000); j < viewRow; j += 1000)
            painter.drawLine(0, j * baseUnitSize,
                             viewColumn * baseUnitSize - 1, j * baseUnitSize);//绘制行
        ++level;
    }

    if(level < 0 || level > 4)
        qDebug() << level << "Line color error.";
}
