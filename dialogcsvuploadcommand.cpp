#include "dialogcsvuploadcommand.h"
#include "ui_dialogcsvuploadcommand.h"

#include <QSettings>

DialogCsvUploadCommand::DialogCsvUploadCommand(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCsvUploadCommand)
{
    ui->setupUi(this);

	QSettings set;

	set.beginGroup("CSV_Upload");
	ui->lineEdit_command->setText(set.value("command","").toString());
	ui->lineEdit_parameters->setText(set.value("parameters","").toString());
	set.endGroup();
}

DialogCsvUploadCommand::~DialogCsvUploadCommand()
{

	QSettings set;

	set.beginGroup("CSV_Upload");
	set.setValue("command", ui->lineEdit_command->text());
	set.setValue("parameters", ui->lineEdit_parameters->text());
	set.endGroup();

    delete ui;
}

QString DialogCsvUploadCommand::commandText()
{
    return ui->lineEdit_command->text();
}

QStringList DialogCsvUploadCommand::parameters()
{
    return ui->lineEdit_parameters->text().split(' ');
}
