#include "dialogcompetitionproperties.h"
#include "ui_dialogcompetitionproperties.h"

#include "mainwindow.h"
#include "competition.h"

#include <QFileInfo>

DialogCompetitionProperties::DialogCompetitionProperties(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCompetitionProperties)
{
    ui->setupUi(this);

    Competition* c = MainWindow::app()->competition();
    MainWindow* m = MainWindow::app();

    if(c == 0)
        return;

    ui->lineEdit_fileName->setText(m->getCurrentFile().fileName());
    ui->lineEdit_path->setText(m->getCurrentFile().absolutePath());
    ui->lineEdit_fileSize->setText(QString::number(m->getCurrentFile().size()/1024)+"kB");

    ui->plainTextEdit->setPlainText(c->getDescription());

    ui->lineEdit_boatsPerRound->setText(QString::number(c->getBoatsPerRound()));
    ui->lineEdit_colors->setText(c->getColors().join(" "));

    ui->groupBox_statistics->setVisible(false);
	
	ui->lineEdit_databaseName->setText(c->getDatabaseName());

    // TODO: statistics
}

DialogCompetitionProperties::~DialogCompetitionProperties()
{
    delete ui;
}

void DialogCompetitionProperties::closeEvent(QCloseEvent* /*event*/)
{
	Competition* c = MainWindow::app()->competition();
	
	if(c!=0)
	{
		c->setDescription(ui->plainTextEdit->toPlainText());
		c->setDatabaseName(ui->lineEdit_databaseName->text());
	}
}
