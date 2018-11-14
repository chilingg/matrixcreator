#include <QDebug>
#include <QTimer>
#include "matrixcontroller.h"

MatriController::MatriController(QWidget *parent)
    : QMainWindow(parent),
      model(new MatrixModel()),
      view(new MatrixView(model, this))
{
    setWindowTitle(tr("MatrixCreator"));
    //resize(INIT_VIEW_WIDTH, INIT_VIEW_HEIGHT);
    setWindowState(Qt::WindowMaximized);
    setCentralWidget(view);
    //view->centerView();

    startTimer(1000/10);
    start = false;
    moveViewPos = QPoint(0, 0);
    selectRect = QRect();
    selectTool = POINT;

    QTimer::singleShot(0, view, MatrixView::centerView);qDebug() << "-->In the timer";
}

MatriController::~MatriController()
{
    delete model;
    model = nullptr;
}

void MatriController::mousePressEvent(QMouseEvent *event)
{
    //若不在view范围内则退出
    if(!(view->isInView(event->pos())))
    {
        selectRect = QRect();
        view->selectedUnits(selectRect);
        view->update();
        return;
    }

    //只要点击事件发生且不是左键，就清除选择对象
    if(event->button() != Qt::LeftButton)
    {
        if (selectRect.isValid())
        {
            selectRect = QRect();
            view->selectedUnits(selectRect);
            view->update();
        }
    }else if(start == false)//左键且暂停状态
    {
        if(selectTool == CIRCLE)//框选
        {
            QPoint clickedPos = view->getUnitPoint(view->getModelPoint(event->pos()));
            QRect selected = view->getSelectedUnitRect();

            //框选状态下，单击发生在SelectBox内，则取消选框
            if(selectRect.isValid())
            {
                if(selected.contains(clickedPos) &&
                    selected.width() == view->getBaseUnitSize() &&
                    selected.height() == view->getBaseUnitSize())
                {
                    //qDebug() << selected << clickedPos << "Test selectRect";
                    selectRect = QRect();
                    view->selectedUnits(selectRect);
                    view->update();
                }
                else
                {
                    selectRect = view->getUnitRect(view->getModelPoint(event->pos()));//单击选择
                    view->selectedUnits(selectRect);
                    view->update();
                }
            }
            else
            {
                selectRect = view->getUnitRect(view->getModelPoint(event->pos()));//单击选择
                view->selectedUnits(selectRect);
                view->update();
            }
        }
        else if(selectTool == POINT)//点选
        {
            if (selectRect.isValid())
            {
                selectRect = QRect();
                view->selectedUnits(selectRect);
                view->update();
            }

            QPoint modelPoint = view->getModelPoint(event->pos());
            //点击的单元转换为相反状态
            model->changeModelValue(modelPoint.x(), modelPoint.y());
            view->update();
        }
    }

    if (event->button() == Qt::MidButton)
    {
        moveViewPos = event->pos();//点击中键记录当前坐标
    }
}

void MatriController::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::MiddleButton)
    {
        QPoint point = event->pos();

        //移动的坐标与记录的坐标相差一个基础单元以上时触发视图移动
        if(point.x() - moveViewPos.x() >= view->getBaseUnitSize() ||
                moveViewPos.x() - point.x() >= view->getBaseUnitSize())//Horizontal
        {
            view->moveView((moveViewPos.x() - point.x()) / view->getBaseUnitSize(), 0);
            moveViewPos.setX(point.x());
        }
        if(point.y() - moveViewPos.y() >= view->getBaseUnitSize() ||
                moveViewPos.y() - point.y() >= view->getBaseUnitSize())//Vertical
        {
            view->moveView(0, (moveViewPos.y() - point.y()) / view->getBaseUnitSize());
            moveViewPos.setY(point.y());
        }

        view->update();

        //qDebug() << point - movePos << view->getBaseUnitSize() << "Mouse move test";
    }

    if(event->buttons() == Qt::LeftButton && start == false)
    {
        if(!(view->isInView(event->pos())))
            return;

        if(selectTool == CIRCLE)
        {
            if(selectRect.isEmpty())
            {
                selectRect = view->getUnitRect(view->getModelPoint(event->pos()));//单击选择
                view->selectedUnits(selectRect);
                view->update();
            }

            QRect beforePos = selectRect;
            QRect afterPos = view->getUnitRect(view->getModelPoint(event->pos()));

            if(selectRect.x() < afterPos.x())
            {
                if(selectRect.y() < afterPos.y())
                    beforePos.setBottomRight(afterPos.bottomRight());//框选
                else
                    beforePos.setTopRight(afterPos.topRight());
            }
            else
            {
                if(selectRect.y() < afterPos.y())
                    beforePos.setBottomLeft(afterPos.bottomLeft());
                else
                    beforePos.setTopLeft(afterPos.topLeft());
            }

            view->selectedUnits(beforePos);
            view->update();
        }
    }
}

void MatriController::mouseDoubleClickEvent(QMouseEvent *event)
{
    //点击坐标转换到model坐标, 若不在view范围内则退出函数
    if(!(view->isInView(event->pos())))
        return;

    if (event->buttons() == Qt::LeftButton && start == false)
    {
        if(selectTool == POINT)//点选
        {
            QPoint modelPoint = view->getModelPoint(event->pos());
            //点击的单元转换为相反状态
            model->changeModelValue(modelPoint.x(), modelPoint.y());
            view->update();
        }
    }
}

void MatriController::timerEvent(QTimerEvent *)
{
    qDebug() << width() << height() << "Test initial view.";
    if(start == false)
        return;

    //qDebug() << "-->In the timer";
    model->updateModel();
    view->update();
}

void MatriController::keyPressEvent(QKeyEvent *event)
{
    //键盘事件发生时，先处理选择对象
    if (selectRect.isValid())
    {
        if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_F)//填充选框
        {
            QRect models = view->getSelectedModelRect();
            //qDebug() << models << "Fill Selected models.";

            for(int i = models.left(); i <= models.right(); ++i)
            {
                for(int j = models.top(); j <= models.bottom(); ++j)
                {
                    //qDebug() << i << j;
                    model->changeModelValue(i, j);
                }
            }
            view->update();
        }
        else if(event->key() == Qt::Key_Delete)
        {
            QRect selectArea = view->getSelectedModelRect();
            model->clearModel(selectArea.left(), selectArea.top(), selectArea.width(), selectArea.height());
            //qDebug() << view->getSelectedModelRect();
            view->update();
        }
        else if(event->modifiers() != Qt::ControlModifier)
        {
            selectRect = QRect();
            view->selectedUnits(selectRect);
            view->update();
        }
    }

    if(event->key() == Qt::Key_Space)
        start = !start;

    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Semicolon)
    {
        view->referenceLineOnOff();
        view->update();
    }

    if(event->modifiers() == Qt::ControlModifier)
    {
        selectTool = CIRCLE;
    }
}

void MatriController::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        selectTool = POINT;
    }
}

void MatriController::wheelEvent(QWheelEvent *event)
{
    //只要事件发生就清除选择对象
    if (selectRect.isValid())
    {
        selectRect = QRect();
        view->selectedUnits(selectRect);
        view->update();
    }

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
    QPoint unitPoint = view->getUnitCentralPoint(view->getModelPoint(clickedX, clickedY));
    QPoint offset = event->globalPos() - event->pos() + view->getViewOffsetPoint();
    cursor().setPos(unitPoint + offset);
    //qDebug() << event->globalPos() << event->pos() << view->getviewOffsetPoint() << offset << "G-P-V";
    //qDebug() << cursor().pos() << "After";
}
