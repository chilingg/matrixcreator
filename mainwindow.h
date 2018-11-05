#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>

#include "matrixmodel.h"
#include "matrixview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    MatrixModel *model;
    MatrixView *view;
};

#endif // MAINWINDOW_H
