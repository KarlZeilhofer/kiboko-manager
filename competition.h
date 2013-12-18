#ifndef COMPETITION_H
#define COMPETITION_H

#include <QObject>

// forward declarations:
#include <QString>
#include <QStringList>
#include <QList>
#include <QtDebug>
#include "rundata.h"
#include "timestamp.h"
#include "boat.h"
#include "boatbox.h"

class Competition : public QObject
{
    Q_OBJECT
public:
	explicit Competition(QObject *parent, QString descr, int bpr, QStringList colors,
					 QString databaseHostName, QString databaseName);

	~Competition();

	// load and save
	bool load(QString fileName);
	bool save(QString fileName);
	bool exportCSV(QString fileName);
	bool isModified();

	// general setters
	void setDescription(QString description);
	void setBoatsPerRound(int bpr);
	void setColors(QStringList colors);
	void setDatabaseHostName(QString databaseHostName);
	void setDatabaseName(QString databaseName);

	// general getters
	QDateTime getCreationDate();
	QString getCreationHost();
	QString getDescription();
	int getBoatsPerRound();
	QStringList getColors();
	QString getDatabaseHostName();
	QString getDatabaseName();

    // insert functions
	void addRun(RunData* run);
	void addTimeStamp(TimeStamp* timeStamp);

    // linking functions
	void bindBox2BoatID(int boatboxID, int boatID);
	void setBoatName(int boatID, QString boatName);
    void unassignTimeStamp(int tsID);

	// list getters
	RunData* getRun(int ID);
	TimeStamp* getTimeStamp(int ID);

	// get links
	int getBoatIdOfBoatbox(int boatboxID);
	QString getNameOfBoatID(int boatID);
	int getBoatboxIdOfBoat(int boatID);
	QString getBoatName(int boatID);


	// other
    int getHighestValidRunID();
    int getNumOfRuns();
	int getNumOfTimeStamps();
    int getHighestStartedRunID();  // Karl

	bool isRunFinished(int runID);
	QTime getRunTime(int ID);
	
	QList<TimeStamp*> getTimeStampsInInterval(QTime lowerbound, QTime upperbound);
	QTime getBeginTime();
	QTime getEndTime();
    void setModified(bool m); // solte nur in ausnahmefällen public verwendet werden
	
	
	QList<RunData*> getNextBeginningRuns(int maxSize);
	QList<RunData*> getRunningRuns();
	QList<RunData*> getLastFinishedRuns(int maxSize);

signals:
	void runChanged(RunData* run);
	void timeStampChanged(TimeStamp* ts);
	void timeStampAdded(TimeStamp* ts);
	void boatboxBindingChanged(int boatboxID);
    void boatNameChanged(int boatID);
	void isModifiedChanged();
    void modified();
    void highestValidRunChanged(int newID);
	void manualPublished(RunData* run);
	void runStarted(RunData* run);
	void runFinished(RunData* run);

public slots:
	void runChangedSlot(RunData* run);
	void timeStampAssignedSlot(RunData* run);
	void runChangedID(int runID);
	void timeStampChangedID(int timeStampID);
	
	void boatboxBindingChangedSlot(int boatboxID);
	void boatNameChangedSlot(int boatID);

private:
	// Entries:
	QDateTime creationDate;		// the date of creation of this competition
	QString creationHost;		// the name of the computer where this competion was created
	QString description;
	int boatsPerRound;
	QStringList colors; // empty list, when colors are from server
	QString databaseHostName;	// is not used
	QString databaseName;		// is used for bewerbsID

	QList<RunData*> runs;
	QList<TimeStamp*> timeStamps;
	QMap<QTime, TimeStamp*> timeStampsMap;		// redundant to TimeStamps but is faster so search for time
	QList<Boat*> boats;
	QList<Boatbox*> boatboxes;
	
	int oldHighestValidRunID;
	
    bool myModified;
	
	QList<RunData*> sortByTimeUpwards(QList<RunData*> list, bool startOrGoal);
};

#endif // COMPETITION_H
