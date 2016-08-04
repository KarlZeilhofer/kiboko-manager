#include "boat.h"

Boat::Boat(QObject *parent, int ID, QString name) :
	QObject(parent)
{
	this->ID=ID;
    setName(name);
}


void Boat::setName(QString name)
{
    if(name != this->name){
        this->name=name;
        emit boatNameChanged(ID);
    }
}

QString Boat::getName()
{
	return name;
}

int Boat::getID()
{
	return ID;
}
