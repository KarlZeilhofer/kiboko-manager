#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

/*
  use .value() for indexing, if you are not sure, if the ID exists.
*/

#include <QTime>
#include <QString>

class TimeStamp : public QObject
{
    Q_OBJECT
public:
	enum Source{
		LS, // light sensor start
		LGL, LGR,  // light sensor goal (left, right)
		TSS, // trigger-station start
		TSGL, TSGR, // trigger-station goal (left, right)
		M, // manual
		E // edited
	};

	static QStringList sourceStrings;

	explicit TimeStamp(QObject *parent, QTime time, Source source);
	explicit TimeStamp(QObject *parent, QXmlStreamReader& reader);

	void setID(int ID);
	void setTime(QTime time);
	void setSource(Source source);
	void setRunID(int runID);
	void setBoatboxID(int boatboxID);
	void setBoatID(int boatID);
	void setBoatName(QString boatName);

	int getID();
	QTime getTime();
	Source getSource();
	QString getSourceName();
	int getRunID();
	int getBoatboxID();
	int getBoatID();
	QString getBoatName();

	void writeXML(QXmlStreamWriter& writer);

signals:
	void runChangedID(int runID);
	void timeStampChangedID(int tsID);

public slots:

private:
	int ID;
	QTime time;
	Source source;
	int runID;
	int boatboxID; // 1,...,N_BOATBOXES, ID of the hardware box
	int boatID; // typ. 1,...,15, ID of the referenced boatnumber in the main table
	QString boatName; // typ. 1,...,15,A,B,C,...
};

#endif // TIMESTAMP_H
