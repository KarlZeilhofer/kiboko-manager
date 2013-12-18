#ifndef BOATBOX_H
#define BOATBOX_H

#include <QObject>

/*
  A Boatbox is placed in a boat. Therefore it has a boat-ID and of course itself has an ID.
  When a timestamp is received from the time-base, first only the boat-box-ID is known.
  Then there must be a connection to the corresponding boat-ID.
  All boatboxes are stored in the Competition::boatboxes.
*/

class Boatbox : public QObject
{
    Q_OBJECT
public:
	explicit Boatbox(QObject *parent, int ID, int boatID);

	void setBoatID(int boatID);
	int getBoatID();
	int getID();

signals:
    void boatboxBindingChanged(int boatboxID);

public slots:

private:
	int ID;
	int boatID;
};

#endif // BOATBOX_H
