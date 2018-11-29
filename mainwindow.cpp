#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "emptyclassroomfinder.h"
#include "httpclient.h"
#include "global.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    client(new HttpClient(this)),
    finder(new EmptyClassroomFinder(this))
{
    ui->setupUi(this);
    initUI();
    connect(client, &HttpClient::finished,
            finder, &EmptyClassroomFinder::parseHtml);
    if (!getHtmlDir().exists()) {
        const QString indexUrl = "https://w5.ab.ust.hk/wcq/cgi-bin/1810/";
        client->downloadAll(indexUrl);
    }
    else finder->parseHtml();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI(){

    QDateTime now = QDateTime::currentDateTime();
    qDebug() << now;

    QDate todayDate  = now.date();
    int toDay = todayDate.dayOfWeek() - 1;

    weekdayButtons[0] = ui->monButton;
    weekdayButtons[1] = ui->tueButton;
    weekdayButtons[2] = ui->wedButton;
    weekdayButtons[3] = ui->thuButton;
    weekdayButtons[4] = ui->friButton;

    QTime timeNow = now.time();

    for (QTime t = QTime(9,0); t != QTime(0, 0); t = t.addSecs(1800)){
        ui->startTimeCombo->addItem(t.toString("hh:mm AP"), t);
    }

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
    connect(ui->areaList, SIGNAL(itemSelectionChanged()),
            this, SLOT(handleAreaListSelectionChanged()));

    for (int i = 0; i<5; ++i)
        connect(weekdayButtons[i], &QPushButton::clicked,
                this , &MainWindow::handleInputChanged);

    // weekdayButtons[toDay]->setChecked(true);

    ui->areaList->setSortingEnabled(true);
    ui->roomList->setSortingEnabled(true);

    ui->statusBar->showMessage("Please select a weekday");

    // handleInputChanged();
}

int MainWindow::getCheckedButton() {
    for (int i = 0; i < 5; ++i)
        if (weekdayButtons[i]->isChecked())
            return i;
    return -1;
}

void MainWindow::handleInputChanged() {
    int weekday = getCheckedButton();
    if (weekday == -1)
        return;
    ui->startTimeCombo->setEnabled(true);
    ui->durationCombo->setEnabled(true);
    int startTimeIndex = ui->startTimeCombo->currentIndex();
    QTime startTime = ui->startTimeCombo->itemData(startTimeIndex).toTime();
    int durationIndex = ui->durationCombo->currentIndex();
    QTime endTime = startTime.addSecs((durationIndex + 1) * 30 * 60);
    QSet<const Address*> rooms = finder->findEmptyRooms(weekday, startTime, endTime);
    ui->statusBar->showMessage(QString("%1 empty classrooms found").arg(rooms.size()));
    currRooms.clear();
    for (const Address* addr : rooms)
        currRooms[addr->getArea()].append(addr->getRoom());
    ui->areaList->clear();
    for (auto it = currRooms.constBegin(); it != currRooms.end(); ++it) {
        QString format = "%1 (%2)";
        QString area = it.key();
        int count = it.value().size();
        ui->areaList->addItem(format.arg(area, count));
    }
    ui->areaList->addItems(currRooms.keys());
    ui->roomList->clear();
}

void MainWindow::handleAreaListSelectionChanged() {
    QString currArea = ui->areaList->currentItem()->text();
    if (currArea.isEmpty())
        return;
    int paren = currArea.lastIndexOf('(');
    QString key = currArea.left(paren - 1);
    ui->roomList->clear();
    ui->roomList->addItems(currRooms[key]);
}
