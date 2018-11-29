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

    QPushButton* weekdayButton[] = { ui->monButton, ui->tueButton, ui->wedButton, ui->thuButton, ui->friButton };
    weekdayButton[toDay]->setDown(true);

    QTime timeNow = now.time();

    for (QTime t = QTime(9,0); t != QTime(0, 0); t = t.addSecs(1800)){
        ui->startTimeCombo->addItem(t.toString("hh:mm AP"));
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

    for (int i = 0; i<5; ++i)
        connect(weekdayButton[i], &QPushButton::pressed,
                this , &MainWindow::handleInputChanged);


}

void MainWindow::handleInputChanged() {

}
