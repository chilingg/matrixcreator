#include <QDebug>
#include "matrixcontroller.h"

MatriController::MatriController(QWidget *parent)
    : QMainWindow(parent),
      model(std::make_unique<MatrixModel>()),
      view(new MatrixView(model.get(), this))
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
    cursorTool = POINT;
    lastCursorTool = CIRCLE;
    circleCursor = QCursor(QPixmap(":/cursor/circleSelect"), 0, 0);
    pointCursor = QCursor(QPixmap(":/cursor/pointSelect"), 0, 0);
    translateCursor = QCursor(QPixmap(":/cursor/zoomView"), 8, 8);

    setCursor(pointCursor);
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
        if(cursorTool == CIRCLE)//框选
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
        else if(cursorTool == POINT)//点选
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
            model->setValue(modelPoint.x(), modelPoint.y(), !model->value(modelPoint.x(), modelPoint.y()));
            view->update();
        }
        else if(cursorTool == TRANSLATE)//点选
        {
            moveViewPos = event->pos();
        }
    }

    if (event->button() == Qt::MiddleButton)
    {
        moveViewPos = event->pos();//点击中键记录当前坐标
        setCursor(translateCursor);
    }
}

void MatriController::mouseMoveEvent(QMouseEvent *event)
{
    if(!(view->isInView(event->pos())))
        return;

    if(event->buttons() == Qt::MiddleButton)
    {
        QPoint point = event->pos();

        //从视图外移动进视图中
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
        if(cursorTool == CIRCLE)
        {
            //从视图外移动进视图中
            if(selectRect.isEmpty())
            {
                selectRect = view->getUnitRect(view->getModelPoint(event->pos()));
                view->selectedUnits(selectRect);
                view->update();
            }

            QRect beforePos = selectRect;//起始点
            QRect afterPos = view->getUnitRect(view->getModelPoint(event->pos()));//终点

            if(selectRect.x() < afterPos.x())//向右框选
            {
                if(selectRect.y() < afterPos.y())
                    beforePos.setBottomRight(afterPos.bottomRight());//向下框选
                else
                    beforePos.setTopRight(afterPos.topRight());//向上框选
            }
            else//向左框选
            {
                if(selectRect.y() < afterPos.y())
                    beforePos.setBottomLeft(afterPos.bottomLeft());//向下框选
                else
                    beforePos.setTopLeft(afterPos.topLeft());//向上框选
            }

            view->selectedUnits(beforePos);
            view->notRedraw();//不要重绘模型图像
            view->update();
        }
        else if(cursorTool == POINT)
        {
            QPoint pos = view->getModelPoint(event->pos());

            //从视图外移动进视图中
            if(clickedPos.isNull())
                clickedPos = pos;

            if(clickedPos != pos)
            {
                model->setValue(pos.x(), pos.y(), !model->value(pos.x(), pos.y()));
                view->update();
                clickedPos = pos;
                //考虑以一个临时区域显示并存储当前信息，鼠标释放后修改并入模型视图，用以解决延迟问题
            }
        }
        else if(cursorTool == TRANSLATE)
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
        if(cursorTool == POINT)//点选
        {
            QPoint modelPoint = view->getModelPoint(event->pos());
            //点击的单元转换为相反状态
            model->setValue(modelPoint.x(), modelPoint.y(), !model->value(modelPoint.x(), modelPoint.y()));
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
        switch (cursorTool)
        {
        case POINT:
            setCursor(pointCursor);
            break;
        case CIRCLE:
            setCursor(circleCursor);
            break;
        case TRANSLATE:
            setCursor(translateCursor);
            break;
        default:
            break;
        }
    }
}

void MatriController::timerEvent(QTimerEvent *)
{
    if(start)
    {
        model->updateInThread();
        view->update();
    }
    else if(view->currentStatus())
    {
        view->notRedraw();
        view->update();
    }

}

void MatriController::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << event->key();

    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Apostrophe)//ctrl+; 参考线开关
    {
        view->referenceLineOnOff();
        //view->notRedraw(); //不要更新一级参考线
        view->update();
    }

    if(event->key() == Qt::Key_Tab)//Tab 帧率显示开关
    {
        view->FPSDisplayOnOff();
        view->update();
    }

    if(event->key() == Qt::Key_Z)//平移
    {
        lastCursorTool = cursorTool;
        cursorTool = TRANSLATE;
        setCursor(translateCursor);
    }

    if(event->key() == Qt::Key_Plus)//放大
    {
        view->zoomView(width()/2, height()/2, true);
        view->update();
    }
    if(event->key() == Qt::Key_Minus)//缩小
    {
        view->zoomView(width()/2, height()/2, false);
        view->update();
    }

    if(start == true && event->key() != Qt::Key_Space)//开始状态下只接收空格
        return;

    if(event->key() == Qt::Key_Space)//空格 开始暂停
    {
        start = !start;
    }

    if(event->key() == Qt::Key_F12)//拍照
    {
        view->takePicture();
        view->startAnimation();
        return;
    }

    //键盘事件发生时，先处理选择对象
    if (selectRect.isValid())
    {
        if(event->modifiers() == Qt::ShiftModifier && event->key() == Qt::Key_F5)//shift+F5 反向选中
        {
            QRect models = view->getSelectedModelRect();
            //qDebug() << models << "Fill Selected models.";

            model->setRangeValue(models.left(), models.top(), models.width(), models.height(), 1);
            view->selectedUnits(QRect());//之后清除选框
            view->update();
        }
        else if(event->key() == Qt::Key_Delete)//delete 删除选中
        {
            QRect selectArea = view->getSelectedModelRect();
            model->setRangeValue(selectArea.left(), selectArea.top(), selectArea.width(), selectArea.height(), 0);
            //qDebug() << view->getSelectedModelRect();
            view->update();
        }
        else if(event->modifiers() != Qt::ShiftModifier)//不是shift键都会清除选框
        {
            selectRect = QRect();
            view->selectedUnits(selectRect);
            view->update();
        }
    }

    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Delete)//ctrl+delete 清除全部
    {
        model->clear();
        view->update();
    }

    if(event->key() == Qt::Key_Control)//ctrl 暂切选择工具
    {
        CursorTool temp = lastCursorTool;
        lastCursorTool = cursorTool;
        cursorTool = temp;

        switch (cursorTool)
        {
        case POINT:
            setCursor(pointCursor);
            break;
        case CIRCLE:
            setCursor(circleCursor);
            break;
        case TRANSLATE:
            setCursor(translateCursor);
            break;
        default:
            break;
        }
    }
    if(event->key() == Qt::Key_A)//a 点选
    {
        lastCursorTool = cursorTool;
        cursorTool = POINT;
        setCursor(pointCursor);
    }
    if(event->key() == Qt::Key_V)//v 框选
    {
        lastCursorTool = cursorTool;
        cursorTool = CIRCLE;
        setCursor(circleCursor);
    }

    if(event->key() == Qt::Key_F4)//居中
    {
        view->centerView();
        view->update();
    }

    if(event->key() == Qt::Key_Right)//单步前进
    {
        model->updateInThread();
        view->update();
    }
}

void MatriController::keyReleaseEvent(QKeyEvent *event)
{
    if(start)
        return;

    //松开ctrl后恢复原本鼠标工具
    if(event->key() == Qt::Key_Control)
    {
        CursorTool temp = lastCursorTool;
        lastCursorTool = cursorTool;
        cursorTool = temp;

        switch (cursorTool)
        {
        case POINT:
            setCursor(pointCursor);
            break;
        case CIRCLE:
            setCursor(circleCursor);
            break;
        case TRANSLATE:
            setCursor(translateCursor);
            break;
        default:
            break;
        }
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

    int clickedX = event->position().x();
    int clickedY = event->position().y();

    //滚动时指针的坐标转换到model坐标, 若不在view范围内则调用父类同名函数处理
    if(!(view->isInView(clickedX, clickedY)))
    {
        QMainWindow::wheelEvent(event);
        return;
    }
    
    if(event->angleDelta().y() > 0)
        view->zoomView(clickedX, clickedY, true);//缩小
    else
        view->zoomView(clickedX, clickedY, false);//放大

    //缩放后鼠标位置处于view中
    if(view->isInView(event->pos()))
    {
        //获取鼠标所处单元的中心点坐标和view与全局坐标的偏差值，并把光标移到该坐标
        QPoint unitPoint = view->getUnitCentralPoint(view->getModelPoint(clickedX, clickedY));
        QPoint offset = event->globalPos() - event->pos() + view->getViewOffsetPoint();
        cursor().setPos(unitPoint + offset);
        //qDebug() << event->globalPos() << event->pos() << view->getviewOffsetPoint() << offset << "G-P-V";
        //qDebug() << cursor().pos() << "After";
    }
}
