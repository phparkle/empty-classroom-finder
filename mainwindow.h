#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <hcxselect/hcxselect.h>

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class HttpClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    void parseHtml();

    Ui::MainWindow* const ui;
    HttpClient* const client;
};

#endif // MAINWINDOW_H
