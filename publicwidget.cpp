#include "publicwidget.h"
#include "publictablewidget.h"
#include <QLayout>
#include "rundata.h"
#include "mainwindow.h"
#include "competition.h"
#include <QGroupBox>
#include "dialoggeneralsettings.h"

PublicWidget::PublicWidget(QWidget *parent) :
    QWidget(parent)
{
    QGridLayout* mainLayout = new QGridLayout(this);

    //setWindowFlags(Qt::Dialog);

    tableOfBoats = new PublicTableWidget(31);

    startLabel = new QLabel(this);

    QFont f;
    f.setPixelSize(30);
    f.setBold(true);
    startLabel->setFont(f);


    mainLayout->addWidget(tableOfBoats, 0,0,1,1);
    mainLayout->addWidget(startLabel, 1,0,1,1);


    mainLayout->setRowStretch(0,1);
    mainLayout->setRowStretch(1,0);

    connect(&timer, SIGNAL(timeout()), this, SLOT(oneSecondElapsed()));
    //reloadCountDown();
    startLabel->setText("");

    show();
}

void PublicWidget::refreshRuns()
{
//    qDebug("PublicWidget::refreshRuns()");
//    // TODO dynamisch an die widget-höhe anpassen
    // zu beachten: das interne widget soll jedoch das gleiche wie das am Full-Hd monitor anzeigen.

    QList<RunData*> comboList;
    int n=31; // TODO: make it dynamic

    QList<RunData*> runningList = MainWindow::app()->competition()->getRunningRuns();
    n-=runningList.length();
    QList<RunData*> waitingList = MainWindow::app()->competition()->getNextBeginningRuns(3);
    n-=waitingList.length();
    QList<RunData*> finishedList = MainWindow::app()->competition()->getLastFinishedRuns(n);

    QList<RunData*> revWaitingList;
    // reverse the waiting-list:
    for(int n=0; n<waitingList.length(); n++){
        revWaitingList.append(waitingList.at(waitingList.length()-1-n));
    }
    comboList.append(revWaitingList);
    comboList.append(runningList);
    comboList.append(finishedList);

    tableOfBoats->setRunsList(comboList);
    tableOfBoats->numOfRuns(); // TODO: remove! is here just for debugging

}

void PublicWidget::reloadCountDown()
{
    if(MainWindow::app()->dialogGeneralSettings){ // if pointer available
        countDownCounter = MainWindow::app()->dialogGeneralSettings->startInterval();
        if(countDownCounter>0){
            timer.start(1000);
        }else{
            timer.stop();
            startLabel->setText("");
        }
    }else{
        qDebug() << "Invalid pointer to dialogGeneralSettings in " << __CLASS__ << "::" << __func__;
    }
}

void PublicWidget::oneSecondElapsed()
{
    countDownCounter--;
    if(countDownCounter>0)
	{
        startLabel->setText(QString(tr("nächster Start in %1s")).arg(countDownCounter));
    }
    else // countDownCounter=0
	{
        startLabel->setText("Zille startet!");
		timer.stop();
    }
}

void PublicWidget::resizeEvent(QResizeEvent * event)
{
    refreshRuns();
    QWidget::resizeEvent(event);
}


