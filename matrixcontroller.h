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
    bool viewResume;

    QPoint moveViewPos;
    QPoint clickedPos;
    QRect selectRect;

    CursorTool cursorTool;
    CursorTool lastCursorTool;
    QCursor circleCursor;
    QCursor pointCursor;
    QCursor translateCursor;
};

inline void MatrixController::clearSelectBox()
{
    if(selectRect.isValid())
    {
        selectRect = QRect();
        view.selectUnits(selectRect);
    }
}


#endif // MATRIXCONTROLLER_H
