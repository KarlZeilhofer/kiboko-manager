#ifndef ASSIGNMENTMANAGER_H
#define ASSIGNMENTMANAGER_H

#include "dialogbindboatbox.h"

#include <QObject>

/*
	This Class is a static class, which only provides some functions to
		1) assign/unassign timestamps with a run
		2) bind/unbind a boatbox with a boat
	It handles all the communication with the user.
*/

class AssignmentManager : public QObject
{
    Q_OBJECT
public:
    explicit AssignmentManager(QObject *parent = 0);

	/*
		Here we distinguish some different cases:
		We always put the timestamp into the competition, if this isn't already done
		Always check first, if a run is finished (i.e. the run is marked as 'ready for publish'), if bound only temporarily for 'this run' then unbind this boatbox from this boat.

		1) If we only get a timestamp (TS) (without a run)
			Check, if it is a TS from a trigger-station.
				If so, then check, if the boat-box, the TS is coming from, is already bound to a boat-ID
					if so, then assign (###) it automatically to a run, if this is possible.
					if not, then ask the user to bind (###) this boat-box to a boat
						if this is done, then assign (###) it to a run, if this is possible
						if the user clicks later, we cannot assign the TS now.
				If not, we cannot assign the TS now.

		2) If we get a TS and a run-ID
			Check, if it is a TS from a trigger-station.
				If so, then check, if the boat-box, the TS is coming from, is already bound to a boat-ID
					if so, then check, if the bound boat-ID ist the same as the boat-ID of the run.
						if so, then do the assignment (###), if it is possible.
						if not, then ask the user, if he wants to replace the binding of the boatbox with the boat-id of the current run.
							if so, assign (###) the TS to the run, and update the binding (###) of the boatbox.
                                if the new boat-box is in a replacement-boat, ask the user, if he wants to use this binding only this round or for ever
                                    If only for this round, save a marker
                                    if for ever, nothing extra to do
							if not, only do the assignemnt (###) of the TS
					if the TS is not coming from an already bound boatbox, ask the user, if he wants to bind the boatbox
						if "Yes" bind (###) boatbox and boat 'permanently' and do the assignment (###) of the TS to the run.
						if "Yes, only for this run" (this option is only available, if the TS is from the TS-Start): set somewhere a marker for this run, bind (###) boatbox and boat and assign (###) the TS to the run.
						if "No": only do the assignment (###) of the TS.
				If the TS is not from a TS, then do the assignment (###) of the TS to the Run.

		returns true on success.
	*/
	bool tryToAssignTimeStampToRun(int timestampID, int runID=0);
	bool tryAutoAssignment(int timestampID);


	/*
	  Always, when an assignment (###) of a TS to a defined run should be done, check if the TS is from Triggerstation-Start or Triggerstation-Goal.
		  If TS is from start: check, if this run has a start-timestamp already assigned.
			  if so, ask the user, if he wants to replace the existing one
		  If TS is from goal:check, if this run has a goal-timestamp already assigned.
			  if so, ask the user, if he wants to replace the existing one
		  If TS is general (manual or edited), check for empty start and/or goal time-fields:
			  both are empty: assign to the start-time
			  only one is empty: assign to the empty one.
			  none is empty: aks the user, which time he wants to replace (Start/Goal/None).

	  returns true on success.
	*/
	bool assignToStartOrGoal(int timestampID, int runID);
    bool unassignTimeStamp(int timeStampID);

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
	bool tryToBindBoatboxWithBoat(int timestampID, int boatID=0);

	/*
		if the run has only one time-stamp, unassign this one.
		if the run has both time-stamps (start and goal) assigned, ask the user
		if the run has no time-stamp, return false;
	*/
	bool tryToUnassignStartGoalTimeStamp(int runID);
	bool unbindBoatbox(int boatboxID);

private:
	// these functions only do the raw assignment and binding.
	// already existing assignments/bindings will be overwritten.
	// return true on success. false will be returned only on invalid IDs.
	bool assignTimeStampToRunStart(int timestampID, int runID);
	bool assignTimeStampToRunGoal(int timestampID, int runID);
	bool bindBoatboxWithBoat(int timestampID, int boatID=0, QString boatName=QString());

signals:
	void unassignedTimeStamp(TimeStamp* ts);

public slots:
    void reset();

public:
	DialogBindBoatBox* dialogBindBoatBox;

};

#endif // ASSIGNMENTMANAGER_H
