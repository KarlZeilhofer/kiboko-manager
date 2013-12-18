#include "timeline.h"
#include "mainwindow.h"

#include <QtGui>

#define TIME_INC_MS			20				// timer increment in ms
#define LEAVE_DELAY_MS		1000			// delay time after leave event in ms

#define ALPHA_SLIDE			0.000001		// spring parameter for slide
#define BETA_SLIDE			0.002			// damping parameter for slide
#define A_SLIDE_LIMIT		0.0005			// acceleration limit
#define TUNNELING_SPEED		1				// if this speed is reached, simply jump to xDest instead of sliding

TimeLine::TimeLine(QWidget *parent) :
	QWidget(parent)
{
	comp=0;
	tZero.setHMS(0, 0, 0, 0);		// dummy time with value=0
	
	vSlide=0;
	justInTime=true;
	centering=Nothing;
	onlineMode=true;
	selectedRun=0;
	
	scrollbar=new QScrollBar(Qt::Horizontal);
	timelineView=new TimelineView(scrollbar);
	
	QLabel* labelStart=new QLabel(tr("Start:"));
	QLabel* labelGoalLeft=new QLabel(tr("Ziel L:"));
	QLabel* labelGoalRight=new QLabel(tr("Ziel R:"));
	
	QGridLayout* grid=new QGridLayout;
	
	grid->addWidget(labelStart, 0, 0);
	grid->addWidget(labelGoalLeft, 1, 0);
	grid->addWidget(labelGoalRight, 2, 0);
	
	grid->addWidget(timelineView, 0, 1, 3, 1);
	grid->addWidget(scrollbar, 3, 1);
	grid->setColumnStretch(1, 1);
	
	setLayout(grid);
	
	updateTimer.start(TIME_INC_MS);
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(timeInc()));
	
	visibleTime_ms=60000;			// Zoom size
	
	connect(&leaveDelay, SIGNAL(timeout()), this, SLOT(leaveAction()));
	
	timelineView->setMouseTracking(true);
	
	connect(timelineView, SIGNAL(zoomAt(int,double)), this, SLOT(zoomAt(int,double)));
	
    connect(timelineView, SIGNAL(selectionChanged(TimeStamp*)), this, SIGNAL(selectionChanged(TimeStamp*)));
}

void TimeLine::timeInc()
{
	comp=MainWindow::competition();
	
	// find left and right end of timeline
	if(comp->getNumOfTimeStamps()>0)
	{
		leftEndTime_ms=tZero.msecsTo(comp->getBeginTime())-60000;		// left end is set to 60s before first timestamp
	}
	else
	{
		leftEndTime_ms=0;												// the left is set to 00:00:00
	}
	
	if(onlineMode)
	{
        rightEndTime_ms=tZero.msecsTo(MainWindow::app()->getTimeBaseTime());			// right end is set to current time
	}
	else
	{
		if(comp->getNumOfTimeStamps()>0)
		{
			rightEndTime_ms=tZero.msecsTo(comp->getEndTime())+10000;	// right end is set to 10s before last timestamp
		}
		else
		{
			rightEndTime_ms=tZero.msecsTo(QTime(23,59,59));				// right end is set to 23:59:59
		}
	}
	
	/*
	 * the scrollbar value is the leftTime of visible region
	 */
	scrollbar->setPageStep(visibleTime_ms);
	scrollbar->setSingleStep(visibleTime_ms/50);
	scrollbar->setMinimum(leftEndTime_ms);
	
	if(justInTime && onlineMode)
	{
		// scroll to the right end
		if(scrollbar->value()<scrollbar->maximum())
		{
			// we are too far away so do a slide first
			sliding=true;
			scrollbar->setMaximum(rightEndTime_ms-visibleTime_ms);
			xDest=time2pixel(scrollbar->maximum()+5000);
		}
		else
		{
			// we are here, just 'hook in' to maximum
			sliding=false;
			scrollbar->setMaximum(rightEndTime_ms-visibleTime_ms);
			scrollbar->setValue(scrollbar->maximum());
		}
		//qDebug() << scrollbar->maximum()-scrollbar->value();
	}
	else
	{
		scrollbar->setMaximum(rightEndTime_ms-visibleTime_ms);
	}
	
	if(sliding)
	{
		slideAction();
	}
	
	timelineView->setVisibleTimeInterval(scrollbar->value(), scrollbar->value()+visibleTime_ms);
	//timelineView->update();
}


void TimeLine::slideAction()
{	
	// the slide action is calculated in pixels, not in ms
	xSlide=time2pixel(scrollbar->value());

	//qDebug() << "vSlide " << vSlide;
	//qDebug() << "xSlide " << xSlide;

	if(xSlide==xDest)
	{
		sliding=false;
		vSlide=0;
		return;
	}

	double aSlide = -(xSlide-xDest)*ALPHA_SLIDE - vSlide*BETA_SLIDE;

	// limit the acceleration
	if(aSlide>A_SLIDE_LIMIT)
	{
		aSlide=A_SLIDE_LIMIT;
	}
	else if(aSlide<-A_SLIDE_LIMIT)
	{
		aSlide=-A_SLIDE_LIMIT;
	}
	
	//qDebug() << "aSlide " << aSlide;

	vSlide+=aSlide*TIME_INC_MS;
	xSlide+=vSlide*TIME_INC_MS;
	
	// check critical speed
	if(fabs(vSlide)>TUNNELING_SPEED)
	{
		// critical speed is reached --> tunneling!!!
		xSlide=xDest-copysign(1200, vSlide);
		vSlide=copysign(TUNNELING_SPEED, vSlide);
	}

	scrollbar->setValue(pixel2time(xSlide));
}


void TimeLine::enterEvent(QEvent* /*event*/)
{
	leaveDelay.stop();		// TimeLine is entered again, don't do the leave action
	vSlide=0;				// stop sliding
	sliding=false;
	justInTime=false;		// don't follow the nowpoint
}


void TimeLine::leaveEvent(QEvent* /*event*/)
{
	// mouse cursor left TimeLine
	// wait a little bit before doing the leaveAction
	leaveDelay.start(LEAVE_DELAY_MS);
}


void TimeLine::leaveAction()
{
	leaveDelay.stop();	// don't do the leaveAction again
	if(centering==Nothing)
	{
		justInTime=true;	// go forward to nowpoint again
	}
}


void TimeLine::wheelEvent(QWheelEvent *event)
{
	// scrolling: forward the event to the scrollbar
	scrollbar->event(event);
}


void TimeLine::zoomAt(int zoomCenterTime, double zoomFactor)
{
	int leftTime=scrollbar->value();
	int rightTime=leftTime+visibleTime_ms;
	
	leftTime=zoomCenterTime-(zoomCenterTime-leftTime)*zoomFactor;
	rightTime=zoomCenterTime+(rightTime-zoomCenterTime)*zoomFactor;
	
	// limit the zoom area to boundaries
	if(leftTime<leftEndTime_ms)
	{
		leftTime=leftEndTime_ms;
		//qDebug() << "left end reached";
	}
	if(rightTime>rightEndTime_ms)
	{
		rightTime=rightEndTime_ms;
		//qDebug() << "right end reached";
	}
	
	// don't zoom if visible time is too small
	if(rightTime-leftTime>1000)
	{
		visibleTime_ms=rightTime-leftTime;
		scrollbar->setPageStep(visibleTime_ms);
		scrollbar->setMaximum(rightEndTime_ms-visibleTime_ms);
		scrollbar->setValue(leftTime);
	}
}


double TimeLine::time2pixel(int time)
{
	return (double)time*timelineView->width()/visibleTime_ms;
}

int TimeLine::pixel2time(double pixel)
{
	return (double)pixel*visibleTime_ms/timelineView->width();
}


void TimeLine::selectTimeStamp(TimeStamp* ts)
{
	timelineView->selectTimeStamp(ts);
	
	// center unassigned timestamps
	if(ts!=0 && centering==Unassigned && ts->getRunID()==0)
	{
		int centerTime_ms=tZero.msecsTo(ts->getTime());
		xDest=time2pixel(centerTime_ms-visibleTime_ms/2);
		sliding=true;
	}
}

void TimeLine::setCentering(Centering c)
{
	centering=c;
	if(centering==Nothing)
	{
		justInTime=true;
	}
	else
	{
		justInTime=false;
		if(centering==Start || centering==Goal)
		{
			selectRun(selectedRun);
		}
	}
}

void TimeLine::setOnlineMode(bool mode)
{
	onlineMode=mode;
	if(!onlineMode)
	{
		//qDebug() << "offline mode -> go to the right";
		// go to the right end
		sliding=false;
		timeInc();
		scrollbar->setValue(scrollbar->maximum());
	}
}

void TimeLine::selectRun(RunData *run)
{
	selectedRun=run;
	if(run!=0 && (centering==Start||centering==Goal))
	{
		TimeStamp* ts;
		if(centering==Start)
		{
			ts=comp->getTimeStamp(run->getStartTimeID());
		}
		else
		{
			ts=comp->getTimeStamp(run->getGoalTimeID());
		}
		if(ts!=0)
		{
			// if the timestamp exists, slide it to center and select it
			selectTimeStamp(ts);
			int centerTime_ms=tZero.msecsTo(ts->getTime());
			xDest=time2pixel(centerTime_ms-visibleTime_ms/2);
			sliding=true;
		}
	}
	
	timelineView->selectRun(run);
}
