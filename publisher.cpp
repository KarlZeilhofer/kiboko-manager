#include "publisher.h"
#include "dialoggeneralsettings.h"
#include "competition.h"

Publisher::Publisher(QObject *parent) :
	QObject(parent)
{
	connect(&timer, SIGNAL(timeout()), this, SLOT(displayNextRun()));
	timer.setSingleShot(true);
}


void Publisher::addToQueueManual(RunData* run)
{
	// add the run to the queue and show it now if possible
	
	if(run!=0 && run->getPublished())
	{
		removeInvalidRuns();		// remove all runs that are not marked for publishing
		queue.removeAll(run);		// remove double entries
		queue.append(run);
		
		if(!timer.isActive())		// if there is nothing displayed at the moment, show it now
		{
			displayNextRun();
		}
	}
}

void Publisher::addToQueueAuto(RunData* run)
{
	// add the run to the queue
	// don't do anything if the run does not exist or it is not marked for publishing
	
	if(run!=0)
	{
		removeInvalidRuns();		// remove all runs that are not marked for publishing
		queue.removeAll(run);		// remove double entries
		queue.append(run);
		
		run->setPublished(true, true);
		
		if(!timer.isActive())		// if there is nothing displayed at the moment, start timer
		{
			startTimer();
		}
	}
}


// slot for the timer
void Publisher::displayNextRun()
{
	Competition* comp=MainWindow::competition();
	RunData* run;
	
	//qDebug() << "Runs to publish: "<< queue.size();
	
	removeInvalidRuns();
	
	if(!queue.isEmpty())
	{
		run=queue.takeFirst();
		
		QTime runtime=comp->getRunTime(run->getID());
		
		qDebug() << "Publish: Runde: " << run->getRound() << ", Zille: " << run->getBoatID() << ", Zeit: " << runtime.toString("mm:ss.zzz") << ", DSQ: "<< run->getDSQ();
		emit publishRunNow(run);
		
		startTimer();
		// don't restart timer when queue is empty
	}
}


void Publisher::clear()
{
	queue.clear();
	timer.stop();
	//qDebug() << "Runs to publish: "<< queue.size();
}


void Publisher::startTimer()
{
	int publishInterval=MainWindow::app()->dialogGeneralSettings->publishInterval();
	if(publishInterval<1)
	{
		// make a lower limit for the publish interval: 1 sec
		publishInterval=1;
	}
	timer.start(publishInterval*1000);
}


void Publisher::removeInvalidRuns()
{
	// remove all runs that are not marked for publishing
	for(int i=0; i<queue.size(); i++)
	{
		if(!queue.at(i)->getPublished())
		{
			queue.removeAt(i);
		}
	}
}

