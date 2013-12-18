#include "mainwindow.h"
#include "maintablewidget.h"
#include "timestampstablewidget.h"
#include "dialogvisiblecolumns.h"
#include <QHeaderView>
#include <algorithm>


MainTableWidget::MainTableWidget(QWidget *parent) :
	QTableWidget(parent)
{
	//dialogVisibleColumns.setParent(parent());
	dialogVisibleColumns.hide();
	dialogVisibleColumns.setParent(MainWindow::app(), Qt::Dialog);
	connect(&dialogVisibleColumns, SIGNAL(columnsChanged()), this, SLOT(regenerateTable()));
	
	
	//dialogEditNames.setParent(parent());
	dialogEditNames.hide();
    dialogEditNames.setParent(MainWindow::app(), Qt::Dialog);
	connect(&dialogEditNames, SIGNAL(namesEdited(RunData*)), this, SLOT(updateTableRow(RunData*)));
	
	
	//dialogEditTime.setParent(parent());
	dialogEditTime.hide();
    dialogEditTime.setParent(MainWindow::app(), Qt::Dialog);
	connect(&dialogEditTime, SIGNAL(timeEdited(RunData*)), this, SLOT(updateTableRow(RunData*)));
	
	
	QStringList columnNames = DialogVisibleColumns::getAllColumnNames();
	setColumnCount(columnNames.size());
    setRowCount(10);
	
	setHorizontalHeaderLabels(columnNames);
	horizontalHeader()->setVisible(true);
	verticalHeader()->setVisible(false);
	
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	
	resizeColumnsToContents();
	resizeRowsToContents();
	clearContents();
	
	
	QTableWidgetItem* item = new QTableWidgetItem();
	item->setFlags(
				//Qt::ItemIsEditable
				//Qt::ItemIsDragEnabled
				//Qt::ItemIsDropEnabled
				//| Qt::ItemIsEditable
				Qt::ItemIsEnabled
				| Qt::ItemIsSelectable
				//| Qt::ItemIsTristate
				| Qt::ItemIsUserCheckable
				);
	item->setCheckState(Qt::Unchecked); // enable checkbox
	setItem(0,0,item);
	
	connect(this, SIGNAL(cellChanged(int,int)), this, SLOT(processCellChanged(int,int)));
	connect(this, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(on_cellDoubleClicked(int,int)));
	connect(this, SIGNAL(currentCellChanged(int,int,int,int)),
			this, SLOT(sendSelectSignal(int, int, int, int)));
	
	updateTimer.start(1000);		// update runtimes every 1s
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateRunTimes()));
	setEditTriggers(QTableWidget::DoubleClicked);
	
	onlineMode=true;
	
	//setAlternatingRowColors(true); this does not work!?
}

// update all cells of a row in the main-table
void MainTableWidget::updateTableRow(RunData* run)
{	
	if(run){
		//qDebug("updateTableRow()");
		DialogVisibleColumns* d = &dialogVisibleColumns; // just an abbrevation
		QTableWidgetItem* item;
		
		for(int n=0; n<columnCount(); n++){
			if(d->columnType(n) == DialogVisibleColumns::CT_Run){
				item = new QTableWidgetItem(QString::number(run->getID()));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);// set non-editable
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Round){
				item = new QTableWidgetItem(QString::number(run->getRound()));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);// set non-editable
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Color){
				setItem(run->getID()-1, n, new QTableWidgetItem(run->getColor()));
			}else if(d->columnType(n) == DialogVisibleColumns::CT_BoatNumber){
				item = new QTableWidgetItem(QString::number(run->getBoatID()));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);// set non-editable
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Names){
				QString names;
				QString name1 = run->getTitle1() + " " + run->getFirstName1() + " " + run->getLastName1();
				QString name2 = run->getTitle2() + " " + run->getFirstName2() + " " + run->getLastName2();
				if(run->getTitle2().isEmpty() && run->getFirstName2().isEmpty() && run->getLastName2().isEmpty()){
					names = name1;
				}else{
					names = name1 + ", " + name2;
				}
				item = new QTableWidgetItem(names);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);// set non-editable (at least not in standard way)
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_FireBrigade){
				setItem(run->getID()-1, n, new QTableWidgetItem(run->getFireBrigade()));
			}else if(d->columnType(n) == DialogVisibleColumns::CT_ValuationClass){
				setItem(run->getID()-1, n, new QTableWidgetItem(run->getValuationClass()));
			}else if(d->columnType(n) == DialogVisibleColumns::CT_StartTime){
				// e.g. "15:37.63 (TSS)"
				QTime time;
				QString source;
				int bb;
				QString str;
				if(run->getStartTimeID()){
					time = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime();
					source = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getSourceName();
					bb = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getBoatboxID();
					if( source == "LS" || source == "LGL" || source == "LGR" || source == "E" || source == "M"){
						bb=0;
					}
					if(bb>0){
						str = MainWindow::convertTimeToString(time) + " (" + source + ",BB" + QString::number(bb) +")";
					}else{
						str = MainWindow::convertTimeToString(time) + " (" + source + ")";
					}
				}
				item = new QTableWidgetItem(str);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable); // set non-editable
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_GoalTime){
				// e.g. "15:42.89 (TSG1)"
				QTime time;
				QString source;
				QString str;
				int bb;
				if(run->getGoalTimeID()){
					time = MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getTime();
					source = MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getSourceName();
					bb = MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getBoatboxID();
					if( source == "LS" || source == "LGL" || source == "LGR" || source == "E" || source == "M"){
						bb=0;
					}
					if(bb>0){
						str = MainWindow::convertTimeToString(time) + " (" + source + ",BB" + QString::number(bb) +")";
					}else{
						str = MainWindow::convertTimeToString(time) + " (" + source + ")";
					}
				}
				item = new QTableWidgetItem(str);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable); // set non-editable
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_RunTime){
				QString str;
				if(run->getStartTimeID()!=0 && run->getGoalTimeID()!=0)	// only for finished runs
				{
					QTime startTime = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime();
					QTime goalTime = MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getTime();
					QTime runTime;
					runTime = runTime.addMSecs(startTime.msecsTo(goalTime)); // build difference
					str = MainWindow::convertTimeToString(runTime);

				}
				item = new QTableWidgetItem(str);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable); // set non-editable
				QFont font=item->font();
				font.setBold(true);
                font.setPointSize(font.pointSize()+2);
				item->setFont(font);
				setItem(run->getID()-1, n, item);

				/*
				QTime startTime;
				if(run->getStartTimeID()){
					startTime = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime();
				}
				QTime goalTime;
				if(run->getGoalTimeID()){
					goalTime = MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getTime();
				}
				if(goalTime.isNull()){
					goalTime=MainWindow::app()->getTimeBaseTime();
				}
				QTime runTime = QTime::fromString("00:00.000", "mm:ss.zzz");
				runTime = runTime.addMSecs(startTime.msecsTo(goalTime)); // build difference
				QString str;
				if(!startTime.isNull()){
					str = MainWindow::convertTimeToString(runTime);
				}
				item = new QTableWidgetItem(str);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable); // set non-editable
				setItem(run->getID()-1, n, item);
				*/
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Dsq){
				QTableWidgetItem* item = new QTableWidgetItem();
				item->setFlags( Qt::ItemIsEnabled
								| Qt::ItemIsSelectable
								| Qt::ItemIsUserCheckable);
				if(run->getDSQ()){// show checkbox
					item->setCheckState(Qt::Checked);
				}else{
					item->setCheckState(Qt::Unchecked);
				}
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Errors){
				setItem(run->getID()-1, n, new QTableWidgetItem(run->getErrors()));
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Published){
				QTableWidgetItem* item = new QTableWidgetItem();
				item->setFlags( Qt::ItemIsEnabled
								| Qt::ItemIsSelectable
								| Qt::ItemIsUserCheckable);
				if(run->getPublished()){// show checkbox
					item->setCheckState(Qt::Checked);
				}else{
					item->setCheckState(Qt::Unchecked);
				}
				setItem(run->getID()-1, n, item);
			}else if(d->columnType(n) == DialogVisibleColumns::CT_Notes){
				QTableWidgetItem* item = new QTableWidgetItem(run->getNotes());
				item->setFlags( Qt::ItemIsEnabled
							   | Qt::ItemIsSelectable
							   | Qt::ItemIsEditable);
				setItem(run->getID()-1, n, item);
			}
		}
        resizeColumnsToContents();
        resizeRowToContents(run->getID()-1);
    }
}

void MainTableWidget::on_cellDoubleClicked(int row, int col)
{
	RunData* run;
	int n = col;
	if(MainWindow::competition() && (run = MainWindow::competition()->getRun(row+1))){
		DialogVisibleColumns* d = &dialogVisibleColumns; // just an abbrevation
		//QTableWidgetItem* item = ui->tableWidget_main->item(row, col);
		
		
		if(d->columnType(n) == DialogVisibleColumns::CT_Names){
			dialogEditNames.setRunData(run);
			dialogEditNames.show();
            dialogEditNames.exec();
		}
		if(d->columnType(n) == DialogVisibleColumns::CT_StartTime){
			dialogEditTime.setRunData(run, DialogEditTime::TT_START);
			dialogEditTime.show();
            dialogEditTime.exec();
		}
		if(d->columnType(n) == DialogVisibleColumns::CT_GoalTime){
			dialogEditTime.setRunData(run, DialogEditTime::TT_GOAL);
			dialogEditTime.show();
            dialogEditTime.exec();
		}
	}// end if

}

// read settings from the dialogVisibleColumns, and update the table
void MainTableWidget::regenerateTable()
{
	//qDebug("regenerateTable()");
    int currentRowBackup = currentRow();
    int currentColumnBackup = currentColumn();

	clear();
	
	setColumnCount(dialogVisibleColumns.numberOfVisibleColumns());
	
	for(int n=0; n<columnCount(); n++){
		
		setHorizontalHeaderLabels(dialogVisibleColumns.nameList());
	}
	
    setRowCount(MainWindow::competition()->getNumOfRuns());
	for(int n=1; n<=MainWindow::competition()->getNumOfRuns(); n++){
		updateTableRow(MainWindow::competition()->getRun(n));
	}
	
	resizeColumnsToContents();
	resizeRowsToContents();
    setCurrentCell(currentRowBackup, currentColumnBackup);
}

// write changes from the table-widget into the data-structure
// is also called on programmatically changed cells
void MainTableWidget::processCellChanged(int row, int col)
{
    //qDebug("MainTableWidget::processCellChanged(int row, int col)");
	RunData* run;
	int n = col;
	if(MainWindow::competition() && (run = MainWindow::competition()->getRun(row+1))){
		DialogVisibleColumns* d = &dialogVisibleColumns; // just an abbrevation
		QTableWidgetItem* i = item(row, col);
		
		
		if(d->columnType(n) == DialogVisibleColumns::CT_Run){
			// not editable
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Round){
			// not editable
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Color){
			run->setColor(i->text());
		}else if(d->columnType(n) == DialogVisibleColumns::CT_BoatNumber){
			// not editable
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Names){
			// do nothing. is set uneditable. on doubleclick open a dialog.
		}else if(d->columnType(n) == DialogVisibleColumns::CT_FireBrigade){
			run->setFireBrigade(i->text());
		}else if(d->columnType(n) == DialogVisibleColumns::CT_ValuationClass){
			run->setValuationClass(i->text());
		}else if(d->columnType(n) == DialogVisibleColumns::CT_StartTime){
			if(run->getStartTimeID()){
				TimeStamp *ts = MainWindow::competition()->getTimeStamp(run->getStartTimeID());
				ts->setTime(QTime::fromString(i->text(), "mm:ss.zzz"));
			}else{
				//TODO: do nothing.
				// if a run hasn't a start-timeStamp, the user can't edit it.
			}
		}else if(d->columnType(n) == DialogVisibleColumns::CT_GoalTime){
			
		}else if(d->columnType(n) == DialogVisibleColumns::CT_RunTime){
			
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Dsq){
			if(i->checkState()==Qt::Checked){
				run->setDsq(true);
			}else{
				run->setDsq(false);
			}
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Errors){
			run->setErrors(i->text());
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Published){
			if(i->checkState()==Qt::Checked){
				run->setPublished(true);
			}else{
				run->setPublished(false);
			}
		}else if(d->columnType(n) == DialogVisibleColumns::CT_Notes){
			run->setNotes(i->text());
		}
	}// end if
}

// slot
// selects the line in the table, which contains the TS
void MainTableWidget::selectRun(TimeStamp* ts)
{
	if(ts){
		int runId = ts->getRunID();
		if(runId){
			selectRow(runId-1);
		}
	}
}

// selects the line in the table, which contains the TS
void MainTableWidget::selectRun(RunData* run)
{	
	if(run->getID()!=0)
	{
		selectRow(run->getID()-1);
	}
}

void MainTableWidget::showColumnsSettings()
{
	dialogVisibleColumns.show();
	dialogVisibleColumns.exec();
}

void MainTableWidget::sendSelectSignal(int row, int col, int lastRow, int lastCol)
{
	int runID = row+1;
	Competition* c = MainWindow::competition();
	RunData* run = c->getRun(runID);
	
	// if row or col changed
	if(row!=lastRow || col!=lastCol)
	{
		emit runSelectionChanged(run);
	}
	
	if(run!=0)
	{
		// select starttime
		if(run->getStartTimeID()!=0 &&
				(run->getGoalTimeID()==0 || dialogVisibleColumns.columnType(col)==DialogVisibleColumns::CT_StartTime))
		{
			TimeStamp* ts = c->getTimeStamp(run->getStartTimeID());
			emit selectTimeStamp(ts);
		}
		// select goaltime
		else if(run->getGoalTimeID()!=0 &&
				 (!(run->getStartTimeID()) || dialogVisibleColumns.columnType(col)==DialogVisibleColumns::CT_GoalTime))
		{
			TimeStamp* ts = c->getTimeStamp(run->getGoalTimeID());
			emit selectTimeStamp(ts);
		}
		else
		{
			//emit selectTimeStamp(0);
		}
	}
	else
	{
		emit selectTimeStamp(0);
	}
}


// overload this function, to ensure minimal column widths
void MainTableWidget::resizeColumnsToContents() // override function
{
	QTableWidget::resizeColumnsToContents(); // call super-class' function
	// check for minimal column widths:
	int i = dialogVisibleColumns.columnIndex(DialogVisibleColumns::CT_StartTime);
	setColumnWidth(i, std::max(125, columnWidth(i)));
	i = dialogVisibleColumns.columnIndex(DialogVisibleColumns::CT_GoalTime);
	setColumnWidth(i, std::max(125, columnWidth(i)));
	i = dialogVisibleColumns.columnIndex(DialogVisibleColumns::CT_Names);
	setColumnWidth(i, std::max(200, columnWidth(i)));
}


void MainTableWidget::updateRunTimes()
{
	QTableWidgetItem* item;
	int col=dialogVisibleColumns.columnIndex(DialogVisibleColumns::CT_RunTime);
	for(int ID=1; ID<=MainWindow::competition()->getNumOfRuns(); ID++)
	{
		RunData* run=MainWindow::competition()->getRun(ID);
		if(run->getStartTimeID()!=0 && run->getGoalTimeID()==0 && onlineMode)		// for all unfinished runs in online mode
		{
			QTime startTime=MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime();
			QTime runTime;
			runTime = runTime.addMSecs(startTime.msecsTo(MainWindow::app()->getTimeBaseTime())); // build difference
            QString str = MainWindow::convertTimeToString(runTime);
            str = str.left(str.length()-3);
            item = new QTableWidgetItem(str);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable); // set non-editable
			setItem(ID-1, col, item);
		}
	}
}

void MainTableWidget::setOnlineMode(bool online)
{
	onlineMode=online;
}
