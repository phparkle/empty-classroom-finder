#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>

namespace Ui {
class MainWindow;
}

class EmptyClassroomFinder;
class HttpClient;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleInputChanged();
    void handleAreaListSelectionChanged();

private:
    void initUI();
    void parseHtml();
    int getCheckedButton();

    Ui::MainWindow* const ui;
    HttpClient* const client;
    EmptyClassroomFinder* const finder;
    QPushButton* weekdayButtons[5];
    QHash<QString, QStringList> currRooms;
};

#endif // MAINWINDOW_H
