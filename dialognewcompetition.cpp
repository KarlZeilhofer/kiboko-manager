#include "dialognewcompetition.h"
#include "ui_dialognewcompetition.h"
#include <QDate>
#include <QTime>
#include <QToolTip>
#include <QFileInfo>
#include <QHostInfo>
#include "mainwindow.h"
#include "fdisk.h"

DialogNewCompetition::DialogNewCompetition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewCompetition)
{
    ui->setupUi(this);
	setModal(true);
    on_buttonBox_accepted();

	ui->lineEdit_colors->setText(tr("rot blau grün"));
	ui->plainTextEdit_description->appendPlainText(tr("Bewerbsvorlage mit 15 Zillen, 5 Ersatzzillen, den Farben rot blau und grün"));
	ui->plainTextEdit_description->appendPlainText(tr("Datum: ") + QDate::currentDate().toString(tr("dd.MM.yyyy")));
    ui->plainTextEdit_description->appendPlainText(tr("Uhrzeit: ") + MainWindow::app()->getTimeBaseTime().toString(tr("hh:mm:ss")));
    ui->plainTextEdit_description->appendPlainText(tr("Computername: ") + QHostInfo::localHostName());

	infoTimer = new QTimer(this);
}

DialogNewCompetition::~DialogNewCompetition()
{
    delete ui;
}

QString DialogNewCompetition::description()
{
	return myDescription;
}

int DialogNewCompetition::boatsPerRound()
{
    return myBoatsPerRound;
}

int DialogNewCompetition::startRunID()
{
    return myStartRunID;
}

QStringList DialogNewCompetition::colors()
{
	return myColors;
}

QString DialogNewCompetition::databaseName()
{
	return myDatabaseName;
}

// save values to the myXXX members
void DialogNewCompetition::on_buttonBox_accepted()
{
	myDescription = ui->plainTextEdit_description->toPlainText();
    myBoatsPerRound = ui->spinBox_boatsPerRound->value();
    myStartRunID = ui->spinBox_startAtRun->value();
	if(ui->checkBox_manualColor->isChecked()){
		myColors = ui->lineEdit_colors->text().split(QRegExp("\\W+"));
	}else{
		myColors.clear();
	}
	myDatabaseName = ui->lineEdit_databaseName->text();

	hide();
	emit accepted();
}

// restore the old values!
void DialogNewCompetition::on_buttonBox_rejected()
{
	ui->plainTextEdit_description->setPlainText(myDescription);
    ui->spinBox_boatsPerRound->setValue(myBoatsPerRound);
    ui->spinBox_startAtRun->setValue(myStartRunID);
	if(myColors.isEmpty()){
		ui->checkBox_manualColor->setChecked(false);
	}else{
		ui->lineEdit_colors->setText(myColors.join(" "));
	}
	myDatabaseName = ui->lineEdit_databaseName->text();

	hide();
	emit rejected();
}

// show some hints to the user, with tool-tip messages
// use e.g.: QToolTip::showText( ui->lineEdit_filename->mapToGlobal(QPoint( ui->lineEdit_filename->width(), 0 ) ), tr("Diese Datei existiert bereits!") );
void DialogNewCompetition::showInfos()
{

}

void DialogNewCompetition::on_spinBox_startAtRun_valueChanged(int runID)
{
    int bpr = ui->spinBox_boatsPerRound->value();
    if(runID!= 1)
	{
        ui->label_roundAndBoatInfo->setText(QString(tr("(Durchgang %1, Zille %2)")).arg((runID-1)/bpr + 1).arg((runID-1)%bpr + 1));
    }
	else
	{
		ui->label_roundAndBoatInfo->clear();
	}
}

void DialogNewCompetition::on_pushButton_testDatabase_clicked()
{
    Fdisk myFdisk;
	myFdisk.test(ui->lineEdit_databaseName->text());
}
