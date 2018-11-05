#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      model(new MatrixModel()),
      view(new MatrixView(this))
{
    setWindowTitle(tr("MatrixCreator"));
    resize(840, 840);
    setCentralWidget(view);
}

MainWindow::~MainWindow()
{

}
