#include "rundata.h"
#include <QDebug>

RunData::RunData(QObject *parent) :
    QObject(parent)
{
	init();

}

RunData::RunData(QObject *parent, QXmlStreamReader& reader):
	QObject(parent)
{
	init();
	ID=reader.attributes().value("ID").toString().toInt();
	while(reader.readNextStartElement())
	{
		if(reader.name()=="round")
		{
			round=reader.readElementText().toInt();
		}
		else if(reader.name()=="boatID")
		{
			boatID=reader.readElementText().toInt();
		}
		else if(reader.name()=="color")
		{
			color=reader.readElementText();
		}
		else if(reader.name()=="title1")
		{
			title1=reader.readElementText();
		}
		else if(reader.name()=="firstName1")
		{
			firstName1=reader.readElementText();
		}
		else if(reader.name()=="lastName1")
		{
			lastName1=reader.readElementText();
		}
		else if(reader.name()=="title2")
		{
			title2=reader.readElementText();
		}
		else if(reader.name()=="firstName2")
		{
			firstName2=reader.readElementText();
		}
		else if(reader.name()=="lastName2")
		{
			lastName2=reader.readElementText();
		}
		else if(reader.name()=="startTimeID")
		{
			startTimeID=reader.readElementText().toInt();
		}
		else if(reader.name()=="goalTimeID")
		{
			goalTimeID=reader.readElementText().toInt();
		}
		else if(reader.name()=="dsq")
		{
			dsq=reader.readElementText()=="true"?true:false;
		}
		else if(reader.name()=="errors")
		{
			errors=reader.readElementText();
		}
		else if(reader.name()=="published")
		{
			// ignore the published flag, it defaults to false
			//published=reader.readElementText()=="true"?true:false;
			reader.readElementText();
			published=false;
		}
		else if(reader.name()=="notes")
		{
			notes=reader.readElementText();
		}
		else if(reader.name()=="fireBrigade")
		{
			fireBrigade=reader.readElementText();
		}
		else if(reader.name()=="valuationClass")
		{
			valuationClass=reader.readElementText();
		}
		else if(reader.name()=="agePoints")
		{
			agePoints=reader.readElementText();
		}
		else
		{
			reader.raiseError(tr("unbekanntes XML-Tag innerhalb von 'run', Zeile: ") + QString::number(reader.lineNumber()));
		}
	}

	if(ID==0)
	{
		reader.raiseError(tr("Lauf hat ungültige ID!"));
	}
}


void RunData::init()
{
	ID=0;
	round=0;
	boatID=0;
	color="";
	title1="";
	firstName1="";
	lastName1="";
	title2="";
	firstName2="";
	lastName2="";
	startTimeID=0;
	goalTimeID=0;
	dsq=false; // disqualified flag
	errors="";
	published=false;
	notes="";
	fireBrigade="";
	valuationClass="";
	
	editing=false;
	changed=false;
}


void RunData::setID(int ID)
{
	if(this->ID!=ID)
	{
		this->ID=ID;
		changeHandler();
	}
}

void RunData::setRound(int round)
{
	if(this->round!=round)
	{
		this->round=round;
		changeHandler();
	}
}

void RunData::setBoatID(int boatID)
{
	if(this->boatID!=boatID)
	{
		this->boatID=boatID;
		changeHandler();
	}
}

void RunData::setColor(QString color)
{
	if(this->color!=color){
		this->color=color;
		changeHandler();
	}
}

void RunData::setTitle1(QString title)
{
	if(this->title1!=title){
		this->title1=title;
		changeHandler();
	}
}

void RunData::setFirstName1(QString name)
{
	if(this->firstName1!=name){
		this->firstName1=name;
		changeHandler();
	}
}

void RunData::setLastName1(QString name)
{
	if(this->lastName1!=name){
		this->lastName1=name;
		changeHandler();
	}
}

void RunData::setTitle2(QString title)
{
	if(this->title2!=title){
		this->title2=title;
		changeHandler();
	}
}

void RunData::setFirstName2(QString name)
{
	if(this->firstName2!=name){
		this->firstName2=name;
		changeHandler();
	}
}

void RunData::setLastName2(QString name)
{
	if(this->lastName2!=name){
		this->lastName2=name;
		changeHandler();
	}
}

void RunData::setStartTimeID(int timeID)
{
	//qDebug() << "setStartTimeID";
	if(this->startTimeID!=timeID){
		if(startTimeID==0 && goalTimeID==0)	// if the previous starttime was invalid and there is no goaltime yet
		{
			emit runStarted(this);
		}
		this->startTimeID=timeID;
		changeHandler();
		emit timeStampAssigned(this);
	}
}

void RunData::setGoalTimeID(int timeID)
{
	//qDebug() << "setGoalTimeID";
	if(this->goalTimeID!=timeID){
		if(startTimeID!=0 && goalTimeID==0)	// if there is a starttime and the previous goaltime is invalid
		{
			emit runFinished(this);
		}
		this->goalTimeID=timeID;
		changeHandler();
		emit timeStampAssigned(this);
	}
}

void RunData::setDsq(bool flag)
{
	if(this->dsq!=flag){
		this->dsq=flag;
		changeHandler();
	}
}

void RunData::setErrors(QString errors)
{
	if(this->errors!=errors){
		this->errors=errors;
		changeHandler();
	}
}

void RunData::setPublished(bool flag, bool silent)
{
	if(this->published!=flag){
		this->published=flag;
		changeHandler();
		
		// don't this a signal when silent flag is set. (used for auto publish)
		if(flag && !silent)
		{
			emit manualPublished(this);
		}
	}
}

void RunData::setNotes(QString notes)
{
	if(this->notes!=notes){
		this->notes=notes;
		changeHandler();
	}
}

void RunData::setFireBrigade(QString fireBrigade)
{
	if(this->fireBrigade!=fireBrigade){
		this->fireBrigade=fireBrigade;
		changeHandler();
	}
}

void RunData::setValuationClass(QString valuationClass)
{
	if(this->valuationClass!=valuationClass){
		this->valuationClass=valuationClass;
		changeHandler();
	}
}

void RunData::setAgePoints(QString agePoints)
{
	if(this->agePoints!=agePoints){
		this->agePoints=agePoints;
		changeHandler();
	}
}


/* getters */
int RunData::getID()
{
	return ID;
}

int RunData::getRound()
{
	return round;
}

int RunData::getBoatID()
{
	return boatID;
}

QString RunData::getColor()
{
	return color;
}

QString RunData::getTitle1()
{
	return title1;
}

QString RunData::getFirstName1()
{
	return firstName1;
}

QString RunData::getLastName1()
{
	return lastName1;
}

QString RunData::getTitle2()
{
	return title2;
}

QString RunData::getFirstName2()
{
	return firstName2;
}

QString RunData::getLastName2()
{
	return lastName2;
}

QString RunData::getFullName1()
{
	return title1+" "+firstName1+" "+lastName1;
}

QString RunData::getFullName2()
{
	return title2+" "+firstName2+" "+lastName2;
}

int RunData::getStartTimeID()
{
	return startTimeID;
}

int RunData::getGoalTimeID()
{
	return goalTimeID;
}

bool RunData::getDSQ()
{
	return dsq;
}

QString RunData::getErrors()
{
	return errors;
}

bool RunData::getPublished()
{
	return published;
}

QString RunData::getNotes()
{
	return notes;
}

QString RunData::getFireBrigade()
{
	return fireBrigade;
}

QString RunData::getValuationClass()
{
	return valuationClass;
}

QString RunData::getAgePoints()
{
	return agePoints;
}


void RunData::writeXML(QXmlStreamWriter& writer)
{
	writer.writeStartElement("run");
	writer.writeAttribute("ID", QString::number(ID));

	writer.writeTextElement("round", QString::number(round));
	writer.writeTextElement("boatID", QString::number(boatID));
	writer.writeTextElement("color", color);
	writer.writeTextElement("title1", title1);
	writer.writeTextElement("firstName1", firstName1);
	writer.writeTextElement("lastName1", lastName1);
	writer.writeTextElement("title2", title2);
	writer.writeTextElement("firstName2", firstName2);
	writer.writeTextElement("lastName2", lastName2);
	writer.writeTextElement("startTimeID", QString::number(startTimeID));
	writer.writeTextElement("goalTimeID", QString::number(goalTimeID));
	writer.writeTextElement("dsq", QString(dsq?"true":"false"));
	writer.writeTextElement("errors", errors);
	//writer.writeTextElement("published", QString(published?"true":"false"));		// don't save the publish flag
	writer.writeTextElement("notes", notes);
	writer.writeTextElement("fireBrigade", fireBrigade);
	writer.writeTextElement("valuationClass", valuationClass);
	writer.writeTextElement("agePoints", agePoints);

	writer.writeEndElement();	// run
}


void RunData::changeHandler()
{
	if(editing)
	{
		// when in editing mode don't emit now but remember for later
		changed=true;
	}
	else
	{
		// emit now
		emit runChanged(this);
	}
}


void RunData::startEditing()
{
	editing=true;
}

void RunData::editingFinished()
{
	editing=false;
	if(changed)
	{
		changed=false;
		emit runChanged(this);
	}
}
