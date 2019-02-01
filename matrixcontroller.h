#ifndef MATRIXCONTROLLER_H
#define MATRIXCONTROLLER_H

#include "matrixmodel.h"
#include "matrixview.h"
#include <QMainWindow>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QCursor>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QToolBar>

#ifndef M_NO_DEBUG
#include <QDebug>
#endif

class MatrixController : public QMainWindow
{
    Q_OBJECT

public:
    enum CursorTool{ POINT, CIRCLE, TRANSLATE, DEFAULT };
    MatrixController(QWidget *parent = nullptr);

protected:
    void timerEvent(QTimerEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void clearSelectBox();
    void updateMatrixInfo();
    void selectPattern(MatrixModel::ModelPattern p);
    void setCursorTool(CursorTool tool);
    QPoint getPosInCentralWidget(QMouseEvent *event);
    QPoint getPosInCentralWidget(QWheelEvent *event);

    MatrixModel model;
    MatrixView view;
    bool modelResume;
    unsigned pressKeys;//记录当前按键数量

    QPoint moveViewPos;
    QRect selectPos;
    MPoint clickedPos;

    //默认值与替换默认值
    int dfv1;
    int dfv2;
    int defaultValue;

    //鼠标工具
    CursorTool cursorTool;
    CursorTool lastCursorTool;
    QCursor circleCursor;
    QCursor pointCursor;
    QCursor translateCursor;

    //状态栏
    QStatusBar *mStatusBar;
    QLabel *mInfoLabel;
    long long unsigned generation;
    const QString unitInfo;
    const QString mInfo;
    const unsigned THREADS;

    //工具栏
    QToolBar *toolBar;
    QAction *pointTool;
    QAction *circleTool;
    QAction *translateTool;
};

inline void MatrixController::clearSelectBox()
{
    if(view.getSelectViewRect().isValid())
    {
        view.selectUnits(QRect());
    }
}

inline void MatrixController::updateMatrixInfo()
{
    mInfoLabel->setText(mInfo.arg(++generation)
                        .arg(view.getUnitSize())
                        .arg(dfv1)
                        .arg(dfv2)
                        .arg(THREADS));
}

inline QPoint MatrixController::getPosInCentralWidget(QMouseEvent *event)
{
    QPoint pos = event->pos();
    pos.setX(pos.x() - toolBar->width());
    return pos;
}

inline QPoint MatrixController::getPosInCentralWidget(QWheelEvent *event)
{
    QPoint pos = event->pos();
    pos.setX(pos.x() - toolBar->width());
    return pos;
}

#endif // MATRIXCONTROLLER_H
