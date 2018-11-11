#include "matrixview.h"

#include <QDebug>
#include <QPainter>

MatrixView::MatrixView(MatrixModel *model, QWidget *parent)
    :QWidget(parent),
      model(model),
      died(0),
      lived(1),
      dieColor(VIEW::LUMINOSITY_0_0.rgb()),
      liveColer(VIEW::LUMINOSITY_5_255.rgb()),
      zoomList{1,2,4,8,16,20,30,40,50}
{
    //Set window backgroundcolor
    QPalette pal = palette();
    pal.setColor(QPalette::Background, VIEW::LUMINOSITY_1_51);
    setAutoFillBackground(true);
    setPalette(pal);

    baseUnitSize = zoomList[3];//默认的一个单元大小

    viewOffsetX = 0;
    viewOffsetY = 0;
    modelOffsetX = WORLDSIZE / 2;
    modelOffsetY = WORLDSIZE / 2;
    viewColumn = 0;
    viewRow = 0;

}

bool MatrixView::isInView(int clickedX, int clickedY)
{
    clickedX -= viewOffsetX;
    clickedY -= viewOffsetY;

    //判断点击是否发生在view范围内
    if (clickedX > 0 && clickedX < viewColumn * baseUnitSize)
    {
        if (clickedY > 0 && clickedY < viewRow * baseUnitSize)
        {
            return true;
        }
    }

    qDebug() << "Point over!";
    qDebug() << viewColumn << viewRow << baseUnitSize << "C&R&B";
    qDebug() << clickedX / baseUnitSize << clickedX << "-->pos().x";
    qDebug() << clickedY / baseUnitSize << clickedY << "-->pos().y";
    return false;
}

bool MatrixView::isInView(QPoint pos)
{
    pos -= QPoint(viewOffsetX, viewOffsetY);

    //判断点击是否发生在view范围内
    if (pos.x() > 0 && pos.x() < viewColumn * baseUnitSize)
    {
        if (pos.y() > 0 && pos.y() < viewRow * baseUnitSize)
        {
            return true;
        }
    }

    qDebug() << "Point over!";
    qDebug() << viewColumn << viewRow << baseUnitSize << "C&R&B";
    qDebug() << pos.x() / baseUnitSize << pos.x() << "-->pos().x";
    qDebug() << pos.y() / baseUnitSize << pos.y() << "-->pos().y";
    return false;
}

QPoint MatrixView::getModelPoint(int clickedX, int clickedY)
{
    isInView(clickedX, clickedY);

    clickedX -= viewOffsetX;
    clickedY -= viewOffsetY;

    //qDebug() << clickedX << clickedY << "Clicked X and Y";
    clickedX = clickedX / baseUnitSize + modelOffsetX;
    clickedY = clickedY / baseUnitSize + modelOffsetY;

    return QPoint(clickedX, clickedY);
}

QPoint MatrixView::getUnitPoint(int modelX, int modelY) const
{
    int x = modelX * baseUnitSize + baseUnitSize / 2;
    int y = modelY * baseUnitSize + baseUnitSize / 2;
    
    return QPoint(x, y);
}

QPoint MatrixView::getUnitPoint(QPoint modelPoint) const
{
    int x = modelPoint.x() - modelOffsetX;
    int y = modelPoint.y() - modelOffsetY;

    if(x < 0 || x > viewColumn)
        qDebug() << "Unit not in view!" << x << y;
    if(y < 0 || y > viewRow)
        qDebug() << "Unit not in view!" << x << y;

    x = x * baseUnitSize + baseUnitSize / 2;
    y = y * baseUnitSize + baseUnitSize / 2;

    return QPoint(x, y);
}

QPoint MatrixView::getviewOffsetPoint() const
{
    return QPoint(viewOffsetX, viewOffsetY);
}

int MatrixView::getBaseUnitSize() const
{
    return baseUnitSize;
}

void MatrixView::zoomView(int clickedX, int clickedY, bool zoom)
{
    //缩放之前鼠标处的模型坐标
    QPoint beforePoint = getModelPoint(clickedX, clickedY);

    //修正可能存在的错误
    if(baseUnitSize < zoomList[0] || baseUnitSize > zoomList[8])
    {
        qDebug() << baseUnitSize << "Zoom value over!";
        baseUnitSize < zoomList[0] ? baseUnitSize = zoomList[0] : baseUnitSize = zoomList[8];
    }

    //计算当前缩放级别。考虑以后可能会有的自定义基础单元大小，使用while循环计算
    int level = 0;
    while (level < 9)
    {
        if(zoomList[level] >= baseUnitSize)
        {
            //--level;
            break;
        }
        ++level;
    }

    if(zoom && level != 8)
        baseUnitSize = zoomList[level+1];
    else if(zoom && level == 8)
        baseUnitSize = zoomList[level];

    if(!zoom && level != 0)
        baseUnitSize = zoomList[level-1];
    else if(!zoom && level == 0)
        baseUnitSize = zoomList[level];

    updateViewData();//因为update()不会立即重绘视图，所以需在此更新数据，以便在重绘函数中的updateViewData()调用前能得到正确的数据

    //缩放后鼠标处的模型坐标
    QPoint afterPoint = getModelPoint(clickedX, clickedY);

    //修改模型偏差值，使鼠标位置下的单元仍是缩放之前的单元
    modelOffsetX -= afterPoint.x() - beforePoint.x();
    modelOffsetY -= afterPoint.y() - beforePoint.y();

    //qDebug() << QPoint(clickedX, clickedX) << "Test clicked point";
    //qDebug() << QPoint(afterX, afterY) << QPoint(beforeX, beforeY) << "Test zoom point";
    //qDebug() << baseUnitSize << level << "Test zoom";

    update();//在事件循环后重绘视图
}

void MatrixView::updateViewData()
{
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

    //计算视图偏移量
    viewOffsetX = (width() - viewColumn * baseUnitSize) / 2;
    viewOffsetY = (height() - viewRow * baseUnitSize) / 2;
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    updateViewData();

    //添加坐标偏移，使模型居中于视图
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

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

    if(baseUnitSize >= 8)
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
