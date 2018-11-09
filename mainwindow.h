#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QWheelEvent>

#include "matrixmodel.h"
#include "matrixview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    MatrixModel *model;
    MatrixView *view;
    bool start;
};

#endif // MAINWINDOW_H
