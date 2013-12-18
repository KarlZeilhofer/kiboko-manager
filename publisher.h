#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include "rundata.h"
#include "mainwindow.h"


class Publisher : public QObject
{
	Q_OBJECT
public:
	explicit Publisher(QObject *parent = 0);
	
signals:
	void publishRunNow(RunData* run);
	
public slots:
	void addToQueueManual(RunData* run);
	void addToQueueAuto(RunData* run);
	void clear();
	
private slots:
	void displayNextRun();
	
private:
	QList<RunData*> queue;
	QTimer timer;
	
	void startTimer();
	void removeInvalidRuns();
	
};

#endif // PUBLISHER_H
