#include "boatbox.h"

Boatbox::Boatbox(QObject *parent, int ID, int boatID) :
    QObject(parent)
{
	this->ID=ID;
	this->boatID=boatID;
}


void Boatbox::setBoatID(int boatID)
{
    if(boatID != this->boatID){
        this->boatID=boatID;
        emit boatboxBindingChanged(ID);
    }
}

int Boatbox::getBoatID()
{
	return boatID;
}

int Boatbox::getID()
{
	return ID;
}
