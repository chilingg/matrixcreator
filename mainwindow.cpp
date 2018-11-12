#include <QDebug>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      model(new MatrixModel()),
      view(new MatrixView(model, this))
{
    setWindowTitle(tr("MatrixCreator"));
    resize(INIT_VIEW_WIDTH, INIT_VIEW_HEIGHT);
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

        movePos = event->pos();//点击中键记录当前坐标
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::MiddleButton)
    {
        QPoint point = event->pos();

        //移动的坐标与记录的坐标相差一个基础单元以上时触发视图移动
        if(point.x() - movePos.x() >= view->getBaseUnitSize() ||
                movePos.x() - point.x() >= view->getBaseUnitSize())//Horizontal
        {
            view->moveView((movePos.x() - point.x()) / view->getBaseUnitSize(), 0);
            movePos.setX(point.x());
        }
        if(point.y() - movePos.y() >= view->getBaseUnitSize() ||
                movePos.y() - point.y() >= view->getBaseUnitSize())//Vertical
        {
            view->moveView(0, (movePos.y() - point.y()) / view->getBaseUnitSize());
            movePos.setY(point.y());
        }

        view->update();

        //qDebug() << point - movePos << view->getBaseUnitSize() << "Mouse move test";
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
