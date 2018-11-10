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

        //点击坐标转换到model坐标, 若不在view范围内则退出函数
        if(!(view->toModelPoint(x, y)))
            return;

        //点击的单元转换为相反状态
        model->changeModelValue(x, y);
        view->update();
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
    int x = event->pos().x();
    int y = event->pos().y();
    //滚动时指针的坐标转换到model坐标, 若不在view范围内则调用父类同名函数处理
    if(!(view->toModelPoint(x, y)))
    {
        QMainWindow::wheelEvent(event);
        return;
    }

    if(event->delta() > 0)
        view->zoomView(x, y, true);
    else
        view->zoomView(x, y, false);
}
