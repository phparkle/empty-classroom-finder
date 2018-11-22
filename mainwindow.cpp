#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtNetwork>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    qnam(new QNetworkAccessManager)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
    delete qnam;
}
