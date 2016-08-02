#ifndef BOXSTATES_H
#define BOXSTATES_H

#include <QObject>
#include <QTime>
#include <QLabel>

#include "defs.h"
#include "timestamp.h"
//#include "../../eclipse/rfsw_0.1/packet.h"

/*
 * This class stores the current states of all Boxes in the system
 * It generates the TimeStamps
 */


class BoxStates : public QObject
{
	Q_OBJECT
public:
	enum Station{START=0, GOAL=1};
	
	explicit BoxStates(QObject *parent = 0);
	
	// setters
    void setTimeBaseTime(QTime baseTime);
	
	// battery voltages
	void setTimeBaseVoltage(double voltage);
	void setTriggerStationVoltage(Station s, double voltage);
	void setMatrixDisplayVoltage(int display, double voltage);
    void setBoatBoxVoltage(int ID, BoxStates::Station s, double voltage);
	
	// rssi values
	void setTriggerStationRssi(Station s, double rssi);
	void setMatrixDisplayRssi(int display, double rssi);
	void setBoatBoxRssi(int ID, Station s, double rssi);

	// trigger times
	void setTsTriggerTimeL(Station s, QTime time);
	void setTsTriggerTimeR(Station s, QTime time);
    void setBoatBoxTriggerTime(int ID, Station s, QTime time, TimeStamp::Source source);
	
	// lastAliveTime is set automatically
	
	
	// getters
	
	// last alive times
	QTime timeBaseLastAliveTime() {return timeBaseState.lastAliveTime;}
	QTime triggerStationLastAliveTime(Station s) {return triggerStationState[s].lastAliveTime;}
	QTime matrixDisplayLastAliveTime(int d) {return triggerStationState[d].lastAliveTime;}
    QTime boatBoxLastAliveTime(int ID, BoxStates::Station s) {return boatBoxState[ID].lastAliveTime[s];}
	
	// battery voltages
	double timeBaseVoltage() {return timeBaseState.voltage;}
	double triggerStationVoltage(Station s) {return triggerStationState[s].voltage;}
	double matrixDisplayVoltage(int d) {return triggerStationState[d].voltage;}
	double boatBoxVoltage(int ID) {return boatBoxState[ID].voltage;}
	
	// rssi values
	double triggerStationRssi(Station s) {return triggerStationState[s].rssi;}
	double matrixDisplayRssi(int d) {return triggerStationState[d].rssi;}
	double boatBoxRssi(int ID, Station s) {return boatBoxState[ID].rssi[s];}

private:
    double voltageToBatteryState(double voltage);
	double voltageToBatteryStatePb(double voltage);
    double rssiToSignalStrength(double rssi);
    QString getAbsentText(QTime time);
	
	void checkAbsentTimes();
    void emitNewTimeStamp(QTime time, TimeStamp::Source source, int boatBoxID=0);

private slots:
    void refreshAbsentTimes();
	
signals:
	void receivedNewTimeStamp(TimeStamp* ts);
	
public slots:
    void updateBoatNames();

private:
    QTime firstTimeBaseTimeReceived; // TODO
	
public:
	
	struct
	{
        QTime baseTime;         // current system time of Time-Base
		QTime lastAliveTime;	// time of the last packet received from this box
		double voltage;			// in V, battery voltage
		bool lowVoltage;
		QTime lowVoltageSince;
		bool absent;

        QLabel txtBaseTime;
        QLabel txtVoltage;
        QLabel txtAbsentSince;
	} timeBaseState;
	
	struct
	{
		QTime lastAliveTime;	// time of the last packet received from this box
		double rssi;			// in dBm, RSSI value of 433MHz connection to timebase 
		double voltage;			// in V, battery voltage
		QTime triggerTimeL;		// time of last trigger on the left side
		QTime triggerTimeR;		// time of last trigger on the right side
		bool lowVoltage;
		QTime lowVoltageSince;
		bool absent;

        QLabel txtVoltage;
        QLabel txtSignalStrength;
        QLabel txtAbsentSince;
	} triggerStationState[2];	// use enum Station as array index
	
	struct
	{
		QTime lastAliveTime;	// time of the last packet received from this display
		double rssi;			// in dBm, RSSI value of 433MHz connection to timebase 
		double voltage;			// in V, battery voltage (this is a 12V AGM Pb battery)
		bool lowVoltage;
		QTime lowVoltageSince;
		bool absent;

        QLabel txtVoltage;
        QLabel txtSignalStrength;
        QLabel txtAbsentSince;
	} matrixDisplayState[3];
	
	struct
	{
        // use enum Station as array index:
        QTime lastAliveTime[2];	// time of the last packet received from this box
		double rssi[2];			// in dBm, RSSI value of 2.4GHz connection to TS

		double voltage;			// in V, battery voltage
		bool lowVoltage;
		QTime lowVoltageSince;
		
		QList<QTime> recentTriggerTimes;					// last time the boatbox detected a trigger signal
		TimeStamp::Source triggerSource;	// source of last trigger signal 

        QLabel txtBoatName;
        QLabel txtVoltage;
        QLabel txtAbsentSince[2]; // use enum Station as array index
        QLabel txtSignalStrength[2]; // use enum Station as array index
    } boatBoxState[N_BOATBOXES+1]; // use boatboxID for index (dummy element at index=0)
};

#endif // BOXSTATES_H
