#include "competition.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QtAlgorithms>

#include "rundata.h"
#include "timestamp.h"
#include "boat.h"
#include "boatbox.h"
#include "defs.h"

#include "mainwindow.h"
#include "infoscreen.h"

#include <QHostInfo>


Competition::Competition(QObject *parent, QString descr, int bpr, QStringList colors,
					 QString databaseHostName, QString databaseName):
	QObject(parent)
{
	this->description=descr;
	this->boatsPerRound=bpr;
	this->colors=colors;
	this->databaseHostName=databaseHostName;
	this->databaseName=databaseName;
	
	this->creationDate=QDateTime::currentDateTime();
	this->creationHost=QHostInfo::localHostName();

	// create dummy run with ID=0
	runs.append(new RunData());

	// create dummy timestamp with ID=0
	QTime dummytime;
	timeStamps.append(new TimeStamp(0, dummytime, TimeStamp::E));

	// create boats list
	for(int i=0; i<=boatsPerRound; i++)
	{
		boats.append(new Boat(this, i, ""));		// initiate name to "", ID=0 is a dummy Boat
        connect(boats.last(), SIGNAL(boatNameChanged(int)), this, SLOT(boatNameChangedSlot(int))); // forward internal signals
	}

	// create boatboxes list
	for(int i=0; i<=N_BOATBOXES; i++)
	{
		boatboxes.append(new Boatbox(this, i, 0));	// initiate to 0, ID=0 is a dummy Boatbox
        connect(boatboxes.last(), SIGNAL(boatboxBindingChanged(int)), this, SLOT(boatboxBindingChangedSlot(int))); // forward internal signals
	}
	
	setModified(true);
	
	oldHighestValidRunID=0;
}


Competition::~Competition()
{
	for(int i=0; i<runs.size(); i++)
	{
		delete runs.at(i);
	}
	for(int i=0; i<timeStamps.size(); i++)
	{
		delete timeStamps.at(i);
	}
	for(int i=0; i<boats.size(); i++)
	{
		delete boats.at(i);
	}
	for(int i=0; i<boatboxes.size(); i++)
	{
		delete boatboxes.at(i);
	}
}


bool Competition::load(QString fileName)
{

	// try to open file
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly | QFile::Text))
	{
		return false;
	}

	// clear old data but keep dummy-element
	while(runs.size()>1)
	{
		delete runs.last();
		runs.removeLast();
	}
	while(timeStamps.size()>1)
	{
		delete timeStamps.last();
		timeStamps.removeLast();
	}
	timeStampsMap.clear();
	while(boats.size()>1)
	{
		delete boats.last();
		boats.removeLast();
	}
	while(boatboxes.size()>1)
	{
		delete boatboxes.last();
		boatboxes.removeLast();
	}

	QXmlStreamReader reader(&file);


	if(reader.readNextStartElement() && reader.name()=="competition")
	{
		boatsPerRound=0;
		while(reader.readNextStartElement())
		{
			//qDebug() << reader.name();
			
			if(reader.name()=="creationDate")
			{
				creationDate=QDateTime::fromString(reader.readElementText(), "yyyy-MM-ddThh:mm");
			}
			else if(reader.name()=="creationHost")
			{
				creationHost=reader.readElementText();
			}
			else if(reader.name()=="description")
			{
				description=reader.readElementText();
			}
			else if(reader.name()=="boatsPerRound")
			{
				boatsPerRound=reader.readElementText().toInt();
			}
			else if(reader.name()=="colors")
			{
				colors=reader.readElementText().split(" ");
			}
			else if(reader.name()=="databaseHostName")
			{
				databaseHostName=reader.readElementText();
			}
			else if(reader.name()=="databaseName")
			{
				databaseName=reader.readElementText();
			}
			else if(reader.name()=="runs")
			{
				while(reader.readNextStartElement())
				{
					runs.append(new RunData(this, reader));
					connect(runs.last(), SIGNAL(runChanged(RunData*)), this, SLOT(runChangedSlot(RunData*)));
					connect(runs.last(), SIGNAL(timeStampAssigned(RunData*)), this, SLOT(timeStampAssignedSlot(RunData*)));
					connect(runs.last(), SIGNAL(manualPublished(RunData*)), this, SIGNAL(manualPublished(RunData*)));
					connect(runs.last(), SIGNAL(runStarted(RunData*)), this, SIGNAL(runStarted(RunData*)));
					connect(runs.last(), SIGNAL(runFinished(RunData*)), this, SIGNAL(runFinished(RunData*)));
				}
			}
			else if(reader.name()=="timeStamps")
			{
				while(reader.readNextStartElement())
				{
					timeStamps.append(new TimeStamp(this, reader));
					connect(timeStamps.last(), SIGNAL(runChangedID(int)), this, SLOT(runChangedID(int)));
					connect(timeStamps.last(), SIGNAL(timeStampChangedID(int)), this, SLOT(timeStampChangedID(int)));
				}
			}
			else if(reader.name()=="boats")
			{
				while(reader.readNextStartElement())
				{
					if(reader.name()=="boat")
					{
						if(reader.attributes().value(reader.namespaceUri().toString(),"boatID").toString().toInt() == boats.size())
						{
							if(reader.readNextStartElement() && reader.name()=="boatName")
							{
								boats.append(new Boat(this, boats.size(), reader.readElementText()));
								connect(boats.last(), SIGNAL(boatNameChanged(int)), this, SLOT(boatNameChangedSlot(int)));

								reader.readNextStartElement();		// this will fail as there is only one boatName-Element in a boat-Element, this is just to leave the boatName-Element
							}
							else
							{
								reader.raiseError(tr("Kein gültiges boatName-Element, Zeile: ") + QString::number(reader.lineNumber()));
							}
						}
						else
						{
							reader.raiseError(tr("ungültige boatID, Zeile: ") + QString::number(reader.lineNumber()));
						}
					}
					else
					{
						reader.raiseError(tr("Kein gültiges boat-Element, Zeile: ") + QString::number(reader.lineNumber()));
					}
				}
			}
			else if(reader.name()=="boatboxes")
			{
				while(reader.readNextStartElement())
				{
					if(reader.name()=="boatbox")
					{
						if(reader.attributes().value(reader.namespaceUri().toString(),"boatboxID").toString().toInt() == boatboxes.size())
						{
							if(reader.readNextStartElement() && reader.name()=="boatID")
							{
								boatboxes.append(new Boatbox(this, boatboxes.size(), reader.readElementText().toInt()));
								connect(boatboxes.last(), SIGNAL(boatboxBindingChanged(int)), this, SLOT(boatboxBindingChangedSlot(int)));

								reader.readNextStartElement();		// this will fail as there is only one boatID-Element in a boatbox-Element, this is just to leave the boatbox-Element
							}
							else
							{
								reader.raiseError(tr("Kein gültiges boatID-Element, Zeile: ") + QString::number(reader.lineNumber()));
							}
						}
						else
						{
							reader.raiseError(tr("ungültige boatbox-ID, Zeile: ") + QString::number(reader.lineNumber()));
						}
					}
					else
					{
						reader.raiseError(tr("Kein gültiges boatbox-Element, Zeile: ") + QString::number(reader.lineNumber()));
					}
				}
			}
			else
			{
				qDebug() << "unknown XML-Tag:" << reader.name();
				reader.readElementText();
				// Das bringt nichts, da nach einem raiseError das Lesen abbricht und dann auch die boatsPerRound Prüfung schief geht. Diese Meldung wird dann überschrieben.
				//reader.raiseError(tr("unbekanntes XML-Tag: ") + reader.name().toString() + tr(", Zeile: ") + QString::number(reader.lineNumber()));
			}
		}
		if(boatsPerRound==0)
		{
			// this value might not have been read
			reader.raiseError(tr("Bewerb hat ungültigen Wert für 'boatsPerRound'!"));
		}
	}
	else
	{
		reader.raiseError(tr("Dies ist keine Bewerbsdatei!"));
	}

	file.close();

	if(reader.hasError())
	{
		MainWindow::app()->infoscreen()->appendError(tr("Fehler beim Parsen der Datei: ") + reader.errorString());
		return false;
	}
	else if(file.error() != QFile::NoError)
	{
		return false;
	}
	
	
	// copy data to timeStampsMap
	for(int i=1; i<timeStamps.size(); i++)
	{
		timeStampsMap.insertMulti(timeStamps.at(i)->getTime(), timeStamps.at(i));
	}

	qDebug("file loaded");
	
	setModified(false);
	

	return true;
}


// does not change the modified-flag
bool Competition::save(QString fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::WriteOnly | QFile::Text))
	{
		return false;
	}

	QXmlStreamWriter writer(&file);
	writer.setAutoFormatting(true);
	writer.setCodec("UTF-8");
	writer.writeStartDocument();
	writer.writeStartElement("competition");

	
	writer.writeTextElement("creationDate", creationDate.toString("yyyy-MM-ddThh:mm"));
	writer.writeTextElement("creationHost", creationHost);
	writer.writeTextElement("description", description);
	writer.writeTextElement("boatsPerRound", QString::number(boatsPerRound));
	writer.writeTextElement("colors", colors.join(" "));
	writer.writeTextElement("databaseHostName", databaseHostName);
	writer.writeTextElement("databaseName", databaseName);

	// write table of runs
	writer.writeStartElement("runs");
	for(int i=1; i<runs.size(); i++)
	{
		runs.at(i)->writeXML(writer);
	}
	writer.writeEndElement();	// runs

	// write table of timeStamps
	writer.writeStartElement("timeStamps");
	for(int i=1; i<timeStamps.size(); i++)
	{
		timeStamps.at(i)->writeXML(writer);
	}
	writer.writeEndElement();	// timestamps

	// write table of boats
	writer.writeStartElement("boats");
	for(int i=1; i<boats.size(); i++)
	{
		writer.writeStartElement("boat");
		writer.writeAttribute("boatID", QString::number(i));
		writer.writeTextElement("boatName", boats.at(i)->getName());
		writer.writeEndElement();
	}
	writer.writeEndElement();	// boats

	// write table of boatboxes
	writer.writeStartElement("boatboxes");
	for(int i=1; i<boatboxes.size(); i++)
	{
		writer.writeStartElement("boatbox");
		writer.writeAttribute("boatboxID", QString::number(i));
		writer.writeTextElement("boatID", QString::number(boatboxes.at(i)->getBoatID()));
		writer.writeEndElement();
	}
	writer.writeEndElement();	// boatboxes

	// write table of boatboxes

	writer.writeEndElement();	// competition
	writer.writeEndDocument();
	
	file.close();

	qDebug("file saved");
	
	return true;
}


bool Competition::exportCSV(QString fileName)
{
	//qDebug() << "write CSV";
	QFile file(fileName);
	if(!file.open(QFile::WriteOnly | QFile::Text))
	{
		return false;
	}
	
	QTextStream out(&file);
	out.setCodec("UTF-8");
	
	QString sep=";";		// seperator: semicolon
	QString linesep="\n";	// line seperator: linefeed
	
	// print header
	out << "Bewerb am: " << creationDate.toString("yyyy-MM-dd,hh:mm") << linesep;
	out << "erstellt auf Rechner: " << creationHost << linesep;
	out << "Beschreibung: " << description << linesep;
	out << "*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!*~!" << linesep;
	
	// print table header
	out << "Bewerbsnummer"<<sep<<"Durchgang"<<sep<<"Zille"<<sep<<"Farbe"<<sep<<"Dienstgrad 1"<<sep<<"Vorname 1"<<sep<<"Nachname 1"<<sep<<"Dienstgrad 2"<<sep<<"Vorname 2"<<sep<<"Nachname 2"<<sep
        <<"Startzeit"<<sep<<"Zielzeit"<<sep<<"Laufzeit"<<sep<<"DSQ"<<sep<<"Fehler"<<sep<<"Anmerkungen"<<sep<<"Feuerwehrverband"<<sep<<"Wertungsklasse"<<sep<<"Alterspunkte"<<sep<<"Publiziert"<<linesep;
	
	// print table lines
	for(int i=1; i<runs.size(); i++)
	{
		RunData* r=runs.at(i);
		TimeStamp* tsStart=getTimeStamp(r->getStartTimeID());
		TimeStamp* tsGoal=getTimeStamp(r->getGoalTimeID());
		
		out << r->getID()<<sep;
		out << r->getRound()<<sep;
		out << r->getBoatID()<<sep;
		out << r->getColor()<<sep;
		out << r->getTitle1()<<sep<<r->getFirstName1()<<sep<<r->getLastName1()<<sep;
		out << r->getTitle2()<<sep<<r->getFirstName2()<<sep<<r->getLastName2()<<sep;
		
		if(tsStart!=0)
		{
			out << tsStart->getTime().toString("hh:mm:ss.zzz").left(11);
		}
		out << sep;
		
		if(tsGoal!=0)
		{
			out << tsGoal->getTime().toString("hh:mm:ss.zzz").left(11);
		}
		out << sep;
		
		QTime runTime=getRunTime(i);
		if(runTime.isValid())
		{
			out << runTime.toString("hh:mm:ss.zzz").left(11);
		}
		out << sep;
		
		if(r->getDSQ())
		{
			out << "X";
		}
		out << sep;
		
		out << r->getErrors()<<sep;
		out << r->getNotes()<<sep;
		out << r->getFireBrigade()<<sep;
		out << r->getValuationClass()<<sep;
        out << r->getAgePoints()<<sep;

        if(r->getPublished())
        {
            out << "X";
        }
        out<<linesep;
	}
	
	file.close();
	
	//qDebug() << "CSV written";
	return true;
}



void Competition::setDescription(QString description)
{
	if(description!=this->description)
	{
		this->description=description;
		setModified(true);
		
	}
}

void Competition::setBoatsPerRound(int bpr)
{
	if(runs.size()==1)
	{
		if(bpr!=boatsPerRound)
		{
			boatsPerRound=bpr;
			setModified(true);
			
		}
	}
	else
	{
		MainWindow::app()->infoscreen()->appendError(tr("Bewerb hat bereits begonnen, Anzahl der Zillen"
												"pro Durchgang kann nicht mehr verändert werden!"));
	}
}

void Competition::setColors(QStringList colors)
{
	if(runs.size()==1) // if only dummy-element exists
	{
		this->colors=colors;
		setModified(true);
		
	}
	else
	{
		MainWindow::app()->infoscreen()->appendError(tr("Bewerb hat bereits begonnen, Farbenfolge\n"
													"kann nicht mehr verändert werden!"));
	}
}

void Competition::setDatabaseHostName(QString databaseHostName)
{
	this->databaseHostName=databaseHostName;
	setModified(true);	
}

void Competition::setDatabaseName(QString databaseName)
{
	if(databaseName!=this->databaseName)
	{
		this->databaseName=databaseName;
		setModified(true);
	}
}


QDateTime Competition::getCreationDate()
{
	return creationDate;
}

QString Competition::getCreationHost()
{
	return creationHost;
}

QString Competition::getDescription()
{
	return description;
}

int Competition::getBoatsPerRound()
{
	return boatsPerRound;
}

QStringList Competition::getColors()
{
	return colors;
}

QString Competition::getDatabaseHostName()
{
	return databaseHostName;
}

QString Competition::getDatabaseName()
{
	return databaseName;
}

void Competition::addRun(RunData* run)
{
	// insert new run with next ID
	run->setParent(this);
	run->setID(runs.size());
	run->setRound((run->getID()-1)/boatsPerRound+1);
	run->setBoatID((run->getID()-1)%boatsPerRound+1);
	if(colors.size()>0){
		run->setColor(colors.at((run->getRound()-1)%colors.size()));
	}

	connect(run, SIGNAL(runChanged(RunData*)), this, SLOT(runChangedSlot(RunData*)));
	connect(run, SIGNAL(timeStampAssigned(RunData*)), this, SLOT(timeStampAssignedSlot(RunData*)));
	connect(run, SIGNAL(manualPublished(RunData*)), this, SIGNAL(manualPublished(RunData*)));
	connect(run, SIGNAL(runStarted(RunData*)), this, SIGNAL(runStarted(RunData*)));
	connect(run, SIGNAL(runFinished(RunData*)), this, SIGNAL(runFinished(RunData*)));
	runs.append(run);

	emit runChanged(run);
	setModified(true);
	
    //qDebug(QString("addRun(ID = %1)").arg(run->getID()).toAscii());
}


void Competition::addTimeStamp(TimeStamp* timeStamp)
{
	timeStamp->setParent(this);
	timeStamp->setID(timeStamps.size());
	
	connect(timeStamp, SIGNAL(runChangedID(int)), this, SLOT(runChangedID(int)));
	connect(timeStamp, SIGNAL(timeStampChangedID(int)), this, SLOT(timeStampChangedID(int)));
	timeStamps.append(timeStamp);
	timeStampsMap.insertMulti(timeStamp->getTime(), timeStamp);
	
	emit timeStampChanged(timeStamp);
	emit timeStampAdded(timeStamp);
	setModified(true);
}


// use boatID=0 for unbinding!
void Competition::bindBox2BoatID(int boatboxID, int boatID)
{
	if(boatboxID>0 && boatboxID<boatboxes.size())
	{
		// clear all possible bindings from other boatboxes to the same boatID
		for(int i=1; i<boatboxes.size(); i++)
		{
			if(boatboxes.at(i)->getBoatID()==boatID)
			{
				boatboxes.at(i)->setBoatID(0);
			}
		}

		// set new binding from boatboxID to boatID
		boatboxes.at(boatboxID)->setBoatID(boatID);
		
		setModified(true);
		
	}
}


void Competition::setBoatName(int boatID, QString boatName)
{
	if(boatID>0 && boatID<boats.size())
	{
		// for all possible boatIDs with this name, clear the name
		for(int i=1; i<boats.size(); i++)
		{
			if(boats.at(i)->getName()==boatName)
			{
				boats.at(i)->setName("");
			}
		}

		// set new name for boatID
		boats.at(boatID)->setName(boatName);
		
		setModified(true);
		
	}
}


int Competition::getBoatIdOfBoatbox(int boatboxID)
{
	if(boatboxID>0 && boatboxID<boatboxes.size())
	{
		return boatboxes.at(boatboxID)->getBoatID();
	}
	else
	{
		return 0;
	}
}

int Competition::getBoatboxIdOfBoat(int boatID)
{
	if(boatID>0 && boatID<boats.size())
	{
		for(int n=1; n<boatboxes.size(); n++){
			if(boatboxes.at(n)->getBoatID() == boatID){
				return n;
			}
		}
	}
	return 0;
}

QString Competition::getNameOfBoatID(int boatID)
{
	if(boatID>0 && boatID<boats.size())
	{
		return boats.at(boatID)->getName();
	}
	else
	{
		return "";
	}
}

// a run is valid, when both times are assigned
// it is used for laoding new runs from the data base or add dummy runs to the mainTable
int Competition::getHighestValidRunID()
{
	int ID=0;
    for(int i=1; i<runs.size(); i++)
	{
		if(runs.at(i)->getStartTimeID()!=0 || runs.at(i)->getGoalTimeID()!=0)
		{
			// this run is 'valid'
			ID=i;
		}
	}
	
	return ID;
}

QString Competition::getBoatName(int boatID){
	if(boatID >= 1 && boatID < boats.size()){
		return boats.at(boatID)->getName();
	}
	return QString();
}

int Competition::getNumOfRuns()
{
	return runs.size()-1;		// there is one dummy element
}


int Competition::getNumOfTimeStamps()
{
    return timeStamps.size()-1;		// there is one dummy element
}

// a run is started, when start time is assigned
// it is used for the automatic assignment  in the assignmentmanager
int Competition::getHighestStartedRunID() // TODO: improve search by remember the last result.
    {
        int ID=0;
        for(int i=1; i<runs.size(); i++)
        {
            if(runs.at(i)->getStartTimeID()!=0 || runs.at(i)->getGoalTimeID()!=0)
            {
                // this run is 'valid'
                ID=i;
            }
        }

        return ID;
    }


RunData* Competition::getRun(int ID)
{
	if(ID>0 && ID<runs.size())
	{
		return runs.at(ID);
	}
	else
	{
		return 0;
	}
}


TimeStamp* Competition::getTimeStamp(int ID)
{
	if(ID>0 && ID<timeStamps.size())
	{
		return timeStamps.at(ID);
	}
	else
	{
		return 0;
	}
}

void Competition::runChangedSlot(RunData* run)
{
	if(run!=0)
	{
		//qDebug() << "Competition: runChanged";
		emit runChanged(run);
		setModified(true);
	}
}

void Competition::timeStampAssignedSlot(RunData* run)
{
	if(run!=0)
	{
		int ID=getHighestValidRunID();
		if(ID!=oldHighestValidRunID)
		{
			oldHighestValidRunID=ID;
			emit highestValidRunChanged(ID);
		}
	}
}

void Competition::runChangedID(int runID)
{
	if(runID>0 && runID<runs.size())
	{
		emit runChanged(runs.at(runID));
		setModified(true);
	}
}

void Competition::timeStampChangedID(int timeStampID)
{
	if(timeStampID>0 && timeStampID<timeStamps.size())
	{
		/*
		 * If the time of a timestamp was changed, the key in timeStampsMap is wrong and must be refreshed too.
		 */
		
		// get timestamp
		TimeStamp* changedTs=timeStamps.at(timeStampID);
		
		// get the time-key of TimeStamp that was changed in the map
		QList<QTime> list=timeStampsMap.keys(changedTs);
		
		// check if there is exactly 1 time fitting to this timestamp
		if(list.size()!=1)
		{
			qDebug("ERROR: invalid pair in timeStampMap");
			// continue anyway
		}
		
		// get old time
		QTime oldTime=list.first();
		
		// take all timestamps with old time from map and store them in temp
		QList<TimeStamp*> temp;
		while(timeStampsMap.contains(oldTime))
		{
			TimeStamp* ts=timeStampsMap.take(oldTime);
			temp.append(ts);
		}
		
		// put all the timestamps from temp and put them back in map with new keys
		for(int i=0; i<temp.size(); i++)
		{
			TimeStamp* ts=temp.at(i);
			timeStampsMap.insertMulti(ts->getTime(), ts);
		}
		
		emit timeStampChanged(changedTs);
		
		setModified(true);
		
	}
}


void Competition::boatboxBindingChangedSlot(int boatboxID)
{
	if(boatboxID>0 && boatboxID<boatboxes.size())
	{
		emit boatboxBindingChanged(boatboxID);
		setModified(true);
		
	}
}


void Competition::boatNameChangedSlot(int boatID)
{
	if(boatID>0 && boatID<boats.size())
	{
		emit boatNameChanged(boatID);
		setModified(true);
		
	}
}


bool Competition::isRunFinished(int runID)
{
	if(runID>0 && runID<runs.size() && runs.at(runID)->getStartTimeID()!=0 && runs.at(runID)->getGoalTimeID()!=0){
		return true;
	}else{
		return false;
	}
}


QTime Competition::getRunTime(int ID)
{
	//qDebug() << "getRunTime...";
	QTime runTime;		// default: invalid time
	
	RunData* run=getRun(ID);
	if(run!=0)
	{
		TimeStamp* tsStart=getTimeStamp(run->getStartTimeID());
		TimeStamp* tsGoal=getTimeStamp(run->getGoalTimeID());
		if(tsStart!=0 && tsGoal!=0)
		{
			QTime startTime=tsStart->getTime();
			QTime goalTime=tsGoal->getTime();
			if(goalTime>startTime)		// only positive runTimes are valid
			{
				runTime = runTime.addMSecs(startTime.msecsTo(goalTime)); // build difference
			}
		}
	}
	//qDebug() << "getRunTime...done";
	
	return runTime;
}


QList<TimeStamp*> Competition::getTimeStampsInInterval(QTime lowerbound, QTime upperbound)
{
	QList<TimeStamp*> list;
	QMap<QTime, TimeStamp*>::const_iterator i = timeStampsMap.lowerBound(lowerbound);
    QMap<QTime, TimeStamp*>::const_iterator upperBound = timeStampsMap.upperBound(upperbound);
    while (i != upperBound)
	{
		list.append(i.value());
        ++i;
    }
	
	return list;
}


QTime Competition::getBeginTime()
{
	if(!timeStampsMap.isEmpty())
	{
		QMap<QTime, TimeStamp*>::const_iterator i=timeStampsMap.begin();
		return i.value()->getTime();
	}
	else
	{
		return QTime();
	}
}


QTime Competition::getEndTime()
{
	if(!timeStampsMap.isEmpty())
	{
		QMap<QTime, TimeStamp*>::const_iterator i=timeStampsMap.end();
		i--;
		return i.value()->getTime();
	}
	else
	{
		return QTime();
	}
}


// todo: test this function and use it in assignmentmanager on replacing timestamps.
void Competition::unassignTimeStamp(int tsID)
{
    if(tsID > 0){
        TimeStamp* ts = getTimeStamp(tsID);
        int runID = ts->getRunID();

        if(runID > 0){
            RunData* run = getRun(runID);

            // remove timestamp from run
            if(run->getStartTimeID() == tsID){
                run->setStartTimeID(0);
            }
            if(run->getGoalTimeID() == tsID){
                run->setGoalTimeID(0);
            }

            // remove run from timestamp
            ts->setRunID(0);
            ts->setBoatID(0);
            ts->setBoatName("");
        }
    }
	setModified(true);

}


bool Competition::isModified()
{
    return myModified;
}


void Competition::setModified(bool m)
{
    if(m!=myModified)
	{
        myModified=m;
		//qDebug() << "Competition: modifiedChanged:" << modified;
        emit isModifiedChanged();
	}

    if(m){
        emit modified();
    }
}


// returns a list of the next starting runs
// the list is not longer than maxSize
// the first run in the list starts next
QList<RunData*> Competition::getNextBeginningRuns(int maxSize)
{
	QList<RunData*> list;
	// find last started run beginning from the end of the list
	for(int i=runs.size()-1; i>=0; i--)
	{
		// it has a start time, or we are at the top, so the next one is the next to start
		if(runs.at(i)->getStartTimeID()!=0 || i==0)
		{
			i++;
			// put the next ones in the list
			for(int j=i; j<runs.size() && list.size()<maxSize; j++)
			{
				list.append(runs.at(j));
			}
			break;
		}
	}
	return list;
}


// returns a list of all running runs
// the first run in the list started at first
QList<RunData*> Competition::getRunningRuns()
{
	QList<RunData*> list;
	// find last started run beginning from the end of the list
	for(int i=runs.size()-1; i>0; i--)
	{
		
		// only select unfinished runs
		if(runs.at(i)->getStartTimeID()!=0 && runs.at(i)->getGoalTimeID()==0)
		{
			list.prepend(runs.at(i));
		}
	}
	
	// sort by start time
	return sortByTimeUpwards(list, true);
}


// returns a list of the last finished runs
QList<RunData*> Competition::getLastFinishedRuns(int maxSize)
{
	QList<RunData*> list;
	
	// look back the last 3 rounds and find all finished runs
	for(int i=runs.size()-1; i>0 && list.size()<maxSize; i--)
	{
		if(isRunFinished(i))
		{
			list.append(runs.at(i));
		}
	}
	
	/*
	// look back the last 3 rounds and find all finished runs
	for(int i=runs.size()-1; i>0 && list.size()<3*boatsPerRound; i--)
	{
		if(isRunFinished(i))
		{
			list.prepend(runs.at(i));
		}
	}
	
	// sort by goal time
	list=sortByTimeUpwards(list, false);
	
	// only take the last maxSize finished runs
	while(list.size()>maxSize)
	{
		list.removeFirst();
	}
	*/
	
	return list;
}



// sort a QList of Runs out of place by start or goal time
// startOrGoal=true means sort by start time, false means sort by goal time
QList<RunData*> Competition::sortByTimeUpwards(QList<RunData*> list, bool startOrGoal)
{
	QList<RunData*> result;
	while(!list.isEmpty())
	{
		QTime smallestTime=QTime(23,59,59);
		RunData* smallestRun=0;
		for(int i=0; i<list.size(); i++)
		{
			int ts;
			if(startOrGoal)
			{
				ts=list.at(i)->getStartTimeID();
			}
			else
			{
				ts=list.at(i)->getGoalTimeID();
			}
			
			if(ts!=0)
			{
				if(timeStamps.at(ts)->getTime()<smallestTime)
				{
					smallestTime=timeStamps.at(ts)->getTime();
					smallestRun=list.at(i);
				}
			}
		}
		list.removeAll(smallestRun);
		result.prepend(smallestRun);
	}
	return result;
}

