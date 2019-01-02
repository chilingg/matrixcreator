#include "matrixview.h"
//#include <QPalette>

MatrixView::MatrixView(MatrixModel &m, QWidget *parent) :
    QMainWindow(parent),
    model(m),
    MODELSIZE(static_cast<int>(m.getModelSize())),
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
    unitsDspl(true),
    gridDspl(true),
    rflDspl(true),
    fpsDspl(true),
    selectedUnitRect(),
    unitImage(),
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
    moveToCoordinate(MODELSIZE/2, MODELSIZE/2);

    fpsTime.start();
}

MPoint MatrixView::inView(QPoint clicktedPos) const
{
    MPoint cdt {false,0,0,clicktedPos,QRect()};
    int viewX = clicktedPos.x() - viewOffsetX;
    viewX -= viewX % unitSize;
    int viewY = clicktedPos.y() - viewOffsetY;
    viewY -= viewY % unitSize;

    if(viewX >= 0 && viewColumn * unitSize && viewY >= 0 && viewY < viewRow * unitSize)
    {
        cdt.valid = true;
            cdt.modelColumn = viewX / unitSize + modelOffsetX;
            cdt.modelRow = viewY / unitSize + modelOffsetY;
            cdt.viewRect = QRect(QPoint(viewX, viewY),
                                 QPoint(viewX+unitSize-1, viewY+unitSize-1));
    }

    return cdt;
}

void MatrixView::zoomView(MPoint cdt, MatrixView::Zoom zoom)
{
    //获取当前缩放级别
    size_t level = 0;
    while(level < zoomList.size() && zoomList[level] >= unitSize)
    {
        ++level;
    }

    if(zoom == ZoomIn)
    {
        if(level < zoomList.size() - 1)
            unitSize = zoomList[level+1];
        else if(level == zoomList.size() - 1)
            unitSize = zoomList[level];//修复过大的unitsize
    }
    else if(zoom == ZoomOut && level > 0)
    {
        unitSize = zoomList[level - 1];
    }

    updateViewSize();

    //缩放后鼠标处的模型坐标
    MPoint afterCdt = inView(cdt.clickted);
    if(!afterCdt.valid)
        return;

    //修改模型偏差值，使鼠标位置下的单元仍是缩放之前的单元
    modelOffsetX -= afterCdt.modelColumn - cdt.modelColumn;
    modelOffsetY -= afterCdt.modelRow - cdt.modelRow;

    moveViewCheckup();
}

void MatrixView::takePicture(QString path)
{
    //图片的后缀，避免同名文件
    static short sum = 0;
    QDateTime currentTime = QDateTime::currentDateTime();

    if(selectedUnitRect.isValid())
    {
        QImage picture(selectedUnitRect.width(), selectedUnitRect.height(), QImage::Format_RGB32);
        picture.fill(MatrixColor::LUMINOSITY_1_17);

        //绘制模型图像
        int modelLeft = selectedUnitRect.left() / unitSize;
        int modelTop = selectedUnitRect.top() / unitSize;
        int modelWidth = selectedUnitRect.width() / unitSize;
        int modelHeight = selectedUnitRect.height() / unitSize;
        drawBaseUnits(modelLeft, modelTop, modelWidth, modelHeight, picture);

        picture.save(path + currentTime.toString("yyyyMMddhhmm-ss%1.png").arg(sum), "PNG");
    }
    else
    {
        unitImage.save(path + currentTime.toString("yyyyMMddhhmm-ss0%1.png").arg(sum), "PNG");
    }
}

void MatrixView::resizeEvent(QResizeEvent *)
{
    updateViewSize();
}

void MatrixView::paintEvent(QPaintEvent *)
{
    //添加坐标偏移，使视图居中于窗口
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

    if(unitsDspl)
        drawBaseUnits();//绘制单元图像
    else
        unitsDspl = true;
    painter.drawImage(0, 0, unitImage);//图像绘制到视图

    if(rflDspl && unitSize != zoomList[0])
        drawReferenceLine();

    if(selectedUnitRect.isValid()) //绘制选框
        drawSelectBox();

    if(fpsDspl)
    {
        frameSum++;
        //sum和fps在fpsThread中修改
        FPSCount();
        drawFPSText();
    }
}

void MatrixView::updateViewSize()
{
    int oldColumn = viewColumn;
    int oldRow = viewRow;

    //计算视图显示的单元行列
    viewColumn = width() / unitSize;
    viewRow = height() / unitSize;
    //如果可显示单元大于模型最大单元
    if(viewColumn > MODELSIZE)
        viewColumn = MODELSIZE;
    if(viewRow > MODELSIZE)
        viewRow = MODELSIZE;

    //行列变动时保持模型单元中心不变
    if(viewColumn != oldColumn)
    {
        modelOffsetX += oldColumn/2;
        modelOffsetX -= viewColumn/2;
    }
    else if(viewRow != oldRow)
    {
        modelOffsetY += oldRow/2;
        modelOffsetY -= viewRow/2;
    }
    moveViewCheckup();

    //计算视图偏移量
    viewOffsetX = (width() - viewColumn * unitSize) / 2;
    viewOffsetY = (height() - viewRow * unitSize) / 2;

    //构建显示模型单元的图像
    unitImage = QImage(viewColumn * unitSize, viewRow * unitSize, QImage::Format_RGB32);
    unitImage.fill(MatrixColor::LUMINOSITY_1_17);
}

void MatrixView::drawBaseUnits(int left, int top, int mWidth, int mHeight, QImage &picture)
{
    unsigned char *pp = picture.bits();
    int pWidht = picture.width();

    for(int i = 0; i < mWidth; ++i)
    {
        for(int j = 0; j < mHeight; ++j)
        {
            //Get modeldata and select color
            QRgb color;
            int value = model.getUnitValue(i + modelOffsetX + left, j + modelOffsetY + top);
            if(value <= 0)
                color = MatrixColor::LUMINOSITY_0_0.rgb();
            else
                color = MatrixColor::LUMINOSITY_4_204.rgb();

            //右下少绘制一行一列，用以形成一级参考线
            int interval = 0;
            if(unitSize < zoomList[3] || !gridDspl)
                interval = 0;
            else
                interval = 4;

            unsigned char b = color & 0Xff;
            unsigned char g = color>>8 & 0Xff;
            unsigned char r = color>>16 & 0Xff;

            int x = i * unitSize * 4;
            int y = j * unitSize * 4;
            for(int k = x + interval; i < x + (unitSize*4); i += 4)
            {
                for(int l = y + interval; j < y + (unitSize*4); j += 4)
                {
                    //image.setPixel(i, j, color); //以一个个像素点绘制基础单元
                    *(pp + k + l*pWidht) = b; //B
                    *(pp + k + l*pWidht +1) = g; //G
                    *(pp + k + l*pWidht +2) = r; //R
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
    for(int i = 10 - (modelOffsetX % 10); i < viewColumn; i += 10)
        painter.drawLine(i * unitSize, 0,
                         i * unitSize, viewRow * unitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 10 - (modelOffsetY % 10); j < viewRow; j += 10)
        painter.drawLine(0, j * unitSize,
                         viewColumn * unitSize - 1, j * unitSize);//绘制行
    ++level;

    //三级参考线，相隔百个单元
    painter.setPen(lineColor[level]);
    for(int i = 100 - (modelOffsetX % 100); i < viewColumn; i += 100)
        painter.drawLine(i * unitSize, 0,
                         i * unitSize, viewRow * unitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 100 - (modelOffsetY % 100); j < viewRow; j += 100)
        painter.drawLine(0, j * unitSize,
                         viewColumn * unitSize - 1, j * unitSize);//绘制行
    ++level;

    //四级参考线，相隔千个单元
    painter.setPen(lineColor[level]);
    for(int i = 1000 - (modelOffsetX % 1000); i < viewColumn; i += 1000)
        painter.drawLine(i * unitSize, 0,
                         i * unitSize, viewRow * unitSize - 1);//绘制列，因以0点象素起，需扣除多的一点象素
    for(int j = 1000 - (modelOffsetY % 1000); j < viewRow; j += 1000)
        painter.drawLine(0, j * unitSize,
                         viewColumn * unitSize - 1, j * unitSize);//绘制行
    ++level;
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
