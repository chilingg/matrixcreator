#include "matrixcontroller.h"
#include <QFileDialog>

MatrixController::MatrixController(QWidget *parent):
    QMainWindow(parent),
    model(4000, MatrixModel::LifeGameTRC),
    view(model, this),
    modelResume(false),
    moveViewPos(),
    selectPos(),
    clickedPos{false,0,0,QPoint(),QRect()},
    dfv1(0),
    dfv2(0),
    defaultValue(dfv1),
    cursorTool(CIRCLE),
    lastCursorTool(CIRCLE),
    circleCursor(QPixmap(":/cursor/circle"), 0, 0),
    pointCursor(QPixmap(":/cursor/point"), 0, 0),
    translateCursor(QPixmap(":/cursor/translate"), 8, 8),
    mStatusBar(statusBar()),
    mInfoLabel(new QLabel()),
    generation(0),
    unitInfo("XY = %1,%2    value = %3"),
    mInfo(" Generation = %1    Scale = 1:%2    Value = %3/%4    Threads = %5  "),
    THREADS(model.getThreads()),
    toolBar(new QToolBar("Tools", this)),
    circleTool(new QAction(QIcon(":/tool/circel"), tr("&Circle Tool"), this)),
    pointTool(new QAction(QIcon(":/tool/point"), tr("&Point Tool"), this)),
    translateTool(new QAction(QIcon(":/tool/translate"), tr("&Translate Tool"), this))
{
    setWindowTitle(tr("MatrixCreator"));
    resize(840, 720);//默认大小
    setWindowState(Qt::WindowMaximized);//默认最大化
    setCentralWidget(&view);

    selectPattern(model.getCurrentPattern());

    //每秒24帧
    startTimer(1000/24);

    //默认点选工具
    setCursorTool(POINT);

    //状态栏设置
    updateMatrixInfo();
    mStatusBar->addPermanentWidget(mInfoLabel);
    //设置工具栏背景色
    QPalette statusPal(palette());
    statusPal.setColor(QPalette::Background, MatrixColor::LUMINOSITY_2_68);
    statusPal.setColor(QPalette::WindowText, MatrixColor::LUMINOSITY_4_204);
    mStatusBar->setAutoFillBackground(true);
    mStatusBar->setPalette(statusPal);

    //工具栏设置
    addToolBar(Qt::LeftToolBarArea, toolBar);
    toolBar->setIconSize(QSize(24, 24));
    toolBar->addAction(circleTool);
    toolBar->addAction(pointTool);
    toolBar->addAction(translateTool);
    //设置工具栏背景色
    QPalette ToolPal(palette());
    ToolPal.setColor(QPalette::Background, MatrixColor::LUMINOSITY_3_136);
    toolBar->setAutoFillBackground(true);
    toolBar->setPalette(ToolPal);
}

void MatrixController::timerEvent(QTimerEvent *)
{
    if(modelResume)
    {
        (model.*(model.updateModel))();
        updateMatrixInfo();
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
        QString clickedInfo = unitInfo.arg(viewPos.modelColumn)
                .arg(viewPos.modelRow)
                .arg(model.getUnitValue(viewPos.modelColumn, viewPos.modelRow));
        mStatusBar->showMessage(clickedInfo, 4000);

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
                    view.translationView(0, beforePos.modelRow - viewPos.modelRow);
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
        view.overRangeLineOff();
        view.update();
    }

    if(event->button() == Qt::LeftButton)
    {
        if(!moveViewPos.isNull())
            moveViewPos = QPoint();

        clickedPos.valid = false;

        if(selectPos.isValid())
            selectPos = QRect();

        if(cursorTool == TRANSLATE)
        {
            view.overRangeLineOff();
            view.update();
        }
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
        setCursorTool(TRANSLATE);
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
        setCursorTool(POINT);
        return;
    }
    //V 框选工具
    if(event->key() == Qt::Key_V)
    {
        setCursorTool(CIRCLE);
        return;
    }
    //X 交换两个默认值
    if(event->key() == Qt::Key_X)
    {
        defaultValue = dfv2;
        dfv2 = dfv1;
        dfv1 = defaultValue;
        updateMatrixInfo();

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
        updateMatrixInfo();
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

void MatrixController::selectPattern(MatrixModel::ModelPattern p)
{
    switch(p)
    {
    case MatrixModel::LifeGameTSF:
        dfv1 = 1;
        dfv2 = 0;
        defaultValue = dfv1;
        break;
    case MatrixModel::LifeGameCCL:
        dfv1 = 3;
        dfv2 = 0;
        defaultValue = dfv1;
        break;
    case MatrixModel::LifeGameTRC:
        dfv1 = 3;
        dfv2 = 0;
        defaultValue = dfv1;
        break;
    default:
        dfv1 = 0;
        dfv2 = 0;
        defaultValue = dfv1;
        break;
    }
}


void MatrixController::setCursorTool(MatrixController::CursorTool tool)
{
    lastCursorTool = cursorTool;
    cursorTool = tool;

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
#ifndef M_NO_DEBUG
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: undefined cursor tool" << cursorTool;
#endif
        break;
    }
}
