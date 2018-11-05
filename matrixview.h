#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>

class MatrixView : public QWidget
{
    Q_OBJECT

public:
    MatrixView(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

private:
    int baseUnit;
    const QRgb died;
    const QRgb lived;
};

#endif // MATRIXVIEW_H
