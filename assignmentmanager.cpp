#include "assignmentmanager.h"
#include "mainwindow.h"
#include "competition.h"

#include <QMessageBox>
#include <QAbstractButton>
#include <QSettings>

#include "dialoggeneralsettings.h"

AssignmentManager::AssignmentManager(QObject *parent) :
    QObject(parent)
{
	dialogBindBoatBox=0;
}

bool AssignmentManager::tryToAssignTimeStampToRun(int timestampID, int runID)
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2, runID = %3)").arg(__func__).arg(timestampID).arg(runID).toAscii());
	int tsID = timestampID; // define an abbrevation
	Competition* c = MainWindow::competition();
	TimeStamp* ts = c->getTimeStamp(tsID);

	if(ts==0){
		return false;
	}


	if(tsID!=0 && runID==0){
		if(ts->getSourceName().startsWith("TS")){ // if TS comes from a trigger-station
			if(c->getBoatIdOfBoatbox(ts->getBoatboxID()) != 0){ // if boatbox is already bound to a boat
				return tryAutoAssignment(timestampID);
			}else{
				if(tryToBindBoatboxWithBoat(timestampID)){
					return tryAutoAssignment(timestampID);
				}else{
                    return false;
				}
			}
		}else{ // TS is not from a trigger-station
			return false; // we cannot assign now
		}
	}else if(tsID!=0 && runID!=0){
		if(ts->getSourceName().startsWith("TS")){ // if TS comes from a trigger-station
			if(c->getBoatIdOfBoatbox(ts->getBoatboxID()) != 0){ // if boatbox is already bound to a boat
				if(ts->getBoatboxID() == c->getBoatboxIdOfBoat(c->getRun(runID)->getBoatID())){ // if ts.bbID == run.bbID
					// then just do the assignment
					return assignToStartOrGoal(timestampID, runID);
				}else{
					//then ask the user, if he wants to replace the binding of the boatbox with the boat-id of the current run.
					QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, tr("Zuordnung ersetzen?"),
							  tr("Die Auslösung passt nicht zur Zillen-Nummer dieses Laufs. Soll die Boat-Box-Zuordnung neu gesetzt werden?"),
							  QMessageBox::Yes | QMessageBox::No, dynamic_cast<QWidget*>(parent()));
					box->exec();
					if(box->result() == QMessageBox::Yes){
                        tryToBindBoatboxWithBoat(timestampID, c->getRun(runID)->getBoatID()); // update the binding of the boatbox.
						return assignToStartOrGoal(timestampID, runID);
					}else if(box->result() == QMessageBox::No){
						return assignToStartOrGoal(timestampID, runID);
					}
				}
            }else{ // if boatbox isn't bound
				QMessageBox *box;

                box = new QMessageBox(QMessageBox::NoIcon, tr("Boat-Box der Zille zuordnen"),
                                      QString(tr("Soll die Boat-Box dieser Auslösung der Zille %1 permanent zugeordnet werden?").arg(c->getRun(runID)->getBoatID())),
                          QMessageBox::Yes | QMessageBox::No, dynamic_cast<QWidget*>(parent()));

                box->button(QMessageBox::No)->setText(tr("Nein"));
                box->button(QMessageBox::Yes)->setText(tr("Ja"));
                box->exec();

                int res = box->result();
                if(res == QMessageBox::Yes){
                    if(tryToBindBoatboxWithBoat(timestampID, c->getRun(runID)->getBoatID())){ // TODO: hier kommt nochmals der große dialog, ist aber überflüssig
                        return assignToStartOrGoal(timestampID, runID);
                    }else{
                        return false; // TODO: prüfen, ob dieser zweig jemals erfüllt ist bwz. notwengi ist.
                    }
				}else if(res == QMessageBox::No){
					return assignToStartOrGoal(timestampID, runID);
				}
            } // end else (not bound boatbox)
		}else{ // TS is not from a trigger-station
			// do the assignment:
            return assignToStartOrGoal(timestampID, runID);
		}
	}
    return false; // should never be reached.
}


// try the automatic assignment.
bool AssignmentManager::tryAutoAssignment(int timestampID)
{
    QSettings settings;
    settings.beginGroup(SS_GS_PREFIX);

	//qDebug(QString("AssignmentManager::%1(timeStampID = %2)").arg(__func__).arg(timestampID).toAscii());
    Competition* c = MainWindow::competition();
	TimeStamp* ts = c->getTimeStamp(timestampID);
	if(ts==0){
		return false;
	}

    // if it is a timestamp from start, search for a corresponding run in the mainTable from the recent started run on.
    if(ts->getSource() == TimeStamp::TSS){
        int bpr = c->getBoatsPerRound();
        int start;
        int end;

        if(settings.value(SS_GS_ASSIGN_TO_CURRENT_ROUND, false).toBool()){// if upper austria mode
            start = c->getHighestStartedRunID()-(bpr-1);
            end = c->getHighestStartedRunID()+(bpr-1);
        }else{// if lower austria mode
            start = c->getHighestStartedRunID()+1;
            end = c->getHighestStartedRunID()+bpr;
        }

        if(start<=0){ // catch the case of no run has started
            start=1;
        }

        for(int r=start; r<=end; r++){
            if(r==0){ // catch the case of no run has started
                r++;
            }
            if(c->getRun(r)->getBoatID() == c->getBoatIdOfBoatbox(ts->getBoatboxID()) && // if run matches the boatbox
                    c->getRun(r)->getStartTimeID()==0){ // and run not started
                assignTimeStampToRunStart(timestampID, r);
                return true;
            }
        }
    }

    // if it is from the goal, search for a matching run in the current round and in the recent round.
    if(ts->getSource() == TimeStamp::TSGL || ts->getSource() == TimeStamp::TSGR){
        int bpr = c->getBoatsPerRound();
        int start;
        int end;

        if(settings.value(SS_GS_ASSIGN_TO_CURRENT_ROUND, false).toBool()){ // if upper austria mode
            start = c->getHighestStartedRunID();
            int currentRound = (start-1)/bpr + 1; // starting with 1
            end = (currentRound-1-1)*bpr+1; // lower end of search. (is the first boat in the recent round)
        }else{ // if lower austria mode
            start = c->getHighestStartedRunID();
            end = c->getHighestStartedRunID()-bpr;
        }

        if(start<=0){ // catch the case of no run has started
            start++;
        }

        for(int r=start; r>=1 && r>=end; r--){
            if(c->getRun(r)->getBoatID() == c->getBoatIdOfBoatbox(ts->getBoatboxID()) && // if run matches the boatbox
                    c->getRun(r)->getStartTimeID()!=0 && c->getRun(r)->getGoalTimeID()==0){ // and run is started && not finished
                assignTimeStampToRunGoal(timestampID, r);
                return true;
            }
        }
    }
	return false;
}


bool AssignmentManager::assignToStartOrGoal(int timestampID, int runID)
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2, runID = %3)").arg(__func__).arg(timestampID).arg(runID).toAscii());
    Competition* c = MainWindow::competition();
	TimeStamp* ts = c->getTimeStamp(timestampID);

	TimeStamp::Source s = ts->getSource();
	if(s==TimeStamp::TSS || s==TimeStamp::LS){ // if ts is from start
		if(c->getRun(runID)->getStartTimeID() != 0){
			QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, tr("Start-Zeit ersetzen?"),
                      tr("Soll die Startzeit ersetzt werden?"),
					  QMessageBox::Yes | QMessageBox::No, dynamic_cast<QWidget*>(parent()));
			box->button(QMessageBox::Yes)->setText(tr("Ja"));
			box->button(QMessageBox::No)->setText(tr("Nein"));
			box->exec();

			if(box->result() == QMessageBox::Yes){ // ersetzzen
                // unassign stimestamp, that will be replaced.
                c->unassignTimeStamp(c->getRun(runID)->getStartTimeID());
				return assignTimeStampToRunStart(timestampID, runID);
			}else if(box->result() == QMessageBox::No){ // nicht ersetzen
				return false;
			}
		}else{
			return assignTimeStampToRunStart(timestampID, runID);
		}
	}else if(s==TimeStamp::TSGL || s==TimeStamp::TSGR || s==TimeStamp::LGL || s==TimeStamp::LGR){ // if ts is from goal
		if(c->getRun(runID)->getGoalTimeID() != 0){
			QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, tr("Ziel-Zeit ersetzen?"),
					  tr("Soll die Zielzeit ersetzt wereden?"),
					  QMessageBox::Yes | QMessageBox::No, dynamic_cast<QWidget*>(parent()));
			box->button(QMessageBox::Yes)->setText(tr("Ja"));
			box->button(QMessageBox::No)->setText(tr("Nein"));
			box->exec();

			if(box->result() == QMessageBox::Yes){ // ersetzzen
                // unassign stimestamp, that will be replaced.
                c->unassignTimeStamp(c->getRun(runID)->getGoalTimeID());
				return assignTimeStampToRunGoal(timestampID, runID);
			}else if(box->result() == QMessageBox::No){ // nicht ersetzen
				return false;
			}
		}else{
			return assignTimeStampToRunGoal(timestampID, runID);
		}
	}else if(s==TimeStamp::M || s==TimeStamp::E){ // if ts is from start
		if(c->getRun(runID)->getStartTimeID() == 0 && c->getRun(runID)->getGoalTimeID() == 0){ // both empty
			// assign to start
			assignTimeStampToRunStart(timestampID, runID);
		}else if(c->getRun(runID)->getStartTimeID() == 0 && c->getRun(runID)->getGoalTimeID() != 0){ // start empty
			// assign to start
			assignTimeStampToRunStart(timestampID, runID);
		}else if(c->getRun(runID)->getStartTimeID() != 0 && c->getRun(runID)->getGoalTimeID() == 0){ // goal empty
			// assign to goal
			assignTimeStampToRunGoal(timestampID, runID);
		}else if(c->getRun(runID)->getStartTimeID() != 0 && c->getRun(runID)->getGoalTimeID() != 0){ // none empty
			// ask user, which time he wants to replace
			QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, tr("Welche Zeit ersetzen?"),
					  tr("Welche Zeit soll ersetzt werden?"),
					  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, dynamic_cast<QWidget*>(parent()));
			box->button(QMessageBox::Yes)->setText(tr("Startzeit"));
			box->button(QMessageBox::No)->setText(tr("Zielzeit"));
			box->button(QMessageBox::Cancel)->setText(tr("Abbrechen"));
			box->exec();

			if(box->result() == QMessageBox::Yes){ // startzeit
				unassignTimeStamp(c->getRun(runID)->getStartTimeID());
				return assignTimeStampToRunStart(timestampID, runID);
			}else if(box->result() == QMessageBox::No){ // zielzeit
				unassignTimeStamp(c->getRun(runID)->getGoalTimeID());
				return assignTimeStampToRunGoal(timestampID, runID);
			}else if(box->result() == QMessageBox::No){ // zielzeit
				return false;
			}
		}
	}
    return false;
}

/*
	if the run has only one time-stamp, unassign this one.
	if the run has both time-stamps (start and goal) assigned, ask the user
	if the run has no time-stamp, return false;
*/
bool AssignmentManager::tryToUnassignStartGoalTimeStamp(int runID)
{
	//qDebug(QString("AssignmentManager::%1(runID = %2)").arg(__func__).arg(runID).toAscii());
    Competition* c = MainWindow::competition();
	RunData* run = c->getRun(runID);

	if(run->getStartTimeID()!=0 && run->getGoalTimeID()!=0){
		QMessageBox* box = new QMessageBox(QMessageBox::NoIcon, tr("Welche Zeit entfernen?"),
				  tr("Welche Zeit soll entfernt werden?"),
				  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, dynamic_cast<QWidget*>(parent()));
		box->button(QMessageBox::Yes)->setText(tr("Startzeit"));
		box->button(QMessageBox::No)->setText(tr("Zielzeit"));
		box->button(QMessageBox::Cancel)->setText(tr("Abbrechen"));
		box->exec();

		if(box->result() == QMessageBox::Yes){ // starttime
			return unassignTimeStamp(c->getRun(runID)->getStartTimeID());
		}else if(box->result() == QMessageBox::No){ // goaltime
			return unassignTimeStamp(c->getRun(runID)->getGoalTimeID());
		}else if(box->result() == QMessageBox::No){ // cancel
			return false;
		}
	}else if(run->getStartTimeID()!=0 && run->getGoalTimeID()==0){
		return unassignTimeStamp(run->getStartTimeID());
	}else if(run->getStartTimeID()==0 && run->getGoalTimeID()!=0){
		return unassignTimeStamp(run->getGoalTimeID());
	}else if(run->getStartTimeID()==0 && run->getGoalTimeID()==0){
		return false;
	}
    return false;
}

bool AssignmentManager::unassignTimeStamp(int timestampID) // from run
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2)").arg(__func__).arg(timestampID).toAscii());
    Competition* c = MainWindow::competition();
    TimeStamp* ts = c->getTimeStamp(timestampID);
	if(ts==0){
		return false;
	}
	int runID = ts->getRunID();
	RunData* run = c->getRun(runID);

	if(run==0){
		return false;
	}

	// delete run from TS:
	ts->setRunID(0);
	// ts->setBoatboxID(0); // !!! DON'T unassign the boatboxID !!!
	ts->setBoatID(0);
	ts->setBoatName("");

	// delete ts from run
    if(run->getStartTimeID() == timestampID){
		run->setStartTimeID(0);
	}
    if(run->getGoalTimeID() == timestampID){
		run->setGoalTimeID(0);
	}
	
	emit unassignedTimeStamp(ts);

	return true;
}

/*
    Notes to: tryToBindBoatboxWithBoat()

    Only with a timestampID (boatID=0)
        show the dialog, where the user can enter the boat-name (1,2,3,...,15,A,B,...) and,
        if it is a replacement boat (A,B,...), then ask also for the boatID
        if the selected boatID has already a bound boatbox, ask the user, if he wants to replace the binding.
            if so and if the selected boat-name is a replacement boat, ask the user, if he wants to bind only for one round or for ever


    With a given boatboxID and a boatID
        show the dialog, with a fixed boatID.
        the selectable boatnames are only the numerical name "boatID" and the replacement boat names
        if the selected boat-name is a replacement boat, ask the user, if he wants to bind only for one round or for ever

    If the timestamp belonging to this binding is max. 15 minutes old, the time is shown in the dialog.

    returns true on success.
*/
bool AssignmentManager::tryToBindBoatboxWithBoat(int timestampID, int boatID)
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2, boatID = %3)").arg(__func__).arg(timestampID).arg(boatID).toAscii());
    Competition* c = MainWindow::competition();
	TimeStamp* ts = c->getTimeStamp(timestampID);

	if(ts==0){
		return false;
	}


	if(timestampID!=0 && boatID==0){
		if(dialogBindBoatBox == 0){
			dialogBindBoatBox = new DialogBindBoatBox(MainWindow::app(), c->getBoatsPerRound(), 5); // TODO: replace the constant 5
			// connect(c, SIGNAL(boatboxBindingChanged(int)), dialogBindBoatBox, SLOT(updateBinding(int))); // TODO: do the connect in mainWindow
		}

		dialogBindBoatBox->refreshAllBindings();
        dialogBindBoatBox->setTimeStamp(ts);
		dialogBindBoatBox->exec();

		if(dialogBindBoatBox->result() == QDialog::Accepted){
			int tempBoatID = dialogBindBoatBox->getSelectedBoatId();
			QString boatName = dialogBindBoatBox->getSelectedBoatName();
			// check for overriding an assignment
			bool doBind = true;
			if(c->getBoatboxIdOfBoat(tempBoatID) != 0){
				QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Überschreiben?"));
                msgBox.setText(QString(tr("Der Zille %1 wurde bereits eine Boat-Box zugeordnet")).arg(tempBoatID));
				msgBox.setInformativeText(tr("Möchtest du diese Zuordnung überschreiben?"));
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setDefaultButton(QMessageBox::No);
				int ret = msgBox.exec();
                if(ret == QMessageBox::No){
                    doBind = false;
                }
			}
			if(doBind){
				return bindBoatboxWithBoat(timestampID, tempBoatID, boatName);
			}
		}else{
			return false; // user pressed cancel
		}
	}else if(timestampID!=0 && boatID!=0){
		if(dialogBindBoatBox == 0){
			dialogBindBoatBox = new DialogBindBoatBox(MainWindow::app(), c->getBoatsPerRound(), 5); // TODO: replace the constant 5
			// connect(c, SIGNAL(boatboxBindingChanged(int)), dialogBindBoatBox, SLOT(updateBinding(int))); // TODO: do the connect in mainWindow
		}
        dialogBindBoatBox->setFixedBoatID(boatID); // <<== HERE IS THE DIFFERENCE
        dialogBindBoatBox->refreshAllBindings();
        dialogBindBoatBox->setTimeStamp(ts);
		dialogBindBoatBox->exec();

		if(dialogBindBoatBox->result() == QDialog::Accepted){
			int tempBoatID = dialogBindBoatBox->getSelectedBoatId();
			QString boatName = dialogBindBoatBox->getSelectedBoatName();
			// check for overriding an assignment
			bool doBind = true;
			if(c->getBoatboxIdOfBoat(tempBoatID) != 0){
				QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Überschreiben?"));
                msgBox.setText(QString(tr("Der Zille %1 wurde bereits eine Boat-Box zugeordnet")).arg(tempBoatID));
                msgBox.setInformativeText(tr("Möchtest du diese Zuordnung überschreiben?"));
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setDefaultButton(QMessageBox::No);
				int ret = msgBox.exec();
                if(ret == QMessageBox::No){
                    doBind = false;
                }
			}
			if(doBind){
				return bindBoatboxWithBoat(timestampID, tempBoatID, boatName);
			}
		}else{
			return false; // user pressed cancel
		}
	}
    return false;
}

void AssignmentManager::reset()
{
    if(dialogBindBoatBox){
        dialogBindBoatBox->resetTimeStampPointer();
    }
}

bool AssignmentManager::unbindBoatbox(int boatboxID)
{
	//qDebug(QString("AssignmentManager::%1(boatboxID = %2)").arg(__func__).arg(boatboxID).toAscii());
    Competition* c = MainWindow::competition();

    c->bindBox2BoatID(boatboxID, 0);
	return true;
}

// also adds the info-text, about a replacement-boat, if applicable
bool AssignmentManager::assignTimeStampToRunStart(int timestampID, int runID)
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2, runID = %3)").arg(__func__).arg(timestampID).arg(runID).toAscii());
    Competition* c = MainWindow::competition();

	RunData* run = c->getRun(runID);
	run->setStartTimeID(timestampID);

    // if replacement boat, add a note
    if(c->getBoatName(run->getBoatID()).at(0).isLetter()){
        QString notes = run->getNotes();
        run->setNotes(notes.append(tr("Ersatzzille ").append(c->getBoatName(run->getBoatID()))));
    }

	TimeStamp* ts = c->getTimeStamp(timestampID);
	ts->setRunID(runID);
	ts->setBoatID(run->getBoatID());
	ts->setBoatName(c->getBoatName(run->getBoatID()));

	return true;
}

bool AssignmentManager::assignTimeStampToRunGoal(int timestampID, int runID)
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2, runID = %3)").arg(__func__).arg(timestampID).arg(runID).toAscii());
    Competition* c = MainWindow::competition();
	RunData* run = c->getRun(runID);
	run->setGoalTimeID(timestampID);

	TimeStamp* ts = c->getTimeStamp(timestampID);
	ts->setRunID(runID);
	ts->setBoatID(run->getBoatID());
	ts->setBoatName(c->getBoatName(run->getBoatID()));

	return true;
}

bool AssignmentManager::bindBoatboxWithBoat(int timestampID, int boatID, QString boatName)
{
	//qDebug(QString("AssignmentManager::%1(timeStampID = %2, boatID = %3, boatName = \"%4\")").arg(__func__).arg(timestampID).arg(boatID).arg(boatName).toAscii());
    Competition* c = MainWindow::competition();
	TimeStamp* ts = c->getTimeStamp(timestampID);

	int bbID = ts->getBoatboxID();
    c->setBoatName(boatID, boatName); // do the name assignment first, to give the GUI the possibility to display new boat-name of a changed boat-box!
    c->bindBox2BoatID(bbID, boatID);

    return true;

    // manage, that the TS is shown in the main-table (tryAutoAssignment(timestampID);):
    // --> this is done in tryToAssignTimeStampToRun(ts->getID());
}
