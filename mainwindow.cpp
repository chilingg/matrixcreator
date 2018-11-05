#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      view()
{
    setWindowTitle(tr("MatrixCreator"));

    setCentralWidget(view);
}

MainWindow::~MainWindow()
{

}
