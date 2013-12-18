#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QScrollBar>

#include "timestamp.h"
#include "rundata.h"
#include "competition.h"


#define H_BAR			20					// height of timebar
#define H_GAP			5					// distance betweeen timebars
#define H_TIME			(3*H_BAR+2*H_GAP)	// total height

class TimelineView : public QWidget
{
	Q_OBJECT
public:
	explicit TimelineView(QScrollBar* scrollbar, QWidget *parent = 0);

	enum {NONE=0x0, START=0x1, GOAL1=0x2, GOAL2=0x4};
	
	void setVisibleTimeInterval(int leftTime, int rightTime);

public slots:
	void selectTimeStamp(TimeStamp* ts);
	void selectRun(RunData* run);

private:
	void paintEvent(QPaintEvent *event);
	void drawTimeStamp(QPainter* p, TimeStamp* ts, QColor color, int width);

	void mouseMoveEvent(QMouseEvent *event);
	void highlight(QPoint p);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);
	
	void wheelEvent(QWheelEvent *event);

	int selectLine(TimeStamp* ts);
	
	void drawGridLines(QPainter* p);
	
	double time2pos(int time_ms);
	int pos2time(double pos);
	
	int leftTime;
	int rightTime;

    QTime tZero;
	Competition* comp;
	QList<TimeStamp*> visibleTimeStamps;

	TimeStamp* selectedTimeStamp;
	QList<TimeStamp*> highlightedTimeStamps;
	int selectIndex;
	RunData* selectedRun;
	
	bool mousePressed;
	QPoint pressPos;
	int oldPos;
	
	QScrollBar* scrollbar;
	
signals:
	void selectionChanged(TimeStamp* selected);			// is emitted whenever the user clicks in the timeline. selected=0 means nothing selected
	void zoomAt(int zoomCenterTime, double zoomFactor);	// is sent to timeline to do a zoom
	
};

#endif // TIMELINEVIEW_H
