#ifndef BOAT_H
#define BOAT_H

#include <QObject>

/*
  A Boat has an ID. This is typically in the range from 1 to 15.
  The upper limit of the ID is the number of boats per round.
  In the main table is the boat-number which is this ID.
  The name-member is the "name" wich is written on the boat. It is
  typically {"1", "2", "3", ..., "15", "A", "B", ...}
  If the boat-name doesn't match the boat-ID, then automatically an
  entry in the main-table is generated in the notes-column.
*/

class Boat : public QObject
{
    Q_OBJECT
public:
	explicit Boat(QObject *parent, int ID, QString name);

	void setName(QString name);
	QString getName();
	int getID();

signals:
    void boatNameChanged(int boatID);

public slots:

private:
	int ID;
	QString name;
};

#endif // BOAT_H
