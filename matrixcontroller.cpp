#include <QDebug>
#include "matrixcontroller.h"

MatriController::MatriController(QWidget *parent)
    : QMainWindow(parent),
      model(new MatrixModel()),
      view(new MatrixView(model, this))
{
    setWindowTitle(tr("MatrixCreator"));
    resize(INIT_VIEW_WIDTH, INIT_VIEW_HEIGHT);
    setWindowState(Qt::WindowMaximized);
    setCentralWidget(view);
    //view->centerView();

    startTimer(1000/24);
    start = false;
    moveViewPos = QPoint();
    selectRect = QRect();
    clickedPos = QPoint();
    selectTool = POINT;
    circleCursor = QCursor(QPixmap(":/cursor/circleSelect"), 0, 0);
    pointCursor = QCursor(QPixmap(":/cursor/pointSelect"), 0, 0);

    setCursor(pointCursor);
}

MatriController::~MatriController()
{
    delete model;
    model = nullptr;
}

void MatriController::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << view->getModelPoint(event->pos());

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
            clickedPos = modelPoint;//记录刚被转换的模型坐标

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
    if(!(view->isInView(event->pos())))
        return;

    if(event->buttons() == Qt::MiddleButton)
    {
        QPoint point = event->pos();

        if(moveViewPos.isNull())
        {
            moveViewPos = event->pos();
        }

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
            view->notRedraw();
            view->update();
        }
        else if(selectTool == POINT)
        {
            QPoint pos = view->getModelPoint(event->pos());

            if(clickedPos.isNull())//??
                clickedPos = pos;

            if(clickedPos != pos)
            {
                model->changeModelValue(pos.x(), pos.y());
                view->update();
                clickedPos = pos;
                //考虑以一个临时区域显示并存储当前信息，鼠标释放后修改并入模型视图，用以解决延迟问题
            }
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

void MatriController::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton)
    {
        //qDebug() << "mouveViewPos = 0";
        moveViewPos = QPoint();
    }
}

void MatriController::timerEvent(QTimerEvent *)
{
    if(start == false)
        return;

    //qDebug() << "-->In the timer";
    model->updateModelThread();
    view->update();
}

void MatriController::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Semicolon)//ctrl+; 参考线开关
    {
        view->referenceLineOnOff();
        view->notRedraw();
        view->update();
    }

    if(start == true && event->key() != Qt::Key_Space)//开始状态下只接收空格
        return;

    if(event->key() == Qt::Key_Space)//空格 开始暂停
    {
        start = !start;
    }

    //键盘事件发生时，先处理选择对象
    if (selectRect.isValid())
    {
        if(event->modifiers() == Qt::ShiftModifier && event->key() == Qt::Key_F5)//shift+F5 反向选中
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
        else if(event->key() == Qt::Key_Delete)//delete 删除选中
        {
            QRect selectArea = view->getSelectedModelRect();
            model->clearModel(selectArea.left(), selectArea.top(), selectArea.width(), selectArea.height());
            //qDebug() << view->getSelectedModelRect();
            view->update();
        }
        else if(event->modifiers() != Qt::ShiftModifier)
        {
            selectRect = QRect();
            view->selectedUnits(selectRect);
            view->update();
        }
    }

    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Delete)//ctrl+delete 清除全部
    {
        model->clearAllModel();
        view->update();
    }

    if(event->key() == Qt::Key_Control)//ctrl 暂切选择工具
    {
        selectTool == POINT ? selectTool = CIRCLE : selectTool = POINT;
        selectTool == POINT ? setCursor(pointCursor) : setCursor(circleCursor);
    }
    if(event->key() == Qt::Key_A)//a 点选
    {
        selectTool = POINT;
        setCursor(pointCursor);
    }
    if(event->key() == Qt::Key_V)//v 框选
    {
        selectTool = CIRCLE;
        setCursor(circleCursor);
    }

    if(event->key() == Qt::Key_F4)
    {
        view->centerView();
        view->update();
    }

    if(event->key() == Qt::Key_Right)
    {
        model->updateModelThread();
        view->update();
    }
}

void MatriController::keyReleaseEvent(QKeyEvent *event)
{
    if(start)
        return;

    if(event->key() == Qt::Key_Control)
    {
        selectTool == POINT ? selectTool = CIRCLE : selectTool = POINT;
        selectTool == POINT ? setCursor(pointCursor) : setCursor(circleCursor);
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
