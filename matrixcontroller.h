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

#ifndef M_NO_DEBUG
#include <QDebug>
#endif

class MatrixController : public QMainWindow
{
    Q_OBJECT

public:
    enum CursorTool{ POINT, CIRCLE, TRANSLATE };
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

    MatrixModel model;
    MatrixView view;
    bool modelResume;

    QPoint moveViewPos;
    QRect selectPos;
    MPoint clickedPos;
    int dfv1;
    int dfv2;
    int defaultValue;

    CursorTool cursorTool;
    CursorTool lastCursorTool;
    QCursor circleCursor;
    QCursor pointCursor;
    QCursor translateCursor;
};

inline void MatrixController::clearSelectBox()
{
    if(view.getSelectViewRect().isValid())
    {
        view.selectUnits(QRect());
    }
}


#endif // MATRIXCONTROLLER_H
