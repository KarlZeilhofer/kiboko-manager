#include "timestamp.h"
#include <QStringList>

QStringList TimeStamp::sourceStrings=QStringList()<<"LS"<<"LGL"<<"LGR"<<"TSS"<<"TSGL"<<"TSGR"<<"M"<<"E";

TimeStamp::TimeStamp(QObject *parent, QTime time, Source source) :
    QObject(parent)
{
	ID=0;
	this->time=time;
	this->source=source;
	runID=0;
	boatboxID=0;
	boatID=0;
	boatName="";

}

TimeStamp::TimeStamp(QObject *parent, QXmlStreamReader& reader):
	QObject(parent)
{
	ID=0;
	time=QTime();
	source=(Source)(-1);
	runID=0;
	boatboxID=0;
	boatID=0;
	boatName="";

	ID=reader.attributes().value("ID").toString().toInt();
	while(reader.readNextStartElement())
	{
		if(reader.name()=="time")
		{
			time=QTime::fromString(reader.readElementText(), "hh:mm:ss:zzz");
		}
		else if(reader.name()=="source")
		{
			QString s=reader.readElementText();
			// search for source string
			for(int i=0; i<sourceStrings.size(); i++)
			{
				if(s==sourceStrings.at(i))
				{
					// string found
					source=(Source)i;
					break;
				}
			}
			if(source==-1)
			{
				// string not found
				reader.raiseError(tr("ungültige Zeitquelle, Zeile: ") + QString::number(reader.lineNumber()));
			}
		}
		else if(reader.name()=="runID")
		{
			runID=reader.readElementText().toInt();
		}
		else if(reader.name()=="boatboxID")
		{
			boatboxID=reader.readElementText().toInt();
		}
		else if(reader.name()=="boatID")
		{
			boatID=reader.readElementText().toInt();
		}
		else if(reader.name()=="boatName")
		{
			boatName=reader.readElementText();
		}
		else
		{
			reader.raiseError(tr("Kein gültiges XML-Tag, Zeile: ") + QString::number(reader.lineNumber()));
		}
	}

	if(!time.isValid())
	{
		reader.raiseError(tr("ungültige Zeit"));
	}
}

//########################################
//########### SETTERS ####################
//########################################
void TimeStamp::setID(int ID)
{
	this->ID=ID;
}

void TimeStamp::setTime(QTime time)
{
	if(time.isValid() && time!=this->time)
	{
		this->time=time;
		if(runID!=0)
		{
			emit runChangedID(runID);
		}
		emit timeStampChangedID(this->ID);
	}
}

void TimeStamp::setSource(Source source)
{
	if(source!=this->source)
	{
		this->source=source;
		emit timeStampChangedID(this->ID);
	}
}

void TimeStamp::setRunID(int runID)
{
	if(this->runID!=runID)
	{
		this->runID=runID;
		emit timeStampChangedID(this->ID);
	}
}

void TimeStamp::setBoatboxID(int boatboxID)
{
	if(boatboxID!=this->boatboxID)
	{
		this->boatboxID=boatboxID;
		emit timeStampChangedID(this->ID);
	}
}

void TimeStamp::setBoatID(int boatID)
{
	if(boatID!=this->boatID)
	{
		this->boatID=boatID;
		emit timeStampChangedID(this->ID);
	}
}

void TimeStamp::setBoatName(QString boatName)
{
	if(boatName!=this->boatName)
	{
		this->boatName=boatName;
		emit timeStampChangedID(this->ID);
	}
}




//########################################
//########### GETTERS ####################
//########################################
int TimeStamp::getID()
{
	return ID;
}

QTime TimeStamp::getTime()
{
	return time;
}

TimeStamp::Source TimeStamp::getSource()
{
	return source;
}

QString TimeStamp::getSourceName()
{
	return sourceStrings.at(source);
}

int TimeStamp::getRunID()
{
	return runID;
}

int TimeStamp::getBoatboxID()
{
	return boatboxID;
}

int TimeStamp::getBoatID()
{
	return boatID;
}

QString TimeStamp::getBoatName()
{
	return boatName;
}


void TimeStamp::writeXML(QXmlStreamWriter& writer)
{
	writer.writeStartElement("timeStamp");
	writer.writeAttribute("ID", QString::number(ID));

	writer.writeTextElement("time", time.toString("hh:mm:ss:zzz"));
	writer.writeTextElement("source", sourceStrings.at(source));
	writer.writeTextElement("runID", QString::number(runID));
	writer.writeTextElement("boatboxID", QString::number(boatboxID));
	writer.writeTextElement("boatID", QString::number(boatID));
	writer.writeTextElement("boatName", boatName);

	writer.writeEndElement();	// timeStamp
}
