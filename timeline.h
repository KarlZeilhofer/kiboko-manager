#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QScrollBar>
#include <QLabel>
#include <QTimer>

#include "timestamp.h"
#include "rundata.h"
#include "timelineview.h"
#include "competition.h"


class TimeLine : public QWidget
{
	Q_OBJECT
public:
	enum Centering{
		Nothing,	// normal operation, just follow the current time
		Start,		// center the start time of the selected run (if existing)
		Goal,		// center the goal time of the existing run (if existing)
		Unassigned	// center an unassigned timestamp
	};

	explicit TimeLine(QWidget *parent = 0);
	
	void setCentering(Centering c);
	void setOnlineMode(bool mode);
	
signals:
	void selectionChanged(TimeStamp* selected);	// is emitted when the user clicks on a timeStamp, selected=0 means nothing selected
	
public slots:
	void selectTimeStamp(TimeStamp* ts);	// draws this timestamp in a different color
	void selectRun(RunData* run);			// draw a rectangle from starttime to goaltime of this run, depending on the centering option, the corresponding timestamp is centered
	
private slots:
	void timeInc();
	void leaveAction();
	void zoomAt(int zoomCenterTime, double zoomFactor);
	
private:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void wheelEvent(QWheelEvent *event);
	
	TimelineView* timelineView;
	QScrollBar* scrollbar;
	
	Competition* comp;
	
	int rightEndTime_ms;		// time in ms (right end of timeline)
	int leftEndTime_ms;			// time in ms at left end of timeline
	int visibleTime_ms;
	
	QTimer updateTimer;
	
	
	// time delay after leaving the widget
	QTimer leaveDelay;

	// to decide if it was a click or not
	int mouseMoveDistance;

	// do a smooth slide to a specified position
	void slideAction();	
	bool sliding;
	double vSlide;
	double xSlide;
	double xDest;
	
	bool justInTime;			// true if the widget is following the current time
	Centering centering;		// the current centering setting
	bool onlineMode;
	RunData* selectedRun;
	
	double time2pixel(int time);
	int pixel2time(double pixel);
	
	QTime tZero;
};

#endif // TIMELINE_H
