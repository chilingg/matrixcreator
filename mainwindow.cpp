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
        if(!(view->toModelPoints(x, y)))
            return;

        //点击的单元转换为相反状态
        model->changeModelValue(x, y);
        view->update();

        //qDebug() << QPoint(x,y) << "Clicked x & y";
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

    QPoint beforeViewOffset = view->getViewOffset();//缩放之前的视图偏差
    qDebug() << beforeViewOffset << "Before";

    if(event->delta() > 0)
        view->zoomView(clickedX, clickedY, true);//缩小
    else
        view->zoomView(clickedX, clickedY, false);//放大

    QPoint afterViewOffset = view->getViewOffset();//缩放之后的视图偏差
    qDebug() << afterViewOffset << "After";

    qDebug() << cursor().pos() << "After";
    cursor().setPos(event->globalPos() + beforeViewOffset - afterViewOffset);
    qDebug() << cursor().pos() << "After";
    qDebug() << event->globalPos() << beforeViewOffset << afterViewOffset << "cursor";
}
