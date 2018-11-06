#include "matrixview.h"
#include "constants.h"

#include <QDebug>
#include <QPainter>

MatrixView::MatrixView(MatrixModel *model, QWidget *parent)
    :QWidget(parent),
      died(0),
      lived(1),
      dieColor(qRgb(7, 0, 0)),
      liveColer(qRgb(204, 204, 204)),
      model(model)
{
    baseUnitSize = 32;
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QImage image(size(), QImage::Format_RGB32);
    painter.drawLine(0, 0, 100, 100);//Test

    for(int i = 0; i * baseUnitSize < width() - baseUnitSize; ++i)
    {
        for(int j = 0; j * baseUnitSize < height() - baseUnitSize; ++j)
        {
            QRgb color;
            int value = model->getModelValue(i, j);

            if(value == died)
                color = dieColor;
            else if(value == lived)
                color = liveColer;
            else
                color = qRgb(255, 0, 0);

            qDebug() << i << j << value << "-->This is color()";
            drawBaseUnit(i * baseUnitSize, j * baseUnitSize, color, image);
        }
    }

    painter.drawImage(0, 0, image);
    referenceLine(painter);

    qDebug() << width() << "-->This is width()";
    qDebug() << size() << "-->This is size()";
}

void MatrixView::drawBaseUnit(int x, int y, QRgb color, QImage &image)
{
    for(int i = x; i < x + baseUnitSize; ++i)
    {
        for(int j = y; j < y + baseUnitSize; ++j)
        {
            image.setPixel(i, j, color); //qRgb(255,0,0)
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
