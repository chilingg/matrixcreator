#ifndef MATRIXCONTROLLER_H
#define MATRIXCONTROLLER_H

#include <QMainWindow>
#include <QMouseEvent>
//#include <QTimerEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QCursor>

class MatrixController : public QMainWindow
{
    Q_OBJECT

public:
    MatrixController(QWidget *parent = nullptr);

protected:

private:
    //CursorTool cursorTool;
};

enum CursorTool
{
    POINT,
    CIRCLE,
    TRANSLATE
};

#endif // MATRIXCONTROLLER_H
