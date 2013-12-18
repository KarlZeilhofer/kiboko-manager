#include "dialogedittime.h"
#include "ui_dialogedittime.h"
#include "mainwindow.h"
#include "rundata.h"
#include "timestamp.h"
#include "competition.h"

DialogEditTime::DialogEditTime(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditTime)
{
    ui->setupUi(this);
	setModal(true);
}

DialogEditTime::~DialogEditTime()
{
    delete ui;
}

void DialogEditTime::setRunData(RunData* run, TimeType timeType)
{
	myTimeType = timeType;
	myRun = run;

	if(run){ // ignore runns with Nullpointer
		if(myTimeType == TT_START){
			ui->label->setText(tr("Startzeit:"));
			if(run->getStartTimeID() == 0){ // if no TS exists, use current time
				ui->lineEdit->setText(MainWindow::convertTimeToString(MainWindow::app()->getTimeBaseTime()));
			}else{
				ui->lineEdit->setText(MainWindow::convertTimeToString(MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime()));
			}
		}else{
			ui->label->setText(tr("Zielzeit:"));
			if(run->getGoalTimeID() == 0){ // if no TS exists, use current time
				ui->lineEdit->setText(MainWindow::convertTimeToString(MainWindow::app()->getTimeBaseTime()));
			}else{
				ui->lineEdit->setText(MainWindow::convertTimeToString(MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getTime()));
			}
		}
	}
}

void DialogEditTime::on_buttonBox_accepted()
{
    // workaround for missing leading zero bug: prepend a "0", if it is missing
    QString tStr = ui->lineEdit->text();
    QStringList helper = tStr.split(":");
    if(helper.first().length() == 1){
        tStr.prepend("0");
    }
    // end of workaround

    QTime t = QTime::fromString(tStr);
    qDebug() << "zeit: " << t.toString();

	TimeStamp* ts =0;

	if(myRun){// ignore runns with Nullpointer
		if(myTimeType == TT_START){
			if(myRun->getStartTimeID() == 0){ // if no TS exists, create a new one
				ts = new TimeStamp(0, t, TimeStamp::E);
				MainWindow::competition()->addTimeStamp(ts);
				ts->setRunID(myRun->getID());
				myRun->setStartTimeID(ts->getID());
			}else{
				ts = MainWindow::competition()->getTimeStamp(myRun->getStartTimeID());
			}
        }else{ // goal
			if(myRun->getGoalTimeID() == 0){ // if no TS exists, create a new one
				ts = new TimeStamp(0, t, TimeStamp::E);
				MainWindow::competition()->addTimeStamp(ts);
				ts->setRunID(myRun->getID());
				myRun->setGoalTimeID(ts->getID());
			}else{
				ts = MainWindow::competition()->getTimeStamp(myRun->getGoalTimeID());
			}
		}

		// if new time != old time
		// compare strings, because of sub-0.01s differences, which should be ignored
		if(MainWindow::convertTimeToString(ts->getTime()) != MainWindow::convertTimeToString(t)){
			ts->setTime(t);
			ts->setSource(TimeStamp::E);
			emit timeEdited(myRun);
		}
	}
	hide();
}

void DialogEditTime::on_buttonBox_rejected()
{
	hide();
}
