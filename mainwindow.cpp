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

    startTimer(1000/4);
    start = false;
}

MainWindow::~MainWindow()
{

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && start == false)
    {
        //把点击的坐标转换到view坐标系
        int x = event->pos().x() - view->getViewOffsetX();
        int y = event->pos().y() - view->getViewOffsetY();

        //view坐标转换到model坐标, 若不在view范围内则退出函数
        if(!(view->pointViewToModel(x, y)))
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
