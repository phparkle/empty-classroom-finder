// Filename: mainwindow.h
//
// The MainWindow class holds the GUI and coordinates user input.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>

namespace Ui {
class MainWindow;
}

class EmptyClassroomFinder;
class HttpClient;
class QListWidgetItem;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    // Emitted to signify a change of status.
    void statusChanged(QString message);

private slots:
    // Start processing HTML in a separate thread
    void processHtml();

    // Enables GUI widgets
    void handleFinderFinished();

    // Queries the finder and updates results
    void handleInputChanged();

    // Displays the rooms in the selected area
    void handleAreaListCurrentItemChanged(QListWidgetItem* current);

    // Shows the message in the status bar
    void handleStatusChanged(QString message);

private:
    // Initializes UI components
    void initUI();

    // Returns the currently selected weekday button, or -1 if none are selected
    int getCheckedButton();

    // The GUI
    Ui::MainWindow* const ui;

    // The HTTP client
    HttpClient* const client;

    // The empty classroom finder
    EmptyClassroomFinder* const finder;

    // Pointers to the weekday buttons
    QPushButton* weekdayButtons[5];

    // The data backing the list widgets
    QHash<QString, QStringList> currRooms;
};

#endif // MAINWINDOW_H
