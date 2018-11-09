#include "matrixview.h"

#include <QDebug>
#include <QPainter>

MatrixView::MatrixView(MatrixModel *model, QWidget *parent)
    :QWidget(parent),
      died(0),
      lived(1),
      dieColor(VIEW::LUMINOSITY_0_0.rgb()),
      liveColer(VIEW::LUMINOSITY_5_255.rgb()),
      model(model)
{
    //Set window backgroundcolor
    QPalette pal = palette();
    pal.setColor(QPalette::Background, VIEW::LUMINOSITY_1_51);
    setAutoFillBackground(true);
    setPalette(pal);

    baseUnitSize = 2;//默认的一个单元大小

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
    //若基础单元小于10象素，则把参考线的相隔单位扩大十倍
    int baseSize = baseUnitSize;
    int column = viewColumn;qDebug() << column << "-->This is Column()";
    int row = viewRow;

    if (baseSize < 10)
    {
        baseSize *= 10;
        column /= 10;
        //row *= 10;
    }

    //绘制一级参考线，相隔一个基础单元
    painter.setPen(VIEW::LUMINOSITY_1_17.rgb());
    for(int i = 1; i < column; ++i)
        painter.drawLine(i * baseSize, 0,
                         i * baseSize, row * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 1; j < row; ++j)
        painter.drawLine(0, j * baseSize,
                         column * baseUnitSize - 1, j * baseSize);//绘制行

    //绘制二级参考线，相隔十个基础单元
    painter.setPen(VIEW::LUMINOSITY_1_34.rgb());
    for(int i = 10 - (modelOffsetX % 10); i < column; i += 10)
        painter.drawLine(i * baseSize, 0,
                         i * baseSize, row * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 10 - (modelOffsetY % 10); j < row; j += 10)
        painter.drawLine(0, j * baseSize,
                         column * baseUnitSize - 1, j * baseSize);//绘制行

    //绘制三级参考线，相隔百个基础单元
    painter.setPen(VIEW::LUMINOSITY_2_68.rgb());
    for(int i = 100 - (modelOffsetX % 100); i < column; i += 100)
        painter.drawLine(i * baseSize, 0,
                         i * baseSize, row * baseUnitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 100 - (modelOffsetY % 100); j < row; j += 100)
        painter.drawLine(0, j * baseSize,
                         column * baseUnitSize - 1, j * baseSize);//绘制行
}
