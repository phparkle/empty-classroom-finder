#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class EmptyClassroomFinder;
class HttpClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleInputChanged();

private:
    void initUI();
    void parseHtml();

    Ui::MainWindow* const ui;
    HttpClient* const client;
    EmptyClassroomFinder* const finder;
};

#endif // MAINWINDOW_H
