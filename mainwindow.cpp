#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "emptyclassroomfinder.h"
#include "httpclient.h"
#include "global.h"

#include <QtConcurrent>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    client(new HttpClient(this)),
    finder(new EmptyClassroomFinder(this))
{
    ui->setupUi(this);
    initUI();

    connect(this, &MainWindow::statusChanged,
            this, &MainWindow::handleStatusChanged);
    connect(client, &HttpClient::statusChanged,
            this, &MainWindow::handleStatusChanged);
    connect(finder, &EmptyClassroomFinder::statusChanged,
            this, &MainWindow::handleStatusChanged);

    connect(client, &HttpClient::finished,
            this, &MainWindow::processHtml);
    connect(finder, &EmptyClassroomFinder::finished,
            this, &MainWindow::handleFinderFinished);

    client->downloadAll();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI(){
    QSize desktop = QDesktopWidget().availableGeometry(this).size();
    resize(desktop.width() * 4 / 10, desktop.height() * 6 / 10);
    setWindowTitle("HKUST Empty Classroom Finder");

    weekdayButtons[0] = ui->monButton;
    weekdayButtons[1] = ui->tueButton;
    weekdayButtons[2] = ui->wedButton;
    weekdayButtons[3] = ui->thuButton;
    weekdayButtons[4] = ui->friButton;

    for (QTime t = QTime(9,0); t != QTime(0, 0); t = t.addSecs(1800))
        ui->startTimeCombo->addItem(t.toString("hh:mm AP"), t);

    ui->durationCombo->addItem("30 minutes");
    ui->durationCombo->addItem("1 hour");
    ui->durationCombo->addItem("1 hour 30 minutes");
    for (int i = 2; i <= 12; ++i) {
        QString item = QString("%1 hours").arg(i);
        ui->durationCombo->addItem(item);
        item.append(" 30 minutes");
        ui->durationCombo->addItem(item);
    }

    connect(ui->startTimeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(handleInputChanged()));
    connect(ui->durationCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(handleInputChanged()));
    connect(ui->areaList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(handleAreaListCurrentItemChanged(QListWidgetItem*)));
    for (int i = 0; i < 5; ++i)
        connect(weekdayButtons[i], &QPushButton::clicked,
                this , &MainWindow::handleInputChanged);
}

int MainWindow::getCheckedButton() {
    for (int i = 0; i < 5; ++i)
        if (weekdayButtons[i]->isChecked())
            return i;
    return -1;
}

void MainWindow::processHtml() {
    QtConcurrent::run(finder, &EmptyClassroomFinder::processHtml);
}

void MainWindow::handleFinderFinished() {
    for (QPushButton* button : weekdayButtons)
        button->setEnabled(true);
    emit statusChanged("Select a weekday");
}

void MainWindow::handleInputChanged() {
    int weekday = getCheckedButton();
    if (weekday == -1)
        return;
    ui->startTimeCombo->setEnabled(true);
    ui->durationCombo->setEnabled(true);
    ui->areaList->setEnabled(true);
    ui->roomList->setEnabled(true);
    int startTimeIndex = ui->startTimeCombo->currentIndex();
    QTime startTime = ui->startTimeCombo->itemData(startTimeIndex).toTime();
    int durationIndex = ui->durationCombo->currentIndex();
    QTime endTime = startTime.addSecs((durationIndex + 1) * 30 * 60);
    QSet<const Address*> rooms = finder->findEmptyRooms(weekday, startTime, endTime);
    handleStatusChanged(QString("%1 empty classrooms found").arg(rooms.size()));
    currRooms.clear();
    for (const Address* addr : rooms)
        currRooms[addr->getArea()].append(addr->getRoom());
    ui->areaList->clear();
    for (auto it = currRooms.constBegin(); it != currRooms.constEnd(); ++it) {
        const QString format = "%1 (%2)";
        const QString area = it.key();
        const int count = it.value().size();
        const QString text = format.arg(area).arg(count);
        QListWidgetItem* item = new QListWidgetItem(text, ui->areaList);
        item->setData(Qt::UserRole, area);
    }
    ui->roomList->clear();
}

void MainWindow::handleAreaListCurrentItemChanged(QListWidgetItem* currItem) {
    if (!currItem)
        return;
    const QString currArea = currItem->data(Qt::UserRole).toString();
    ui->roomList->clear();
    ui->roomList->addItems(currRooms[currArea]);
}

void MainWindow::handleStatusChanged(QString message) {
    ui->statusBar->showMessage(message);
}
