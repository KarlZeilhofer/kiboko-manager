#include "publictablewidget.h"
#include "rundata.h"
#include "mainwindow.h"
#include "competition.h"
#include <QHeaderView>
#include <QApplication>
#include <QTimer>

PublicTableWidget::PublicTableWidget(int numOfLines, QWidget *parent) :
    QTableWidget(parent)
{
    setRowCount(numOfLines);
    setColumnCount(10);
    setHorizontalHeaderLabels(QString("Bew,DG,Farbe,Zille,Name,Feuerwehr,Bewertungsklasse,Laufzeit,DSQ,Fehler").split(','));

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::NoSelection);

    verticalHeader()->hide();

    QFont f;
    f.setPixelSize(getFontSize());
    f.setBold(true);
    horizontalHeader()->setFont(f);
	
    setAlternatingRowColors(true);
	
	// this works on Kubuntu but not on Ubuntu!
	QPalette pal=palette();
	//QColor(200, 113, 55)	// "Kiboko-brown"
	pal.setColor(QPalette::AlternateBase, QColor(150, 150, 150));
	setPalette(pal);
}

void PublicTableWidget::setRunsList(QList<RunData *> list)
{
    runs.clear();
    runs.append(list);
    regenerateTable();
}

int PublicTableWidget::numOfRuns()
{
    //qDebug() << height()/30;
    return height()/41;
}

void PublicTableWidget::regenerateTable()
{
	int lastFinishedRun=-1;
	
    clearContents();
//    qDebug("PublicTableWidget::regenerateTable()");
    // print all items in the table
    for(int n=0; n<runs.length(); n++){
        RunData* run = runs.at(n);

        if(run){
            QTableWidgetItem* item;
            int c=0;

            // run-ID:
            item = getItem(QString::number(run->getID()));
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);

            // round:
            item = getItem(QString::number(run->getRound()));
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);

            // color:
            item = getItem(run->getColor());
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);

            // Boat number:
            item = getItem(QString::number(run->getBoatID()));
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);

            // names:
            QString names;

            if(run->getTitle2().isEmpty() && run->getFirstName2().isEmpty() && run->getLastName2().isEmpty()){
                names = run->getFirstName1() + " " + run->getLastName1();
            }else{
                names = run->getFirstName1().left(1) + ". " + run->getLastName1() + ", " + run->getFirstName2().left(1) + ". " + run->getLastName2();
            }
            item = getItem(names);
            item->setFlags(Qt::ItemIsEnabled);// set non-editable (at least not in standard way)
            setItem(n, c++, item);

            // firebrigade:
            item = getItem(run->getFireBrigade());
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);

            // valuation class:
            item = getItem(run->getValuationClass());
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);

            // RunTime
            QString str;
            if(run->getStartTimeID()!=0 && run->getGoalTimeID()!=0 && run->getPublished())	// only for finished and published runs
            {
				// run finished
                QTime startTime = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime();
                QTime goalTime = MainWindow::competition()->getTimeStamp(run->getGoalTimeID())->getTime();
                QTime runTime;
                runTime = runTime.addMSecs(startTime.msecsTo(goalTime)); // build difference
                str = MainWindow::convertTimeToString(runTime);
				
				// find the last finished run (the first in the list of finished)
				if(lastFinishedRun<0)
				{
					lastFinishedRun=n;
				}
            }
            else if(run->getStartTimeID()!=0 && run->getGoalTimeID()!=0 && run->getPublished()==false)	// only for finished and not published runs
            {
                str = QString(tr("Auswertung"));
            }
            else if(run->getStartTimeID()!=0 && run->getGoalTimeID()==0)	// only for running runs
            {
                str = QString(tr("läuft"));
            }
            else if(run->getStartTimeID()==0 && run->getGoalTimeID()==0)	// only for waiting runs
            {
                str = QString(tr("am Start"));
            }

            item = getItem(str);
            item->setFlags(Qt::ItemIsEnabled); // set non-editable
            setItem(n, c++, item);

            // Dsq
            if(run->getDSQ()){// show checkbox
                item = getItem("  X  ");
			}else{
                item = getItem("");
            }
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            // TODO: set horizontal center aligned!
            setItem(n, c++, item);

            // Errors
            item = getItem(run->getErrors());
            item->setFlags(Qt::ItemIsEnabled);// set non-editable
            setItem(n, c++, item);
        }// end if(run)
    }// end for all runs
	
	
	// highlight the last finished run
	if(lastFinishedRun>=0 && lastFinishedRun<runs.length())
	{
		for(int c=0; c<columnCount(); c++)
		{
			QTableWidgetItem* tableItem=item(lastFinishedRun, c);
			if(tableItem!=0)
			{
				tableItem->setBackgroundColor(QColor(Qt::green).lighter());
			}
		}
		
	}

    resizeColumnsToContents();
    resizeRowsToContents();

    // TODO: gewisse spalten etwas verbreitern. hat noch nicht so funktioniert wie es soll!
//    int colToExtendSize[] = {4,5,7-1};
//    for(int n=0; colToExtendSize[n]>=0; n++){
//        setColumnWidth(colToExtendSize[n], columnWidth(colToExtendSize[n])+20);
//    }

    QFont f;
    f.setPixelSize(getFontSize());
    f.setBold(true);
    horizontalHeader()->setFont(f);

}

void PublicTableWidget::refreshRunTimes()
{
    for(int n=0; n<runs.length(); n++){
        RunData* run = runs.at(n);
        QTableWidgetItem* item;

        if(run){
            QString str;
            if(run->getStartTimeID()!=0 && run->getGoalTimeID()==0)	// only for running runs
            {
                QTime startTime = MainWindow::competition()->getTimeStamp(run->getStartTimeID())->getTime();
                QTime currentTime = MainWindow::app()->getTimeBaseTime();
                QTime runTime;
                runTime = runTime.addMSecs(startTime.msecsTo(currentTime)); // build difference
                str = MainWindow::convertTimeToString(runTime);
                str = str.left(str.length()-1); // remove 1/100s digit

                item = getItem(str);
                item->setFlags(Qt::ItemIsEnabled); // set non-editable
                setItem(n, 7, item);
            }
        }
    }
}

QTableWidgetItem *PublicTableWidget::getItem(QString text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(Qt::ItemIsEnabled);// set non-editable
    QFont f;
    f.setPixelSize(getFontSize());
    f.setBold(true);
    item->setFont(f);
    return item;
}

void PublicTableWidget::resizeEvent(QResizeEvent *event)
{
    regenerateTable();
    QWidget::resizeEvent(event);
}

qreal PublicTableWidget::getFontSize()
{
    //return ((height()-h1)-rowCount()*h2)/rowCount();
    return width()/70;
}
