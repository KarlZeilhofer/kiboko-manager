#include "dialogeditnames.h"
#include "ui_dialogeditnames.h"
#include "rundata.h"

#include <QAbstractButton>

DialogEditNames::DialogEditNames(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditNames)
{
    ui->setupUi(this);
	setModal(true);
}

DialogEditNames::~DialogEditNames()
{
    delete ui;
}

void DialogEditNames::setRunData(RunData* run)
{
	this->run = run;

	ui->lineEdit_title1->setText(run->getTitle1());
	ui->lineEdit_firstName1->setText(run->getFirstName1());
	ui->lineEdit_lastName1->setText(run->getLastName1());

	ui->lineEdit_title2->setText(run->getTitle2());
	ui->lineEdit_firstName2->setText(run->getFirstName2());
	ui->lineEdit_lastName2->setText(run->getLastName2());
}

void DialogEditNames::on_buttonBox_accepted()
{
	run->setTitle1(ui->lineEdit_title1->text());
	run->setFirstName1(ui->lineEdit_firstName1->text());
	run->setLastName1(ui->lineEdit_lastName1->text());

	run->setTitle2(ui->lineEdit_title2->text());
	run->setFirstName2(ui->lineEdit_firstName2->text());
	run->setLastName2(ui->lineEdit_lastName2->text());

	emit namesEdited(run);
	emit accepted();
	hide();
}

void DialogEditNames::on_buttonBox_clicked(QAbstractButton *button)
{
	if(ui->buttonBox->standardButton(button)==QDialogButtonBox::Reset)
	{
		// reset was pressed
		ui->lineEdit_title1->clear();
		ui->lineEdit_firstName1->clear();
		ui->lineEdit_lastName1->clear();
		ui->lineEdit_title2->clear();
		ui->lineEdit_firstName2->clear();
		ui->lineEdit_lastName2->clear();
	}
}
