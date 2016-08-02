#include "dialogtimestampgenerator.h"
#include "ui_dialogtimestampgenerator.h"
#include "timestamp.h"
#include "mainwindow.h"
#include "defs.h"

DialogTimeStampGenerator::DialogTimeStampGenerator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTimeStampGenerator)
{
	ui->setupUi(this);

	ui->timeEdit->setTime(MainWindow::app()->getTimeBaseTime());

	QStringList s = TimeStamp::sourceStrings;

	for(int n=0; n<s.size(); n++){
		ui->comboBox_source->addItem(s.at(n));
	}

	for(int n=0; n<N_BOATBOXES; n++){
		ui->comboBox_boatID->addItem(QString::number(n+1));
	}
}

DialogTimeStampGenerator::~DialogTimeStampGenerator()
{
    delete ui;
}

void DialogTimeStampGenerator::on_comboBox_source_currentIndexChanged(QString str)
{
	if(str.startsWith("TS")){
		ui->comboBox_boatID->setEnabled(true);
	}else{
		ui->comboBox_boatID->setEnabled(false);
	}
}

void DialogTimeStampGenerator::on_buttonBox_accepted()
{
	QTime time = ui->timeEdit->time();
	if(ui->checkBox_useCurrentTime->isChecked()){
		time = MainWindow::app()->getTimeBaseTime();
		ui->timeEdit->setTime(MainWindow::app()->getTimeBaseTime());
	}
	TimeStamp* ts = new TimeStamp(this, time, (TimeStamp::Source)(ui->comboBox_source->currentIndex()));
	if(ui->comboBox_source->currentText().startsWith("TS")){
		ts->setBoatboxID(ui->comboBox_boatID->currentIndex()+1);
	}

	emit timeStampCreated(ts);

}
