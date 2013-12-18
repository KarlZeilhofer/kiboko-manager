#include "timestampstablewidget.h"
#include "timestamp.h"
#include "mainwindow.h"
#include <QHeaderView>
#include "competition.h"



/*
	Documetation:

	The TimeStampsTableWidget holds all unassigned timestamps.
    They are ordered by the timestamp time, the oldest is on top.
	To implement fast lookups in the list, hash maps (rowToId, idToRow) are used.
*/

// TODO: immer zum neuesten timestamp scrollen, wenn schon länger kein UI-input in der Liste war.
// oder evt. mit checkbox lösen.

TimeStampsTableWidget::TimeStampsTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
	setContextMenuPolicy(Qt::CustomContextMenu); // TODO: testen!
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));
	connect(this, SIGNAL(currentCellChanged(int,int,int,int)),
			this, SLOT(sendSelectSignal(int, int, int, int)));
	
	updateTimer.start(1000);	// update age times every 1s
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateAgeTimes()));
	onlineMode=true;
	
	connect(this, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(on_cellDoubleClicked(int,int)));
}

void TimeStampsTableWidget::init()
{
	setColumnCount(3);
	setRowCount(100);

	QStringList headers = QString("A. vor,Zeit,Quelle").split(',');
	setHorizontalHeaderLabels(headers);
	horizontalHeader()->setVisible(true);
	verticalHeader()->setVisible(false);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	regenerateList();
}

void TimeStampsTableWidget::updateTimeStamp(TimeStamp* ts)
{
	//qDebug(QString("TimeStampsTableWidget::updateTimeStamp(TimeStamp* ts)").toAscii());
	if(ts!=0){
		//qDebug(QString(", ID = %1").arg(ts->getID()).toAscii());

		//qDebug("to be removed?");
		if(ts->getRunID() != 0 && idToRow.contains(ts->getID())){ // remove it from the table and the indices
			//qDebug("    yes");
			int deleteRow = idToRow.value(ts->getID()); // must scceed, due to the if condition above
			//qDebug(QString("    deleteRow = %1").arg(deleteRow).toAscii());
			int tempSize = idToRow.size();

			for(int r=deleteRow+1; r<=tempSize; r++){
				moveRowUp(r);
			}

			printHashMaps();
			return;
		}else{
			//qDebug("    no");
		}

		//qDebug("to be inserted?");
		if(!idToRow.contains(ts->getID()) && ts->getRunID()==0){ // insert it first
			//qDebug("    yes");

			setRowCount(idToRow.size()+2);
			int r=0;
			while(rowToID.value(r, INT_MAX) < ts->getID() && r<(ts->getID()-1)){ // linear search // TODO: could be improved!
				r++;
			}

			int insertRow = r; // row starts with 0
			//qDebug(QString("    insertRow = %1").arg(insertRow).toAscii());

			// now r is the row, where the new time-stamp will be inserted.
			for(int r=rowToID.size()-1; r>=insertRow; r--){
				moveRowDown(r);
			}

			idToRow.insert(ts->getID(),insertRow);
			rowToID.insert(insertRow, ts->getID());

			resizeRowsToContents();
			resizeColumnsToContents();
		}else{
			//qDebug("    no");
		}

		//qDebug("to be updated?");
		if(idToRow.contains(ts->getID())){ // update it!
			//qDebug("    yes");
			int r = idToRow.value(ts->getID(), -1); // row

			// print strings:
			//setItem(r, 0, new QTableWidgetItem("xxx s"));
			setItem(r, 1, new QTableWidgetItem(MainWindow::convertTimeToString(ts->getTime())));

			QString source = ts->getSourceName();
			if(ts->getSourceName().startsWith("TS")){
				source += " (" + QString::number(ts->getBoatboxID()) + ")";
			}
			setItem(r, 2, new QTableWidgetItem(source));

			//qDebug(QString("rowToID.size() = %1").arg(rowToID.size()).toAscii());
			if(idToRow.size() == 1){ // mark first entry by default. // TODO: bug: after first entry, size=2
				setCurrentCell(0, 0); //
			}

			//qDebug(QString("currentCell(%1,%2)").arg(currentRow()).arg(currentColumn()).toAscii());

			resizeRowsToContents();
			resizeColumnsToContents();

			printHashMaps();
			return;
		}else{
			//qDebug("    no");
		}
	}	
	printHashMaps();
}

void TimeStampsTableWidget::printHashMaps()
{
#ifndef QT_NO_DEBUG_OUTPUT
	QHash<int,int>::const_iterator i;
	for (i = idToRow.constBegin(); i != idToRow.constEnd(); ++i){
		//qDebug(QString("idToRow(id=%1, row=%2)").arg(i.key()).arg(i.value()).toAscii());
	}
	for (i = rowToID.constBegin(); i != rowToID.constEnd(); ++i){
		//qDebug(QString("rowToID(row=%1, id=%2)").arg(i.key()).arg(i.value()).toAscii());
	}
#endif
}

void TimeStampsTableWidget::regenerateList()
{
	//qDebug("TimeStampsTableWidget::regenerateList()");

	Competition* c = MainWindow::competition();

	idToRow.clear();
	rowToID.clear();

	//qDebug(QString("    after clear: idToRow.size() = %1").arg(idToRow.size()).toAscii());
	int N = c->getNumOfTimeStamps();
	//qDebug(QString("    c->getNumOfTimeStamps() = %1").arg(N).toAscii());

	setRowCount(0); // clear rows
	setRowCount(N+1); // keep last row empty

	for(int n=1; n<=N; n++)
	{
		updateTimeStamp(c->getTimeStamp(n));
	}
}

void TimeStampsTableWidget::moveRowDown(int myRow)
{
	//qDebug("moveRowDown(r)");
	if(rowCount() < myRow+2){
		setRowCount(myRow+2);
	}

	//qDebug("    copy cells");
	for(int c=0; c<columnCount(); c++){
		if(item(myRow, c)!=0){
			if(item(myRow+1, c) == 0){
				setItem(myRow+1, c, new QTableWidgetItem(item(myRow, c)->text())); // create new item
			}else{
				item(myRow+1,c)->setText(item(myRow, c)->text()); // just change the lower item
			}
		}
	}

	//qDebug("    update hash maps");
	// save temporarily old values:
	int upperRow = myRow;
	int lowerRow = myRow+1;
	int upperID = rowToID.value(upperRow, 0);
	int lowerID = rowToID.value(lowerRow, 0);

	// remove outdated index-entries:
	idToRow.remove(lowerID);
	rowToID.remove(upperRow);

	// update new index entries:
	if(upperID != 0){
		rowToID.insert(lowerRow,upperID);
		idToRow.insert(upperID,lowerRow);
	}
}

void TimeStampsTableWidget::moveRowUp(int myRow){
	//qDebug(QString("TimeStampsTableWidget::moveRowUp(%1)").arg(myRow).toAscii());
	if(myRow > 0){
		for(int c=0; c<columnCount(); c++){
			if(item(myRow, c) != 0){ // if myRow if non-empty
				if(item(myRow-1, c) == 0){ // myRow-1 is empty
					setItem(myRow-1, c, new QTableWidgetItem(item(myRow, c)->text())); // create new item
				}else{ // myRow-1 is non-empty
					item(myRow-1,c)->setText(item(myRow, c)->text()); // just change the upper item
				}
				item(myRow, c)->setText(""); // clear moved row
			}else{ // myRow is empty
				setItem(myRow-1, c, new QTableWidgetItem("")); // create new empty item
			}
		}
		//qDebug("    update hash maps");

		// save temporarily old values:
		int upperRow = myRow-1;
		int lowerRow = myRow;
		int upperID = rowToID.value(upperRow, 0);
		int lowerID = rowToID.value(lowerRow, 0);

		// remove outdated index-entries:
		idToRow.remove(upperID);
		rowToID.remove(lowerRow);
		rowToID.remove(upperRow);

		// update new index entries:
		if(lowerID != 0){
			rowToID.insert(upperRow, lowerID);
			idToRow.insert(lowerID, upperRow);
		}
	}
}

int TimeStampsTableWidget::getSelectedID() // returns 0, if none selected
{
	return rowToID.value(currentRow(),0);
}

void TimeStampsTableWidget::on_customContextMenuRequested( const QPoint & pos )
{
    int hideWarning = pos.x(); hideWarning++;
	// TODO:
	//qDebug("TimeStampsTableWidget rechtsklick");
}

void TimeStampsTableWidget::sendSelectSignal(int row, int col, int lastRow, int lastCol)
{
    int hideWarning = col; hideWarning++;
    int hideWarning2 = lastCol; hideWarning2++;

    Competition* c = MainWindow::competition();

	if(row!=lastRow){
		TimeStamp* ts = c->getTimeStamp(rowToID.value(row, 0));
		emit selectionChanged(ts);
	}
}

void TimeStampsTableWidget::selectTimeStamp(TimeStamp* ts)
{
	if(ts){
		int row = idToRow.value(ts->getID(), -1);
		if(row>=0)
		{
			selectRow(row);
		}
	}
}


void TimeStampsTableWidget::updateAgeTimes()
{
	for(int row=0; row<rowCount(); row++)
	{
		TimeStamp* ts = MainWindow::competition()->getTimeStamp(rowToID.value(row, 0));
		if(ts!=0 && onlineMode)
		{
			QTime ageTime;
            ageTime = ageTime.addMSecs(ts->getTime().msecsTo(MainWindow::app()->getTimeBaseTime())); // build difference

            QString str;
            if(ageTime.hour() >= 1){
                str = QString(">1h");
            }else{
                str = MainWindow::convertTimeToString(ageTime);
                str = str.left(str.length()-3);
            }
            setItem(row, 0, new QTableWidgetItem(str));
		}
	}
}


void TimeStampsTableWidget::setOnlineMode(bool online)
{
	onlineMode=online;
}


void TimeStampsTableWidget::on_cellDoubleClicked(int /*row*/, int /*col*/)
{
	emit assign();
}
