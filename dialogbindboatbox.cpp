#include "dialogbindboatbox.h"
#include "ui_dialogbindboatbox.h"
#include "mainwindow.h"
#include "competition.h"
#include "timestamp.h"
// comboBox_boatName: 1,2,3,...,15,A,B,C,... (if no fixedBoatID)
// comboBox_boatName: 7,A,B,C,... (if e.g. fixedBoatID = 7)
// comboBox_boatID: 1,2,3,...,15

DialogBindBoatBox::DialogBindBoatBox(QWidget *parent, int numOfBoatsPerRound, int numOfReplacementBoats) :
	QDialog(parent), ui(new Ui::DialogBindBoatBox)
{
    ui->setupUi(this);
	fixedBoatID=0;
	setModal(true);

    timeStamp = 0;

	numBPR = numOfBoatsPerRound;
	numRB = numOfReplacementBoats;

	ui->comboBox_boatName->clear();
	ui->comboBox_boatID->clear();

	for(int i=0; i<numBPR; i++){
        ui->comboBox_boatName->insertItem(i, QString("%1").arg(i+1),
                                          QString("%1").arg(i+1)); // user data: "1","2","3",...,"A","B",...
        ui->comboBox_boatID->insertItem(i, QString("%1").arg(i+1),
                                        i+1); // user data: int 1,2,3,...,15
	}
	for(int i=0; i<numRB; i++){
        ui->comboBox_boatName->insertItem(i+numBPR, QString("%1").arg((char)('A'+i)),
                                          QString("%1").arg((char)('A'+i))); // user data: "1","2","3",...,"A","B",...
	}

    ui->comboBox_boatName->setCurrentIndex(0);
    ui->comboBox_boatID->setCurrentIndex(0);
}

DialogBindBoatBox::~DialogBindBoatBox()
{
    delete ui;
}

// ATENTION: do not set fixedBoatID directly, always use this setter!
void DialogBindBoatBox::setFixedBoatID(int boatID)
{
    fixedBoatID = boatID;
    refreshAllBindings();
}

void DialogBindBoatBox::setTimeStamp(TimeStamp* ts)
{
    timeStamp = ts;
    timer.stop();
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateDialogText()));
    timer.start(1000);
    updateDialogText();
}

void DialogBindBoatBox::setNumOfBoatsPerRound(int bpr)
{
    numBPR = bpr;
    if(fixedBoatID > bpr){
        fixedBoatID = 0; // reset fixed boat ID
    }
    refreshAllBindings();
}

void DialogBindBoatBox::setNumOfReplacementBoats(int rb)
{
    numRB = rb;
    refreshAllBindings();
}

// update the boat-id combobox
void DialogBindBoatBox::on_comboBox_boatName_currentIndexChanged(int currentIndex)
{
    if(fixedBoatID==0){ // do not change the boat-id combobox on fixedBoatID!
        ui->groupBox_replacementBoat->setEnabled(currentIndex >= numBPR); // enable the second drop box only for replacement boat-names

        if(currentIndex < numBPR){
            ui->comboBox_boatID->setCurrentIndex(currentIndex); //
        }else{
            ui->comboBox_boatID->setCurrentIndex(0); // select first entry as default index
        }
    }
}

void DialogBindBoatBox::on_pushButton_later_clicked()
{
	fixedBoatID=0;
	hide();
	setResult(QDialog::Rejected);
	emit rejected();
}

void DialogBindBoatBox::on_pushButton_OK_clicked()
{
	fixedBoatID=0;
	hide();
	setResult(QDialog::Accepted);
	emit accepted();
}

int DialogBindBoatBox::getSelectedBoatId()
{
    bool ok;
    int id;
    id = ui->comboBox_boatID->itemData(ui->comboBox_boatID->currentIndex()).toInt(&ok);
    if(ok)
        return id;
    else{
        qDebug() << "asked for invalid boat id (in " << __FILE__ << " line " << __LINE__;
        return 0;
    }
}

QString DialogBindBoatBox::getSelectedBoatName()
{
    return ui->comboBox_boatName->itemData(ui->comboBox_boatName->currentIndex()).toString();
}

// slot:
void DialogBindBoatBox::updateBinding(int boatBoxID)
{
	// TODO: do only the necessary updates.
	// for convinience all bindings are refreshed:
    boatBoxID++; // remove warning
	refreshAllBindings();
}

void DialogBindBoatBox::refreshAllBindings()
{
	Competition* c = MainWindow::competition();
	bool indexSetToFirstUnassignedName = false;
    bool indexSetToFirstUnassignedBoatID = false; // flag, if the index in the boatID-combobox has been set to the first unassigned boat.


	// check for assigned boatNames:
	ui->comboBox_boatName->clear();
	ui->comboBox_boatID->clear();


	// numerical "names" of the default boats
    // example: 1,2 and 4 are assigned
    // 1*  2*  3   4*  5   6   7   8   9   10   11   12    13   14   15
	for(int nameIndex=0; nameIndex<numBPR; nameIndex++){
        // add item only if:
        if(fixedBoatID==0 || // for all numbers, when no fixed boatID
                nameIndex+1==fixedBoatID) // or if we are at the fixed id
        {
            bool assigned = false; // flag, if the name is assigned to a boatID (many boat-names, some boatIDs)
            int boatID=nameIndex+1;

            if(c->getNameOfBoatID(boatID) == QString("%1").arg(nameIndex+1)){
                assigned = true;
            }

            if(assigned){
                boatID--; // invert boatID++ of the for-loop
                ui->comboBox_boatName->insertItem(nameIndex, QString("%1 (*)").arg(nameIndex+1),// just make a simple marker (*)
                                                  QString("%1").arg(nameIndex+1));  // user data: "1", "2", ..., "15"
            }else{
                ui->comboBox_boatName->insertItem(nameIndex, QString("%1").arg(nameIndex+1),
                                                  QString("%1").arg(nameIndex+1)); // user data: "1", "2", ..., "15"
                // select the first unassigned item:
                if(!indexSetToFirstUnassignedName){
                    indexSetToFirstUnassignedName = true;
                    indexSetToFirstUnassignedBoatID = true;
                    ui->comboBox_boatName->setCurrentIndex(nameIndex);
                    ui->comboBox_boatID->setCurrentIndex(nameIndex);
                }
            }
        }
	}

	// alphabetical names of the replacement boats
    // example: A and C are assigned to the boat-IDs 11 and 13
    // A (Zille 11)   B   C (Zille 13)   D   E
	for(int nameIndex=0; nameIndex<numRB; nameIndex++){
		bool assigned = false; // flag, if the name is assigned to a boatID (many boat-names, some boatIDs)
		int boatID=0;
		for(boatID=1; boatID<=numBPR && !assigned; boatID++){
			if(c->getNameOfBoatID(boatID) == QString("%1").arg((char)('A'+nameIndex))){
				assigned = true;
			}
		}

		if(assigned){ // if assigned, show the boatID (1...15)
            boatID--; // undo boatID++ of the for-loop
            ui->comboBox_boatName->insertItem(nameIndex+numBPR,
                                              QString("%1 (Zille %2)").arg((char)('A'+nameIndex)).arg(boatID),
                                              QString("%1").arg((char)('A'+nameIndex))); // userdata "A", "B", ...
		}else{
            ui->comboBox_boatName->insertItem(nameIndex+numBPR, QString("%1").arg((char)('A'+nameIndex)),
                                              QString("%1").arg((char)('A'+nameIndex))); // userdata "A", "B", ...
		}
	}


	// check for assigned boatIDs
    // eample:
    // "1 (BB 13)"   "2"   "3"   "4 (BB 1)"   "5"   "6"   "7"    "8"   "9"   "10"   "11"   "12"   "13"   "14"    "15"
	for(int boatID=1; boatID<=numBPR; boatID++){
        // add item only if:
        if(fixedBoatID==0 || // for all numbers, when no fixed boatID
                boatID==fixedBoatID) // or if we are at the fixed id
        {
            bool assigned = false; // flag, if a boatID is assigned to the boatboxID (many boatboxIDs some boatIDs)
            int boatBoxID=0;
            for(boatBoxID=1; boatBoxID<=(numBPR+numRB) && !assigned; boatBoxID++){
                if(c->getBoatIdOfBoatbox(boatBoxID) == boatID){
                    assigned = true;
                }
            }

            if(assigned){
                boatBoxID--; // undo boatBoxID++ of the for-loop
                ui->comboBox_boatID->insertItem(boatID-1, QString("%1 (BB %2)").arg(boatID).arg(boatBoxID),
                                                boatID); // user data: 1,2,3,4,...,15
            }else{
                ui->comboBox_boatID->insertItem(boatID-1, QString("%1").arg(boatID),
                                                 boatID); // user data: 1,2,3,4,...,15);

                // select the first unassigned item:
                if(!indexSetToFirstUnassignedBoatID){
                    indexSetToFirstUnassignedBoatID = true;
                    ui->comboBox_boatID->setCurrentIndex(boatID-1);
                }
            }
        }
	}

	// adopt the second combobox to the first one.
	if(ui->comboBox_boatName->currentIndex() < numBPR){
		ui->comboBox_boatID->setCurrentIndex(ui->comboBox_boatName->currentIndex());
	}
    // override it, when we have fixed boat-id:
    if(fixedBoatID){
        ui->comboBox_boatName->setCurrentIndex(0);
        ui->comboBox_boatID->setCurrentIndex(0);
    }
}

void DialogBindBoatBox::updateDialogText()
{
    if(timeStamp){
        int s = timeStamp->getTime().msecsTo(MainWindow::app()->getTimeBaseTime())/1000;

        if(timeStamp->getSourceName() == "TSS"){
            ui->label_infoText->setText(QString(tr("Unzugeordnete Boat-Box ist vor %1s \ndurch das Start-Tor gefahren!")).arg(s));
        }else if(timeStamp->getSourceName() == "TSGL"){
            ui->label_infoText->setText(QString(tr("Unzugeordnete Boat-Box ist vor %1s \ndurch das linke Ziel-Tor gefahren!")).arg(s));
        }else if(timeStamp->getSourceName() == "TSGR"){
            ui->label_infoText->setText(QString(tr("Unzugeordnete Boat-Box ist vor %1s \ndurch das rechte Ziel-Tor gefahren!")).arg(s));
        }
    }
}

// must be called from the competition or mainwindow, when timestamps get deleted!
void DialogBindBoatBox::resetTimeStampPointer()
{
    timeStamp = 0;
}
