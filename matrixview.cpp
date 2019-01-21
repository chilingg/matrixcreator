#include "matrixview.h"

MatrixView::MatrixView(MatrixModel &m, QWidget *parent) :
    QWidget(parent),
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
    unitSize(zoomList[4]),
    unitsDspl(true),
    gridDspl(true),
    rflDspl(true),
    fpsDspl(true),
    overRange{false,false,false,false,false},
    selectedViewRect(),
    unitImage(),
    fpsCount(0),
    frameSum(0),
    fpsTime()
{
    //设置窗口背景色
    QPalette pal(palette());
    pal.setColor(QPalette::Background, MatrixColor::LUMINOSITY_1_51);
    setAutoFillBackground(true);
    setPalette(pal);

    //居中显示模型单元
    moveToCoordinate(MODELSIZE/2, MODELSIZE/2);

    fpsTime.start();

    switchColorPattern(model.getCurrentPattern());
}

MPoint MatrixView::inView(QPoint clicktedPos) const
{
    MPoint cdt {false,0,0,clicktedPos,QRect()};
    int viewX = clicktedPos.x() - viewOffsetX;
    viewX -= viewX % unitSize;
    int viewY = clicktedPos.y() - viewOffsetY;
    viewY -= viewY % unitSize;

    if(viewX >= 0 && viewX <  viewColumn * unitSize && viewY >= 0 && viewY < viewRow * unitSize)
    {
        cdt.valid = true;
            cdt.modelColumn = viewX / unitSize + modelOffsetX;
            cdt.modelRow = viewY / unitSize + modelOffsetY;
            cdt.viewRect = QRect(QPoint(viewX, viewY),
                                 QPoint(viewX+unitSize-1, viewY+unitSize-1));
    }

    return cdt;
}

void MatrixView::moveViewCheckupDisplay()
{
    //检查模型单元显示
    if(modelOffsetX + viewColumn > MODELSIZE)
    {
        modelOffsetX = MODELSIZE - viewColumn;//检查视图列是否越界，若是则让视图刚好显示模型最后一列
        viewOverRange(false,false,false,true);
    }
    else if(modelOffsetX < 0)
    {
        modelOffsetX = 0;
        viewOverRange(false,false,true,false);
    }

    if(modelOffsetY + viewRow > MODELSIZE)
    {
        modelOffsetY = MODELSIZE - viewRow;//检查视图行是否越界，若是则让视图刚好显示模型最后一行
        viewOverRange(false,true,false,false);
    }
    else if(modelOffsetY < 0)
    {
        modelOffsetY = 0;
        viewOverRange(true,false,false,false);
    }
}

void MatrixView::switchColorPattern(MatrixModel::ModelPattern pattern)
{
    switch(pattern)
    {
    case MatrixModel::LifeGameTSF:
        valueToColor = tValueToColor;
        break;
    case MatrixModel::LifeGameCCL:
        valueToColor = cValueToColor;
        break;
    case MatrixModel::LifeGameTRC:
        valueToColor = cValueToColor;
        break;
    default:
        valueToColor = nullptr;
    }
}

void MatrixView::zoomView(MPoint cdt, MatrixView::Zoom zoom)
{
    //获取当前缩放级别
    size_t level = 0;
    while(level < zoomList.size() && zoomList[level] < unitSize)
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

    if(selectedViewRect.isValid())
    {
        QImage picture(selectedViewRect.width(), selectedViewRect.height(), QImage::Format_RGB32);
        picture.fill(MatrixColor::LUMINOSITY_1_17);

        //绘制模型图像
        int modelLeft = selectedViewRect.left() / unitSize;
        int modelTop = selectedViewRect.top() / unitSize;
        int modelWidth = selectedViewRect.width() / unitSize;
        int modelHeight = selectedViewRect.height() / unitSize;
        drawBaseUnits(modelLeft, modelTop, modelWidth, modelHeight, picture);

        if(!path.isEmpty())
            picture.save(currentTime.toString("yyyyMMddhhmm-ss%1.png").arg(++sum), "PNG");
        else
            picture.save(path, "PNG");
    }
    else
    {
        if(!path.isEmpty())
            unitImage.save(currentTime.toString("yyyyMMddhhmm-ss%1.png").arg(++sum), "PNG");
        else
            unitImage.save(path, "PNG");
    }
}

void MatrixView::resizeEvent(QResizeEvent *)
{
    int oldColumn = viewColumn;
    int oldRow = viewRow;

    updateViewSize();

    //行列变动时保持模型单元中心不变
    if(viewColumn != oldColumn)
    {
        modelOffsetX += oldColumn/2;
        modelOffsetX -= viewColumn/2;
    }
    if(viewRow != oldRow)
    {
        modelOffsetY += oldRow/2;
        modelOffsetY -= viewRow/2;
    }
    moveViewCheckup();
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    //添加坐标偏移，使视图居中于窗口
    painter.setWindow(-viewOffsetX, -viewOffsetY, width(), height());

    if(unitsDspl)
        drawBaseUnits();//绘制单元图像
    else
        unitsDspl = true;
    painter.drawImage(0, 0, unitImage);//图像绘制到视图

    if(rflDspl && unitSize != zoomList[0])
        drawReferenceLine(painter);

    if(selectedViewRect.isValid()) //绘制选框
        drawSelectBox(painter);

    if(overRange[0])//绘制越界提示线
        drawOverRangeLine(painter);

    if(fpsDspl)
    {
        ++frameSum;
        //sum和fps在fpsThread中修改
        FPSCount();
        drawFPSText(painter);
    }
}

void MatrixView::updateViewSize()
{
    //计算视图显示的单元行列
    viewColumn = width() / unitSize;
    viewRow = height() / unitSize;
    //如果可显示单元大于模型最大单元
    if(viewColumn > MODELSIZE)
        viewColumn = MODELSIZE;
    if(viewRow > MODELSIZE)
        viewRow = MODELSIZE;

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
            color = valueToColor(value);

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
            for(int k = x + interval; k < x + (unitSize*4); k += 4)
            {
                for(int l = y + interval; l < y + (unitSize*4); l += 4)
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

void MatrixView::drawReferenceLine(QPainter &painter)
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

void MatrixView::drawFPSText(QPainter &painter)
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

void MatrixView::drawOverRangeLine(QPainter &painter)
{
    painter.save();
    painter.setPen(Qt::NoPen);

    if(overRange[1])
    {
        QLinearGradient gradient(0, 0, 0, unitSize);
        gradient.setColorAt(0.0, MatrixColor::WARNING);
        gradient.setColorAt(1.0, MatrixColor::LUMINOSITY_0_0);
        painter.setBrush(QBrush(gradient));
        painter.drawRect(0, 0, viewColumn*unitSize, unitSize);
    }
    else if(overRange[2])
    {
        QLinearGradient gradient(0, viewRow*unitSize - unitSize, 0, viewRow*unitSize);
        gradient.setColorAt(0.0, MatrixColor::LUMINOSITY_0_0);
        gradient.setColorAt(1.0, MatrixColor::WARNING);
        painter.setBrush(QBrush(gradient));
        painter.drawRect(0, viewRow*unitSize - unitSize, viewColumn*unitSize, unitSize);
    }
    else if(overRange[3])
    {
        QLinearGradient gradient(0, 0, unitSize, 0);
        gradient.setColorAt(0.0, MatrixColor::WARNING);
        gradient.setColorAt(1.0, MatrixColor::LUMINOSITY_0_0);
        painter.setBrush(QBrush(gradient));
        painter.drawRect(0, 0, unitSize, viewRow*unitSize);
    }
    else if(overRange[4])
    {
        QLinearGradient gradient(viewColumn*unitSize - unitSize, 0, viewColumn*unitSize, 0);
        gradient.setColorAt(0.0, MatrixColor::LUMINOSITY_0_0);
        gradient.setColorAt(1.0, MatrixColor::WARNING);
        painter.setBrush(QBrush(gradient));
        painter.drawRect(viewColumn*unitSize - unitSize, 0, unitSize, viewRow*unitSize);
    }

    painter.restore();
}

QRgb MatrixView::tValueToColor(int value)
{
    if(value == 0)
        return MatrixColor::LUMINOSITY_0_0.rgb();
    else
        return MatrixColor::LUMINOSITY_4_204.rgb();
}

QRgb MatrixView::cValueToColor(int value)
{
    QRgb color = 0;
    switch (value)
    {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        color = MatrixColor::LUMINOSITY_4_204.rgb();
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 10:
    case 11:
        break;
    case 12:
    case 13:
        color = MatrixColor::LUMINOSITY_4_204.rgb();
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        break;
    default:
        color = MatrixColor::WARNING.rgb();
#ifndef M_NO_DEBUG
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "Value Over range!" << value ;
#endif
        break;
    }
    return  color;
}
