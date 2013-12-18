#include "timelineview.h"
#include <QPainter>
#include <QGradient>
#include <QTimer>
#include <QDebug>
#include <QPaintEvent>
#include <stdint.h>
#include <cmath>

#include "mainwindow.h"


#define T_LAMBDA_MS		10000			// period time for 'color wave' in ms
#define MOUSEJITTER		5				// maximum allowed distance form press to release for a valid 'click'
#define HIGHLIGHT_ZONE	5				// all timestamps from the mouse +/- HIGHLIGHT_ZONE are highlighted


#define WHEEL_STEP			120
#define ZOOM_SCALE			1.2
#define ZOOM_BASE			(pow(ZOOM_SCALE, 1.0/WHEEL_STEP))		// zoom factor=pow(ZOOM_BASE, delta)

// Colors in Timeline
#define ALPHA_UNASSIGNED	(0.9*255)
#define ALPHA_ASSIGNED		(0.5*255)

#define TS_COLOR			QColor(0, 212, 0, ALPHA_UNASSIGNED)			// for Triggerstations (from Boatboxes)
#define LS_COLOR			QColor(51, 125, 237, ALPHA_UNASSIGNED)		// for Lightswitches
#define M_COLOR				QColor(242, 41, 35, ALPHA_UNASSIGNED)		// for manual timestamps
#define E_COLOR				QColor(255, 150, 0, ALPHA_UNASSIGNED)		// for edited timestamps
#define HI_COLOR			QColor(56, 192, 236, ALPHA_UNASSIGNED)		// for highlighted timestamps
#define SE_COLOR			QColor(41, 41, 41, 255)						// for selected timestamps

#define BACK_COLOR			QColor(213, 255, 176, 255)
#define STRIPE_COLOR		QColor(248, 255, 62, 255)
#define BACK_ALPHA_LO		(0*255)
#define BACK_ALPHA_HI		(1*255)

#define NUM_OF_REFERENCES	20		// maximum number of visible time references


TimelineView::TimelineView(QScrollBar* scrollbar, QWidget *parent) :
	QWidget(parent)
{
	this->scrollbar=scrollbar;
	this->setFixedHeight(H_TIME);
	
	comp=0;
    tZero.setHMS(0, 0, 0, 0);		// dummy time with value=0
	
	selectedTimeStamp=0;
	selectIndex=0;
	selectedRun=0;
	
    rightTime=tZero.msecsTo(MainWindow::app()->getTimeBaseTime())+10000;
	leftTime=rightTime-20000;
	
	mousePressed=false;
}

void TimelineView::setVisibleTimeInterval(int leftTime, int rightTime)
{
	this->leftTime=leftTime;
	this->rightTime=rightTime;
	update();
}


void TimelineView::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::HighQualityAntialiasing);
	
	comp=MainWindow::competition();
	
	p.setBrush(BACK_COLOR);
	p.setPen(Qt::NoPen);

	// paint background (only in paint region)
	p.drawRect(event->rect());
	
	// paint time bar
	for(int x=0; x<width()-1; x++)
	{
		int t=pos2time(x);
		//p.setBrush(QColor::fromHsl(((uint64_t)t*360/T_LAMBDA_MS)%360, 20, 150));
		QColor stripeColor=STRIPE_COLOR;
		// the alpha value of the stripes is varied between BACK_ALPHA_LO and BACK_ALPHA_HI
		stripeColor.setAlpha((BACK_ALPHA_HI-BACK_ALPHA_LO) * (sin(2*t*M_PI/T_LAMBDA_MS)+1)/2 + BACK_ALPHA_LO);
		p.setBrush(stripeColor);
		
		p.drawRect(x, 0, 1, H_TIME);
	}
	
	// paint 2 black gaps
	p.setBrush(Qt::black);
	p.drawRect(0, H_BAR, width()-1, H_GAP);
	p.drawRect(0, H_BAR+H_GAP+H_BAR, width()-1, H_GAP);
	
	// paint time reference stripes
	drawGridLines(&p);
	
	// paint run
	if(selectedRun!=0 && !(selectedRun->getStartTimeID()==0 && selectedRun->getGoalTimeID()==0))
	{
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(255, 0, 0, 80));
		
		int left=0;
		int right=width()-1;
		if(selectedRun->getStartTimeID()!=0)
		{
			left=time2pos(tZero.msecsTo(comp->getTimeStamp(selectedRun->getStartTimeID())->getTime()));
		}
		if(selectedRun->getGoalTimeID()!=0)
		{
			right=time2pos(tZero.msecsTo(comp->getTimeStamp(selectedRun->getGoalTimeID())->getTime()));
		}
		
		p.drawRect(left, 0, right-left, height());
	}
	
    visibleTimeStamps=comp->getTimeStampsInInterval(tZero.addMSecs(leftTime), tZero.addMSecs(rightTime));

	//qDebug() << "number of visible timestamps:" << visibleTimeStamps.size();
	
	// paint the stripes for the timestamps
	for(int i=0; i<visibleTimeStamps.size(); i++)
	{
		TimeStamp* ts=visibleTimeStamps.at(i);

		// select color
		QColor color;
		switch(ts->getSource())
		{
			case TimeStamp::TSS:
			case TimeStamp::TSGL:
			case TimeStamp::TSGR:
				color=TS_COLOR; break;

			case TimeStamp::LS:
			case TimeStamp::LGL:
			case TimeStamp::LGR:
				color=LS_COLOR; break;

			case TimeStamp::M:
				color=M_COLOR; break;

			case TimeStamp::E:
				color=E_COLOR; break;
		}

		/*
		if(ts->getRunID()!=0)
		{
			// this timestamp is bound to a run, fade it's color
			//color.setHsl(color.hue(), 80, color.lightness());
			color.setAlpha(ALPHA_ASSIGNED);
		}
		*/
		
		drawTimeStamp(&p, ts, color, 3);
	}

	for(int i=0; i<highlightedTimeStamps.size(); i++)
	{
		drawTimeStamp(&p, highlightedTimeStamps.at(i), HI_COLOR, 3);
	}
	
    if(selectedTimeStamp!=0)
	{
        drawTimeStamp(&p, selectedTimeStamp, SE_COLOR, 5);
    }
}


void TimelineView::drawTimeStamp(QPainter* p, TimeStamp* ts, QColor color, int width)
{
	
	//qDebug() << "draw timestamp";
    int time=tZero.msecsTo(ts->getTime());
    double pos=time2pos(time);
	
	//qDebug() << "time:" << leftTime << time << rightTime;
	//qDebug() << "pos:" << pos;
	
	
    //set pen
    QPen pen(color);
    pen.setWidth(width);
	pen.setCapStyle(Qt::FlatCap);
    p->setPen(pen);
	
	QPointF upperEnd, lowerEnd;
	upperEnd.setX(pos);
	lowerEnd.setX(pos);
	
    switch(selectLine(ts))
    {
        case START:
			upperEnd.setY(0);
			lowerEnd.setY(H_BAR);
            break;
        case GOAL1:
			upperEnd.setY(H_BAR+H_GAP);
			lowerEnd.setY(H_BAR+H_GAP+H_BAR);
            break;
        case GOAL2:
			upperEnd.setY(H_BAR+H_GAP+H_BAR+H_GAP);
			lowerEnd.setY(H_BAR+H_GAP+H_BAR+H_GAP+H_BAR);
            break;
        case START|GOAL1|GOAL2:
			upperEnd.setY(0);
			lowerEnd.setY(H_TIME);
            break;
        case GOAL1|GOAL2:
			upperEnd.setY(H_BAR+H_GAP);
			lowerEnd.setY(H_TIME);
            break;
    }
	
	p->drawLine(upperEnd, lowerEnd);
}


void TimelineView::mouseMoveEvent(QMouseEvent *event)
{
	int cursorPos=(event->pos().x());
	
	if(mousePressed)
	{
		// drag by mouse
		int dragTime=pos2time(oldPos)-pos2time(cursorPos);
		scrollbar->setValue(scrollbar->value()+dragTime);
		
		oldPos=cursorPos;
	}
	else
	{
		highlight(event->pos());
	}
}


void TimelineView::highlight(QPoint p)
{
	// highlight timestamps per mouse

	int cursorPos=p.x();
	int y=p.y();

	// find out the line under the cursor
	int cursorLine=NONE;
	if(y<H_BAR)
	{
		cursorLine=START;
	}
	else if(y<H_BAR+H_GAP)
	{
		cursorLine=START|GOAL1|GOAL2;
	}
	else if(y<H_BAR+H_GAP+H_BAR)
	{
		cursorLine=GOAL1;
	}
	else if(y<H_BAR+H_GAP+H_BAR+H_GAP)
	{
		cursorLine=GOAL1|GOAL2;
	}
	else if(y<H_BAR+H_GAP+H_BAR+H_GAP+H_BAR)
	{
		cursorLine=GOAL2;
	}

	highlightedTimeStamps.clear();
	selectIndex=0;
	
	for(int i=0; i<visibleTimeStamps.size(); i++)
	{
		TimeStamp* ts=visibleTimeStamps.at(i);
		int pos=time2pos(tZero.msecsTo(ts->getTime()));
		
		// search for timestamps with fitting time
		if(cursorPos > pos-HIGHLIGHT_ZONE && cursorPos < pos+HIGHLIGHT_ZONE)
		{
			if((cursorLine & selectLine(ts)) == cursorLine)
			{
				// mouse was in the right line
				highlightedTimeStamps.append(ts);
			}
		}
	}
	
	update();
}


void TimelineView::leaveEvent(QEvent* /*event*/)
{
	highlightedTimeStamps.clear();
	update();
}


void TimelineView::mousePressEvent(QMouseEvent *event)
{
	mousePressed=true;
	pressPos=event->pos();
	oldPos=event->pos().x();
}

void TimelineView::mouseReleaseEvent(QMouseEvent *event)
{
	mousePressed=false;
	if((event->pos()-pressPos).manhattanLength()<MOUSEJITTER)
	{
		// there was no drag so this is a click
		if(highlightedTimeStamps.isEmpty())	// nothing highlighted
		{
			selectedTimeStamp=0;
		}
		else
		{
			selectIndex++;									// click => next index in highlightedTimeStamps
			if(selectIndex>=highlightedTimeStamps.size())	// index is out of bounds
			{
				selectIndex=0;								// start again at the beginning
			}
			selectedTimeStamp=highlightedTimeStamps.at(selectIndex);
			emit selectionChanged(selectedTimeStamp);
		}
		
		update();
	}
	else
	{
		// there was a drag
		highlight(event->pos());
	}
}

void TimelineView::wheelEvent(QWheelEvent *event)
{
	if(event->modifiers() & Qt::ControlModifier)
	{
		// this is a zoom
		int zoomCenterTime=pos2time(event->pos().x());
		double zoomFactor=pow(ZOOM_BASE, -event->delta());
		
		emit zoomAt(zoomCenterTime, zoomFactor);
		highlight(event->pos());
	}
	else
	{
		// this is a scroll, send to timeline
		event->ignore();
	}
}



int TimelineView::selectLine(TimeStamp* ts)
{
	int line=NONE;	// this is binary enum

	switch(ts->getSource())
	{
		case TimeStamp::TSS:
		case TimeStamp::LS:
			line=START; break;

		case TimeStamp::TSGL:
		case TimeStamp::LGL:
			line=GOAL1; break;

		case TimeStamp::TSGR:
		case TimeStamp::LGR:
			line=GOAL2; break;
		
		case TimeStamp::M:
		case TimeStamp::E:
		{
			// try to find line from run
			int runID=ts->getRunID();
			if(runID!=0)
			{
				// this timestap is bound to a run
				int startID, goalID;
				startID=comp->getRun(runID)->getStartTimeID();
				goalID=comp->getRun(runID)->getGoalTimeID();
				
				if(ts->getID()==startID)
				{
					line=START;
				}
				else if(ts->getID()==goalID)
				{
					line=GOAL1|GOAL2;
				}
			}
			else
			{
				// this timestamp is unbound
				line=START|GOAL1|GOAL2;
			}
		}
	}

	return line;
}


double TimelineView::time2pos(int time_ms)
{
	/*
	 * transform time (in ms) to position (in pixel)
	 * 
	 * time==leftTime means pos=0
	 * time==rightTime means pos=width()
	 */
	
	return (double)(time_ms-leftTime)*width()/(rightTime-leftTime);
}

int TimelineView::pos2time(double pos)
{
	/*
	 * transform position (in pixel) to time (in ms)
	 *
	 * pos==0 means time=leftTime
	 * pos==width() means time=rightTime
	 */
	
	return pos*(rightTime-leftTime)/width() + leftTime;
}


// selection from outside
void TimelineView::selectTimeStamp(TimeStamp* ts)
{
	//qDebug() << "timestamp selected from outside";
	selectedTimeStamp=ts;
	/*
	if(ts==0)
	{
		qDebug() << "SIGNAL: deselect timestamp";
	}
	*/
}


void TimelineView::selectRun(RunData* run)
{
	//qDebug() << "slot: selectRun";
	selectedRun=run;
}


void TimelineView::drawGridLines(QPainter* p)
{
	// the raw value of gridTime
	int dt=(rightTime-leftTime)/NUM_OF_REFERENCES;
	
	QList<int> gridTimeTable;
	gridTimeTable << 100 << 200 << 500 << 1000 << 2000 << 5000 << 10*1000 << 20*1000 << 60*1000 << 2*60*1000 << 5*60*1000 << 10*60*1000 << 20*60*1000 << 60*60*1000 << 2*3600*1000;
	
	int gridTime=gridTimeTable.last(); // default value
	
	// find out the next fitting grid time
	for(int i=0; i<gridTimeTable.size(); i++)
	{
		if(gridTimeTable.at(i)>dt)
		{
			gridTime=gridTimeTable.at(i);
			break;
		}
	}
	
	int n1=leftTime/gridTime+1;		// index of first visible gridline
	int n2=rightTime/gridTime;		// index of last visible gridline
	
	int n=n2-n1+1;					// finite number of gridlines
	
	//set pen
    QPen pen(Qt::darkGray);
    pen.setWidth(1);
	pen.setStyle(Qt::DashLine);
	pen.setCapStyle(Qt::FlatCap);
    p->setPen(pen);
	
	// draw the grid lines
	for(n=n1; n<=n2; n++)
	{
		double pos=time2pos(n*gridTime);
		p->drawLine(pos, 0, pos, H_TIME);
	}
	
	// format grid time and absolute time text
	QString absTimeText, gridTimeText;
	QTime displayTime=tZero.addMSecs(rightTime);
	
	if(gridTime<1000)			// gridTime is in millisecond scale
	{
		gridTimeText=QString::number(gridTime)+"ms";
		absTimeText=displayTime.toString("hh:mm:ss");
	}
	else if(gridTime<60*1000)	// gridTime is in second scale
	{
		gridTimeText=QString::number(gridTime/1000)+"s";
		absTimeText=displayTime.toString("hh:mm:ss");
	}
	else if(gridTime<3600*1000)	// gridTime is in minute scale
	{
		gridTimeText=QString::number(gridTime/(60*1000))+"min";
		absTimeText=displayTime.toString("hh:mm");
	}
	else	// gridTime is in hour scale
	{
		gridTimeText=QString::number(gridTime/(3600*1000))+"h";
		absTimeText=displayTime.toString("hh")+":00";
	}
	
	// display absolute time and grid time
	pen.setColor(Qt::black);
	p->setPen(pen);
	QFont font=QFont("Courier", 12);
	font.setBold(true);
	p->setFont(font);
	
	p->drawText(rect(),Qt::AlignRight|Qt::AlignTop, absTimeText);
	p->drawText(rect(),Qt::AlignRight|Qt::AlignBottom, gridTimeText);
	
	
}
