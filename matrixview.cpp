#include "matrixview.h"
//#include <QPalette>

MatrixView::MatrixView(MatrixModel &m, QWidget *parent) :
    QMainWindow(parent),
    model(m),
    modelSize(m.getModelSize()),
    viewOffsetX(0),
    viewOffsetY(0),
    modelOffsetX(0),
    modelOffsetY(0),
    viewColumn(0),
    viewRow(0),
    zoomList{ 1,2,4,8,16,32,64 },
    lineColor{
        MatrixColor::LUMINOSITY_1_17,
        MatrixColor::LUMINOSITY_1_34,
        MatrixColor::LUMINOSITY_1_51,
        MatrixColor::LUMINOSITY_2_68,
        },
    unitSize(zoomList[3]),
    unitsOnOff(true),
    gridOnOff(true),
    referencelineOnOff(true),
    fpsOnOff(true),
    animationOnOff(false),
    selectedUnitRect(),
    unitImage(),
    ppix(nullptr),
    imageWidth(0),
    fpsCount(0),
    frameSum(0),
    painter(this)
{
    //设置窗口背景色
    QPalette pal(palette());
    pal.setColor(QPalette::Background, MatrixColor::LUMINOSITY_1_51);
    setAutoFillBackground(true);
    setPalette(pal);

    //居中显示模型单元
    moveToCoordinate(modelSize/2, modelSize/2);

    fpsTime.start();
}

void MatrixView::resizeEvent(QResizeEvent *)
{
    updateViewSize();
    moveViewCheckup();
}

void MatrixView::paintEvent(QPaintEvent *)
{
    //添加坐标偏移，使视图居中于窗口
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

    if(unitsOnOff)
        drawBaseUnit();//绘制单元图像
    else
        unitsOnOff = true;
    painter.drawImage(0, 0, unitImage);//图像绘制到视图

    if(referencelineOnOff && unitSize != zoomList[0])
        drawReferenceLine();

    if(animationOnOff)//绘制动画
        animationOnOff = drawTakePicture();

    if(selectedUnitRect.isValid()) //绘制选框
        drawSelectBox();

    if(fpsOnOff)
    {
        frameSum++;
        //sum和fps在fpsThread中修改
        FPSCount();
        drawFPSText();
    }
}

void MatrixView::updateViewSize()
{
    unsigned mWidth = static_cast<unsigned>(width());
    unsigned mHeight = static_cast<unsigned>(height());
    //计算视图显示的单元行列
    viewColumn = mWidth / unitSize;
    viewRow = mHeight / unitSize;
    //如果可显示单元大于模型最大单元
    if(viewColumn > modelSize)
        viewColumn = modelSize;
    if(viewRow > modelSize)
        viewRow = modelSize;

    //计算视图偏移量
    viewOffsetX = (mWidth - viewColumn * modelSize) / 2;
    viewOffsetY = (mHeight - viewRow * modelSize) / 2;

    //构建显示模型单元的图像
    unitImage = QImage(viewColumn * unitSize, viewRow * unitSize, QImage::Format_RGB32);
    unitImage.fill(MatrixColor::LUMINOSITY_1_17);
    ppix = unitImage.bits();
    imageWidth = static_cast<unsigned>(unitImage.width());
}

void MatrixView::moveViewCheckup()
{
    static unsigned oldColumn = 0;
    static unsigned oldRow = 0;

    //行列变动时保持模型单元中心不变
    if(viewColumn != oldColumn)
    {
        modelOffsetX += oldColumn/2;
        modelOffsetX -= viewColumn/2;
        oldColumn = viewColumn;
    }
    else if(viewRow != oldRow)
    {
        modelOffsetY += oldRow/2;
        modelOffsetY -= viewRow/2;
        oldRow = viewRow;
    }

    //检查模型单元显示
    if(modelOffsetX + viewColumn > modelSize)
        modelOffsetX = modelSize - viewColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
    if(modelOffsetY + viewRow > modelSize)
        modelOffsetY = modelSize - viewRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行

}

void MatrixView::drawBaseUnit()
{
    for(unsigned i = 0; i < viewColumn; ++i)
    {
        for(unsigned j = 0; j < viewRow; ++j)
        {
            //Get modeldata and select color
            QRgb color;
            int value = model.getUnitValue(i + modelOffsetX, j + modelOffsetY);
            if(value <= 0)
                color = MatrixColor::LUMINOSITY_0_0.rgb();
            else
                color = MatrixColor::LUMINOSITY_4_204.rgb();

            //右下少绘制一行一列，用以形成一级参考线
            unsigned interval = 0;
            if(unitSize < zoomList[3] || !gridOnOff)
                interval = 0;
            else
                interval = 4;

            unsigned char b = color & 0Xff;
            unsigned char g = color>>8 & 0Xff;
            unsigned char r = color>>16 & 0Xff;

            unsigned x = i * unitSize * 4;
            unsigned y = j * unitSize * 4;
            for(unsigned k = x + interval; i < x + (unitSize*4); i += 4)
            {
                for(unsigned l = y + interval; j < y + (unitSize*4); j += 4)
                {
                    //image.setPixel(i, j, color); //以一个个像素点绘制基础单元
                    *(ppix + k + l*imageWidth) = b; //B
                    *(ppix + k + l*imageWidth +1) = g; //G
                    *(ppix + k + l*imageWidth +2) = r; //R
                }
            }
        }
    }
}

void MatrixView::drawReferenceLine()
{
    unsigned level;//参考线明度等级
    level = unitSize < 8 ? 0 : 1;

    //二级参考线，相隔十个单元
    painter.setPen(lineColor[level]);
    for(unsigned i = 10 - (modelOffsetX % 10); i < viewColumn; i += 10)
        painter.drawLine(i * unitSize, 0,
                         i * unitSize, viewRow * unitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(unsigned j = 10 - (modelOffsetY % 10); j < viewRow; j += 10)
        painter.drawLine(0, j * unitSize,
                         viewColumn * unitSize - 1, j * unitSize);//绘制行
    ++level;

    //三级参考线，相隔百个单元
    painter.setPen(lineColor[level]);
    for(unsigned i = 100 - (modelOffsetX % 100); i < viewColumn; i += 100)
        painter.drawLine(i * unitSize, 0,
                         i * unitSize, viewRow * unitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(unsigned j = 100 - (modelOffsetY % 100); j < viewRow; j += 100)
        painter.drawLine(0, j * unitSize,
                         viewColumn * unitSize - 1, j * unitSize);//绘制行
    ++level;

    //四级参考线，相隔千个单元
    painter.setPen(lineColor[level]);
    for(unsigned i = 1000 - (modelOffsetX % 1000); i < viewColumn; i += 1000)
        painter.drawLine(i * unitSize, 0,
                         i * unitSize, viewRow * unitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(unsigned j = 1000 - (modelOffsetY % 1000); j < viewRow; j += 1000)
        painter.drawLine(0, j * unitSize,
                         viewColumn * unitSize - 1, j * unitSize);//绘制行
    ++level;
}

bool MatrixView::drawTakePicture()
{
    static short sum = 0;

    if(sum == 8)
    {
        sum = 0;
        return false;
    }

    if(selectedUnitRect.isValid())
    {
        QImage picture(selectedUnitRect.width(), selectedUnitRect.height(), QImage::Format_RGB32);
        picture.fill(MatrixColor::LUMINOSITY_1_17);
        painter.drawImage(selectedUnitRect, picture);
    }
    else
    {
        QImage picture = unitImage;
        picture.fill(MatrixColor::LUMINOSITY_1_17);
        painter.drawImage(0, 0, picture);
    }

    ++sum;

    noRedrawUnit();

    return true;
}

void MatrixView::drawFPSText()
{
    QPoint point(-viewOffsetX + 10, -viewOffsetY + 15);

    static QPixmap fpsText(":/texts/FPS");
    static QPixmap pointText(":/texts/point");
    static QPixmap number[10]
    {
                QPixmap(":/texts/0"),
                QPixmap(":/texts/1"),
                QPixmap(":/texts/2"),
                QPixmap(":/texts/3"),
                QPixmap(":/texts/4"),
                QPixmap(":/texts/5"),
                QPixmap(":/texts/6"),
                QPixmap(":/texts/7"),
                QPixmap(":/texts/8"),
                QPixmap(":/texts/9"),
    };

    painter.setPen(QColor(MatrixColor::WARNING));
    painter.drawPixmap(point,fpsText);
    point.setX(point.x() + fpsText.width());

    size_t fpsI = static_cast<size_t>(fpsCount);
    if(fpsI >= 100)
    {
        painter.drawPixmap(point, number[fpsI/100]);
        point.setX(point.x() + number[0].width());

        painter.drawPixmap(point, number[(fpsI%100)/10]);
        point.setX(point.x() + number[0].width());

        painter.drawPixmap(point, number[fpsI%10]);
        point.setX(point.x() + number[0].width());
    }
    else if(fpsI >= 10)
    {
        painter.drawPixmap(point, number[fpsI/10]);
        point.setX(point.x() + number[0].width());

        painter.drawPixmap(point, number[fpsI%10]);
        point.setX(point.x() + number[0].width());
    }else
    {
        painter.drawPixmap(point, number[fpsI]);
        point.setX(point.x() + number[0].width());
    }

    size_t fpsF = static_cast<size_t>((fpsCount - fpsI)*10);
    if(fpsF != 0)
    {
        painter.drawPixmap(point, pointText);
        point.setX(point.x() + number[0].width());
        painter.drawPixmap(point, number[fpsF]);
    }
}
