#include "matrixview.h"
#include "constants.h"

#include <QDebug>
#include <QPainter>

MatrixView::MatrixView(QWidget *parent)
    :QWidget(parent),
     died(qRgb(0, 0, 0)),
     lived(qRgb(255, 255, 255))
{
    baseUnit = 32;
}

void MatrixView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QImage image(size(), QImage::Format_RGB32);

    for(int i = 0; i * baseUnit < width(); ++i)
    {
        for(int j = 0; j * baseUnit < height(); ++j)
        {
            image.setPixel(i * baseUnit, j *baseUnit, died); // <---修改，依据model判断。
        }
    }

    painter.drawImage(0, 0, image);

    qDebug() << width() << "-->This is size()";
    qDebug() << size() << "-->This is size()";
}
