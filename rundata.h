#ifndef RUNDATA_H
#define RUNDATA_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

class RunData : public QObject
{
    Q_OBJECT
public:
    explicit RunData(QObject *parent = 0);
	explicit RunData(QObject *parent, QXmlStreamReader& reader);
	
	void init();

	void setID(int ID);
	void setRound(int round);
	void setBoatID(int boatID);
	void setColor(QString color);
	void setTitle1(QString title);
	void setFirstName1(QString name);
	void setLastName1(QString name);
	void setTitle2(QString title);
	void setFirstName2(QString name);
	void setLastName2(QString name);
	void setStartTimeID(int timeID);
	void setGoalTimeID(int timeID);
	void setDsq(bool flag); // disqualified flag
	void setErrors(QString errors);
	void setPublished(bool flag, bool silent=false);
	void setNotes(QString notes);
	void setFireBrigade(QString fireBrigade);
	void setValuationClass(QString valuationClass);
	void setAgePoints(QString agePoints);

	int getID();
	int getRound();
	int getBoatID();
	QString getColor();
	QString getTitle1();
	QString getFirstName1();
	QString getLastName1();
	QString getTitle2();
	QString getFirstName2();
	QString getLastName2();
	QString getFullName1();
	QString getFullName2();
	int getStartTimeID();
	int getGoalTimeID();
	bool getDSQ();
	QString getErrors();
	bool getPublished();
	QString getNotes();
	QString getFireBrigade();
	QString getValuationClass();
	QString getAgePoints();

	void writeXML(QXmlStreamWriter& writer);
	
	void startEditing();	// after this, no runChanged is emitted
	void editingFinished(); // if the run was changed in between, it is emitted now


signals:
	void runChanged(RunData* run);
	void timeStampAssigned(RunData* run);
	void manualPublished(RunData* run);
	void runStarted(RunData* run);
	void runFinished(RunData* run);

public slots:

private:
	int ID;
	int round;
	int boatID;
	QString color;
	QString title1;
	QString firstName1;
	QString lastName1;
	QString title2;
	QString firstName2;
	QString lastName2;
	int startTimeID;
	int goalTimeID;
	bool dsq; // disqualified flag
	QString errors;
	bool published;
	QString notes;
	QString fireBrigade;
	QString valuationClass;
	QString agePoints;
	
	bool editing;
	bool changed;
	
	void changeHandler();
};

#endif // RUNDATA_H
