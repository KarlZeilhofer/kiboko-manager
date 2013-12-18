#include "mainwindow.h"
#include "competition.h"
#include "timestamp.h"
#include "formtimestamplist.h"
#include "ui_formtimestamplist.h"

FormTimeStampList::FormTimeStampList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTimeStampList)
{
	setWindowFlags(Qt::Window);
    ui->setupUi(this);

	QTableWidget* t = ui->tableWidget;

	t->setColumnCount(7);
	t->setRowCount(100);

	QStringList headers = QString("ID,Time,Source,BoatBoxID,BoatID,BoatName,RunID").split(',');
	ui->tableWidget->setHorizontalHeaderLabels(headers);
	ui->tableWidget->horizontalHeader()->setVisible(true);

	regenerateTable();
}

void FormTimeStampList::regenerateTable()
{
	Competition* c = MainWindow::competition();

	int N = c->getNumOfTimeStamps();

	ui->tableWidget->setRowCount(0);
	ui->tableWidget->setRowCount(N+1); // keep last row empty

	for(int n=1; n<=N; n++)
	{
		updateTableRow(c->getTimeStamp(n));
	}
}

FormTimeStampList::~FormTimeStampList()
{
    delete ui;
}

void FormTimeStampList::updateTableRow(TimeStamp* ts)
{
	qDebug("FormTimeStampList::updateTableRow(TimeStamp* ts)");
	if(ts){
		int row = ts->getID()-1;

		QTableWidget* t = ui->tableWidget;

		if(row>= t->rowCount()){
			t->setRowCount(row+2); // increase table if necessary
		}

		t->setItem(row, 0, new QTableWidgetItem(QString::number(ts->getID())));
		t->setItem(row, 1, new QTableWidgetItem(ts->getTime().toString("HH:mm:ss.zz")));
		t->setItem(row, 2, new QTableWidgetItem(ts->getSourceName()));
		t->setItem(row, 3, new QTableWidgetItem(QString::number(ts->getBoatboxID())));
		t->setItem(row, 4, new QTableWidgetItem(QString::number(ts->getBoatID())));
		t->setItem(row, 5, new QTableWidgetItem(ts->getBoatName()));
		t->setItem(row, 6, new QTableWidgetItem(QString::number(ts->getRunID())));

		t->resizeColumnsToContents();
	}
}




void FormTimeStampList::on_pushButton_refresh_clicked()
{
	regenerateTable();
}
