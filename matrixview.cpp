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

    baseUnitSize = 10;//默认的一个单元大小

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
    int referenceLineSize = baseUnitSize >= 10 ? baseUnitSize : baseUnitSize * 10;

    for(int i = referenceLineSize; i < modelColumn * referenceLineSize; i += referenceLineSize)
    {
        if((i / referenceLineSize + modelOffsetX) % 10 != 0)
            painter.setPen(VIEW::LUMINOSITY_1_17.rgb());
        else if((i / referenceLineSize + modelOffsetX) % 100 != 0)
            painter.setPen(VIEW::LUMINOSITY_1_34.rgb());
        else
            painter.setPen(VIEW::LUMINOSITY_2_68.rgb());

        painter.drawLine(i, 0, i, modelRow * referenceLineSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    }

    for(int j = referenceLineSize; j < modelRow * referenceLineSize; j += referenceLineSize)
    {
        if((j / referenceLineSize + modelOffsetX) % 10 != 0)
            painter.setPen(VIEW::LUMINOSITY_1_17.rgb());
        else if((j / referenceLineSize + modelOffsetX) % 100 != 0)
            painter.setPen(VIEW::LUMINOSITY_1_34.rgb());
        else
            painter.setPen(VIEW::LUMINOSITY_1_51.rgb());

        painter.drawLine(0, j, modelColumn * referenceLineSize - 1, j);//绘制行
        //qDebug() << i << j - referenceLineSize << modelColumn * referenceLineSize;
    }
}
