#ifndef MATRIXCONTROLLER_H
#define MATRIXCONTROLLER_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QWheelEvent>

#include "matrixmodel.h"
#include "matrixview.h"

enum SelectTool
{
    POINT,
    CIRCLE
};

class MatriController : public QMainWindow
{
    Q_OBJECT

public:
    MatriController(QWidget *parent = 0);
    ~MatriController();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    MatrixModel *model;
    MatrixView *view;
    bool start;
    QPoint moveViewPos;
    QRect selectRect;
    SelectTool selectTool;
};

#endif // MATRIXCONTROLLER_H