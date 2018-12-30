#include "matrixcontroller.h"

MatrixController::MatrixController(QWidget *parent):
    QMainWindow(parent),
    model(2048, MatrixModel::LifeGameT),
    view(model, this),
    modelResume(false),
    viewResume(false),
    moveViewPos(),
    clickedPos(),
    selectRect(),
    cursorTool(POINT),
    lastCursorTool(cursorTool),
    circleCursor(QPixmap(":/cursor/circleSelect"), 0, 0),
    pointCursor(QPixmap(":/cursor/pointSelect"), 0, 0),
    translateCursor(QPixmap(":/cursor/zoomView"), 8, 8)
{
    setWindowTitle(tr("MatrixCreator"));
    resize(840, 720);//默认大小
    setWindowState(Qt::WindowMaximized);//默认最大化
    setCentralWidget(&view);

    //每秒24帧
    startTimer(1000/24);

    //默认点选工具
    setCursor(pointCursor);
}

void MatrixController::timerEvent(QTimerEvent *)
{
    if(modelResume)
    {
        (model.*(model.updateModel))();
        view.update();
    }
    else if(viewResume)
    {
        view.noRedrawUnits();
        view.update();
    }
}

void MatrixController::mousePressEvent(QMouseEvent *event)
{
    MPoint viewPos = view.inView(event->pos());

    //查看点击是否发生在视图中
    if(viewPos.valid)
    {
        if(event->button() == Qt::LeftButton)
        {

        }
        else
        {
            clearSelectBox();
        }
    }
    else
    {
        clearSelectBox();
    }
}
