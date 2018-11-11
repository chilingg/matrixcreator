#include <QDebug>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      model(new MatrixModel()),
      view(new MatrixView(model, this))
{
    setWindowTitle(tr("MatrixCreator"));
    resize(840, 720);
    setCentralWidget(view);

    startTimer(1000/10);
    start = false;
    movePos = QPoint(0, 0);
}

MainWindow::~MainWindow()
{

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && start == false)
    {
        int x = event->pos().x();
        int y = event->pos().y();

        //qDebug() << QPoint(x - view->getviewOffsetPoint().x(),
                           //y - view->getviewOffsetPoint().y()) << "Clicked x & y";

        //点击坐标转换到model坐标, 若不在view范围内则退出函数
        if(!(view->isInView(x, y)))
            return;

        QPoint modelPoint = view->getModelPoint(x, y);

        //点击的单元转换为相反状态
        model->changeModelValue(modelPoint.x(), modelPoint.y());
        view->update();

    }

    if (event->button() == Qt::MidButton)
    {
        //点击坐标转换到model坐标, 若不在view范围内则退出函数
        if(!(view->isInView(event->pos())))
            return;

        movePos = event->pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::MiddleButton)
    {
        QPoint point = event->pos();
        if(point.x() - movePos.x() == view->getBaseUnitSize())//Lfet
        {
            view->moveView(1, 0, 0, 0);
            movePos.setX(point.x());
        }
        if(movePos.x() - point.x() == view->getBaseUnitSize())//Right
            view->moveView(0, 0, 0, 1);
        if(point.y() - movePos.y() == view->getBaseUnitSize())//Lfet
            view->moveView(0, 1, 0, 0);
        if(movePos.y() - point.y() == view->getBaseUnitSize())//Right
            view->moveView(0, 0, 1, 1);

        qDebug() << point << "Mouse move test";
    }
}

void MainWindow::timerEvent(QTimerEvent *)
{
    if(start == false)
        return;

    //qDebug() << "-->In the timer";
    model->updateModel();
    view->update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
        start = !start;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    int clickedX = event->pos().x();
    int clickedY = event->pos().y();

    //滚动时指针的坐标转换到model坐标, 若不在view范围内则调用父类同名函数处理
    if(!(view->isInView(clickedX, clickedY)))
    {
        QMainWindow::wheelEvent(event);
        return;
    }
    
    if(event->delta() > 0)
        view->zoomView(clickedX, clickedY, true);//缩小
    else
        view->zoomView(clickedX, clickedY, false);//放大
    
    //获取鼠标所处单元的中心点坐标和view与全局坐标的偏差值，并把光标移到该坐标
    QPoint unitPoint = view->getUnitPoint(view->getModelPoint(clickedX, clickedY));
    QPoint offset = event->globalPos() - event->pos() + view->getviewOffsetPoint();
    cursor().setPos(unitPoint + offset);
    //qDebug() << event->globalPos() << event->pos() << view->getviewOffsetPoint() << offset << "G-P-V";
    //qDebug() << cursor().pos() << "After";
}
