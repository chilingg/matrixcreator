#include "matrixcontroller.h"
#include <QFileDialog>

MatrixController::MatrixController(QWidget *parent):
    QMainWindow(parent),
    model(2000, MatrixModel::LifeGame),
    view(model, this),
    modelResume(false),
    moveViewPos(),
    selectPos(),
    clickedPos{false,0,0,QPoint(),QRect()},
    dfv1(3),
    dfv2(0),
    defaultValue(dfv1),
    cursorTool(POINT),
    lastCursorTool(CIRCLE),
    circleCursor(QPixmap(":/cursor/circle"), 0, 0),
    pointCursor(QPixmap(":/cursor/point"), 0, 0),
    translateCursor(QPixmap(":/cursor/translate"), 8, 8)
{
    setWindowTitle(tr("MatrixCreator"));
    resize(840, 720);//默认大小
    setWindowState(Qt::WindowMaximized);//默认最大化
    setCentralWidget(&view);

    //每秒24帧
    startTimer(1000/24);

    //默认点选工具
    setCursor(pointCursor);

    statusBar();
}

void MatrixController::timerEvent(QTimerEvent *)
{
    if(modelResume)
    {
        (model.*(model.updateModel))();
        view.update();
    }
}

void MatrixController::mousePressEvent(QMouseEvent *event)
{
    clearSelectBox();
    MPoint viewPos = view.inView(event->pos());

    //查看点击是否发生在视图中
    if(viewPos.valid)
    {
        if (event->button() == Qt::MidButton)
        {
            moveViewPos = event->pos();//点击中键记录当前坐标
            setCursor(translateCursor);
            return;
        }

        if(event->button() == Qt::LeftButton)
        {
            if(cursorTool == TRANSLATE)//平移
            {
                moveViewPos = event->pos();
                return;
            }
            else if(cursorTool == CIRCLE && !modelResume)//框选
            {
                selectPos = viewPos.viewRect;
                view.selectUnits(selectPos);
                view.update();
                return;
            }
            else if(cursorTool == POINT && !modelResume)//点选
            {
                clickedPos = viewPos;
                model.changeModelValue(viewPos.modelColumn, viewPos.modelRow, defaultValue);
                view.update();
                return;
            }
        }
    }
}

void MatrixController::mouseMoveEvent(QMouseEvent *event)
{
    MPoint viewPos = view.inView(event->pos());

    if(viewPos.valid)
    {
        if(event->buttons() == Qt::MiddleButton)
        {
            //从视图外移动进视图中
            if(moveViewPos.isNull())
            {
                moveViewPos = viewPos.clickted;
                return;
            }

            MPoint beforePos = view.inView(moveViewPos);

            //移动的坐标与记录的坐标不在一个基础单元以上时触发视图移动
            if(viewPos.modelColumn != beforePos.modelColumn)//Horizontal
            {
                view.translationView(beforePos.modelColumn - viewPos.modelColumn, 0);
                moveViewPos.setX(viewPos.clickted.x());
            }
            if(viewPos.modelRow != beforePos.modelRow)//Vertical
            {
                view.translationView(0, beforePos.modelRow - viewPos.modelRow);
                moveViewPos.setY(viewPos.clickted.y());
            }

            view.update();
            return;
        }
        else if(event->buttons() == Qt::LeftButton)
        {
            if(cursorTool == TRANSLATE)
            {
                //从视图外移动进视图中
                if(moveViewPos.isNull())
                {
                    moveViewPos = viewPos.clickted;
                    return;
                }

                MPoint beforePos = view.inView(moveViewPos);

                //移动的坐标与记录的坐标不在一个基础单元以上时触发视图移动
                if(viewPos.modelColumn != beforePos.modelColumn)//Horizontal
                {
                    view.translationView(beforePos.modelColumn - viewPos.modelColumn, 0);
                    moveViewPos.setX(viewPos.clickted.x());
                }
                if(viewPos.modelRow != beforePos.modelRow)//Vertical
                {
                    view.translationView(beforePos.modelRow - viewPos.modelRow, 0);
                    moveViewPos.setY(viewPos.clickted.y());
                }

                view.update();
                return;
            }
            else if(cursorTool == CIRCLE && modelResume == false)
            {
                QRect beforeRect = selectPos;//起始点

                //视图外移动进来
                if(beforeRect.isEmpty())
                {
                    view.selectUnits(viewPos.viewRect);
                    view.update();
                    return;
                }

                QRect afterPos = viewPos.viewRect;//终点
                if(beforeRect.x() < afterPos.x())//向右框选
                {
                    if(beforeRect.y() < afterPos.y())
                        beforeRect.setBottomRight(afterPos.bottomRight());//向下框选
                    else
                        beforeRect.setTopRight(afterPos.topRight());//向上框选
                }
                else//向左框选
                {
                    if(beforeRect.y() < afterPos.y())
                        beforeRect.setBottomLeft(afterPos.bottomLeft());//向下框选
                    else
                        beforeRect.setTopLeft(afterPos.topLeft());//向上框选
                }

                view.selectUnits(beforeRect);
                view.noRedrawUnits();
                view.update();
                return;
            }
            else if(cursorTool == POINT && modelResume == false)
            {
                //从视图外移动进视图中
                if(!clickedPos.valid)
                {
                    clickedPos = viewPos;
                    return;
                }

                if(clickedPos.modelRow != viewPos.modelRow ||
                        clickedPos.modelColumn != viewPos.modelColumn)
                {
                    model.changeModelValue(viewPos.modelColumn, viewPos.modelRow, defaultValue);
                    view.update();
                    clickedPos = viewPos;
                }
            }
        }
    }
}

void MatrixController::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton)
    {
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

    if(event->button() == Qt::LeftButton)
    {
        moveViewPos = QPoint();
        clickedPos.valid = false;
        selectPos = QRect();
    }
}

void MatrixController::keyPressEvent(QKeyEvent *event)
{
    //ctrl+' 网格开关
    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Apostrophe)
    {
        view.gridOnOff();
        view.update();
        return;
    }
    //ctrl+; 参考线开关
    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Semicolon)
    {
        view.referenceLineOnOff();
        view.update();
        return;
    }
    //Tab 帧率显示开关
    if(event->key() == Qt::Key_Tab)
    {
        view.fpsOnOff();
        view.update();
        return;
    }
    //+ 放大
    if(event->key() == Qt::Key_Plus)
    {
        clearSelectBox();
        view.zoomView(view.inView(QPoint(width()/2, height()/2)), MatrixView::ZoomIn);
        view.update();
        return;
    }
    //- 缩小
    if(event->key() == Qt::Key_Minus)
    {
        clearSelectBox();
        view.zoomView(view.inView(QPoint(width()/2, height()/2)), MatrixView::ZoomOut);
        view.update();
        return;
    }
    //Z 平移工具
    if(event->key() == Qt::Key_Z)
    {
        lastCursorTool = cursorTool;
        cursorTool = TRANSLATE;
        setCursor(translateCursor);
        return;
    }
    //空格 开始暂停
    if(event->key() == Qt::Key_Space)
    {
        clearSelectBox();
        modelResume = !modelResume;
        return;
    }

    //模型运行时不处理以下事件
    if(modelResume)
        return;

    //键盘事件发生时，先处理选择对象
    if (view.getSelectViewRect().isValid())
    {
        //shift+F5 填充默认值
        QRect selectArea = view.getSelectUnitRect();
        if(event->modifiers() == Qt::ShiftModifier && event->key() == Qt::Key_F5)
        {
            for(int i = selectArea.left(); i <= selectArea.right(); ++i)
            {
                for(int j = selectArea.top(); j <= selectArea.bottom(); ++j)
                {
                    model.changeModelValue(i, j, defaultValue);
                }
            }
            view.selectUnits(QRect());//之后清除选框
            view.update();
            return;
        }
        //delete 删除选中
        else if(event->key() == Qt::Key_Delete)
        {
            model.clearUnit(selectArea.left(), selectArea.top(), selectArea.width(), selectArea.height());
            view.update();
            return;
        }
    }

    //F12 拍照
    if(event->key() == Qt::Key_F12)
    {
        QString s = QFileDialog::getSaveFileName(this,
                                                 "Image Save As",
                                                 "./",
                                                 "*.png");
        if(!s.isEmpty())
            //view.takePicture(s);

        clearSelectBox();
        return;
    }

    //不是shift键都会清除选框
    if(event->modifiers() != Qt::ShiftModifier)
    {
        clearSelectBox();//之后清除选框
        view.update();
    }

    //ctrl 暂切选择工具
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
    //A 点选工具
    if(event->key() == Qt::Key_A)
    {
        lastCursorTool = cursorTool;
        cursorTool = POINT;
        setCursor(pointCursor);
        return;
    }
    //V 框选工具
    if(event->key() == Qt::Key_V)
    {
        lastCursorTool = cursorTool;
        cursorTool = CIRCLE;
        setCursor(circleCursor);
        return;
    }
    //X 交换两个默认值
    if(event->key() == Qt::Key_X)
    {
        if(defaultValue == dfv1)
            defaultValue = dfv1;
        else
            defaultValue = dfv2;

        return;
    }
    //ctrl+delete 清除全部
    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Delete)
    {
        model.clearAllUnit();
        view.update();
        return;
    }
    //模型居中
    if(event->key() == Qt::Key_F4)
    {
        view.moveToCoordinate();
        view.update();
        return;
    }
    //→ 单步前进
    if(event->key() == Qt::Key_Right)
    {
        (model.*(model.updateModel))();
        view.update();
        return;
    }
}

void MatrixController::keyReleaseEvent(QKeyEvent *event)
{
    //模型运行时不处理以下事件
    if(modelResume)
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
        return;
    }
}

void MatrixController::wheelEvent(QWheelEvent *event)
{
    clearSelectBox();

    MPoint viewPos = view.inView(event->pos());
    if(viewPos.valid)
    {
        if(event->delta() > 0)
            view.zoomView(viewPos, MatrixView::ZoomIn);//放大
        else
            view.zoomView(viewPos, MatrixView::ZoomOut);//缩小

        //缩放后鼠标位置处于view中
        viewPos = view.inView(event->pos());
        if(viewPos.valid)
        {
            //获取鼠标所处单元的中心点坐标和view与全局坐标的偏差值，并把光标移到该坐标
            QPoint unitCtrPos = viewPos.viewRect.center();
            QPoint offset = event->globalPos() - event->pos() + view.getViewOffsetPoint();
            cursor().setPos(unitCtrPos + offset);
        }
        view.update();
    }
    else
    {
        QMainWindow::wheelEvent(event);
        return;
    }
}
