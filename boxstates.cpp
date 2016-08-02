#include "boxstates.h"
#include <QApplication>
#include <cmath>
#include "competition.h"
#include "mainwindow.h"
#include "infoscreen.h"
#include <QTimer>

#define VOLTAGE_WARNING_LEVEL	20		// %
#define VOLTAGE_WARNING_HYST	1		// %
#define ABSENT_WARNING_LEVEL	20000	// ms
#define LOW_VOLTAGE_TIMEOUT		10		// s


BoxStates::BoxStates(QObject *parent) :
	QObject(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(refreshAbsentTimes()));
    timer->start(111); // run update-timer
	
	timeBaseState.absent=true;
	triggerStationState[START].absent=true;
	triggerStationState[GOAL].absent=true;
	matrixDisplayState[1].absent=true;
	matrixDisplayState[2].absent=true;
	
	timeBaseState.lowVoltageSince=QTime();
	timeBaseState.lowVoltage=false;
	triggerStationState[START].lowVoltageSince=QTime();
	triggerStationState[START].lowVoltage=false;
	triggerStationState[GOAL].lowVoltageSince=QTime();
	triggerStationState[GOAL].lowVoltage=false;
	matrixDisplayState[1].lowVoltageSince=QTime();
	matrixDisplayState[1].lowVoltage=false;
	matrixDisplayState[2].lowVoltageSince=QTime();
	matrixDisplayState[2].lowVoltage=false;
	
	for(int i=1; i<=N_BOATBOXES; i++)
	{
		boatBoxState[i].lowVoltageSince=QTime();
		boatBoxState[i].lowVoltage=false;
	}
}


void BoxStates::setTimeBaseTime(QTime baseTime)
{
    // this implements the feature, that old time-stamps to not appear in a new competition
    // see also emitNewTimeStamp()
    if(firstTimeBaseTimeReceived.isValid() == false){
        firstTimeBaseTimeReceived = baseTime;
    }
    timeBaseState.baseTime = baseTime;
    timeBaseState.txtBaseTime.setText(MainWindow::convertTimeToString(baseTime) + QString(" (%1ms)").arg(MainWindow::app()->localTimeOffset_ms));
	if(timeBaseState.baseTime.isValid() && timeBaseState.absent)
	{
		timeBaseState.absent=false;
		MainWindow::app()->infoscreen()->appendInfo(tr("Verbindung zur Zeitbasis hergestellt"));
	}
}


// battery voltages
void BoxStates::setTimeBaseVoltage(double voltage)
{
    timeBaseState.lastAliveTime=MainWindow::app()->getTimeBaseTime();
	
	if(!timeBaseState.lowVoltageSince.isValid())
	{
		timeBaseState.lowVoltageSince=MainWindow::app()->getTimeBaseTime();
	}
	
	if(voltage!=timeBaseState.voltage)
	{
		timeBaseState.voltage=voltage;
        timeBaseState.txtVoltage.setText(QString("%1%").arg(QString::number(100*voltageToBatteryState(voltage), 'f', 0)));
		
		// check voltage
		if(100*voltageToBatteryState(voltage)<VOLTAGE_WARNING_LEVEL-VOLTAGE_WARNING_HYST)
		{
			// voltage too low
			if(timeBaseState.lowVoltageSince.secsTo(MainWindow::app()->getTimeBaseTime()) > LOW_VOLTAGE_TIMEOUT && !timeBaseState.lowVoltage)
			{
				timeBaseState.lowVoltage=true;
				MainWindow::app()->infoscreen()->appendWarning(tr("Akkustand der Zeitbasis kritisch"));
			}
		}
		if(100*voltageToBatteryState(voltage)>VOLTAGE_WARNING_LEVEL+VOLTAGE_WARNING_HYST)
		{
			// voltage OK
			timeBaseState.lowVoltage=false;
			timeBaseState.lowVoltageSince=MainWindow::app()->getTimeBaseTime();
		}
	}
}

void BoxStates::setTriggerStationVoltage(BoxStates::Station s, double voltage)
{
    triggerStationState[s].lastAliveTime=MainWindow::app()->getTimeBaseTime();
	
	if(!triggerStationState[s].lowVoltageSince.isValid())
	{
		triggerStationState[s].lowVoltageSince=MainWindow::app()->getTimeBaseTime();
	}
	
	if(voltage!=triggerStationState[s].voltage)
	{
		triggerStationState[s].voltage=voltage;
        triggerStationState[s].txtVoltage.setText(QString("%1%").arg(QString::number(100*voltageToBatteryState(voltage), 'f', 0)));
		
		// check voltage
		if(100*voltageToBatteryState(voltage)<VOLTAGE_WARNING_LEVEL-VOLTAGE_WARNING_HYST)
		{
			// voltage too low
			if(triggerStationState[s].lowVoltageSince.secsTo(MainWindow::app()->getTimeBaseTime()) > LOW_VOLTAGE_TIMEOUT && !triggerStationState[s].lowVoltage)
			{
				triggerStationState[s].lowVoltage=true;
				if(s==START)
				{
					MainWindow::app()->infoscreen()->appendWarning(tr("Akkustand der Startbox kritisch"));
				}
				else
				{
					MainWindow::app()->infoscreen()->appendWarning(tr("Akkustand der Zielbox kritisch"));
				}
			}
		}
		if(100*voltageToBatteryState(voltage)>VOLTAGE_WARNING_LEVEL+VOLTAGE_WARNING_HYST)
		{
			// voltage OK
			triggerStationState[s].lowVoltage=false;
			triggerStationState[s].lowVoltageSince=MainWindow::app()->getTimeBaseTime();
		}
	}
}


void BoxStates::setMatrixDisplayVoltage(int display, double voltage)
{
    matrixDisplayState[display].lastAliveTime=MainWindow::app()->getTimeBaseTime();
	
	if(!matrixDisplayState[display].lowVoltageSince.isValid())
	{
		matrixDisplayState[display].lowVoltageSince=MainWindow::app()->getTimeBaseTime();
	}
	
	if(voltage!=matrixDisplayState[display].voltage)
	{
		matrixDisplayState[display].voltage=voltage;
        matrixDisplayState[display].txtVoltage.setText(QString("%1%").arg(QString::number(100*voltageToBatteryStatePb(voltage), 'f', 0)));
		
		//qDebug() << "Matrix Display Voltage: " << voltage;
		
		// check voltage
		if(100*voltageToBatteryStatePb(voltage)<VOLTAGE_WARNING_LEVEL-VOLTAGE_WARNING_HYST)
		{
			// voltage too low
			if(matrixDisplayState[display].lowVoltageSince.secsTo(MainWindow::app()->getTimeBaseTime()) > LOW_VOLTAGE_TIMEOUT && !matrixDisplayState[display].lowVoltage)
			{
				matrixDisplayState[display].lowVoltage=true;
				MainWindow::app()->infoscreen()->appendWarning(tr("Akkustand der Kiboko Matrix %1 kritisch").arg(display));
			}
		}
		if(100*voltageToBatteryStatePb(voltage)>VOLTAGE_WARNING_LEVEL+VOLTAGE_WARNING_HYST)
		{
			// voltage OK
			matrixDisplayState[display].lowVoltage=false;
			matrixDisplayState[display].lowVoltageSince=MainWindow::app()->getTimeBaseTime();
		}
	}
}

void BoxStates::setBoatBoxVoltage(int ID, BoxStates::Station s, double voltage)
{
    boatBoxState[ID].lastAliveTime[s]=MainWindow::app()->getTimeBaseTime();
	
	if(!boatBoxState[ID].lowVoltageSince.isValid())
	{
		boatBoxState[ID].lowVoltageSince=MainWindow::app()->getTimeBaseTime();
	}
	
    if(voltage!=boatBoxState[ID].voltage)
	{
        boatBoxState[ID].voltage=voltage;
        boatBoxState[ID].txtVoltage.setText(QString("%1%").arg(QString::number(100*voltageToBatteryState(voltage), 'f', 0)));
		
		// check voltage
		if(100*voltageToBatteryState(voltage)<VOLTAGE_WARNING_LEVEL-VOLTAGE_WARNING_HYST)
		{
			// voltage too low
			if(boatBoxState[ID].lowVoltageSince.secsTo(MainWindow::app()->getTimeBaseTime()) > LOW_VOLTAGE_TIMEOUT && !boatBoxState[ID].lowVoltage)
			{
				boatBoxState[ID].lowVoltage=true;
				MainWindow::app()->infoscreen()->appendWarning(tr("Akkustand von BoatBox %1 kritisch").arg(ID));
			}
		}
		if(100*voltageToBatteryState(voltage)>VOLTAGE_WARNING_LEVEL+VOLTAGE_WARNING_HYST)
		{
			// voltage OK
			boatBoxState[ID].lowVoltage=false;
			boatBoxState[ID].lowVoltageSince=MainWindow::app()->getTimeBaseTime();
		}
	}
}


// rssi values
void BoxStates::setTriggerStationRssi(BoxStates::Station s, double rssi)
{
    triggerStationState[s].lastAliveTime=MainWindow::app()->getTimeBaseTime();
	if(rssi!=triggerStationState[s].rssi)
	{
		triggerStationState[s].rssi=rssi;
        triggerStationState[s].txtSignalStrength.setText(QString("%1%").arg(QString::number(100*rssiToSignalStrength(rssi), 'f', 0)));
	}
}

void BoxStates::setMatrixDisplayRssi(int display, double rssi)
{
    matrixDisplayState[display].lastAliveTime=MainWindow::app()->getTimeBaseTime();
	if(rssi!=matrixDisplayState[display].rssi)
	{
		matrixDisplayState[display].rssi=rssi;
        matrixDisplayState[display].txtSignalStrength.setText(QString("%1%").arg(QString::number(100*rssiToSignalStrength(rssi), 'f', 0)));
	}
}

void BoxStates::setBoatBoxRssi(int ID, BoxStates::Station s, double rssi)
{
    boatBoxState[ID].lastAliveTime[s]=MainWindow::app()->getTimeBaseTime();
	
	
    if(rssi!=boatBoxState[ID].rssi[s])
	{
        boatBoxState[ID].rssi[s]=rssi;
        boatBoxState[ID].txtSignalStrength[s].setText(QString("%1%").arg(QString::number(100*rssiToSignalStrength(rssi), 'f', 0)));
    }
}


// trigger times
void BoxStates::setTsTriggerTimeL(BoxStates::Station s, QTime time)
{
    triggerStationState[s].lastAliveTime=MainWindow::app()->getTimeBaseTime();
	
	if(time > MainWindow::app()->getTimeBaseTime().addSecs(10))
	{
		qDebug() << "received timestamp from future!";
		return;
	}
	
    if(time!=triggerStationState[s].triggerTimeL) // block repeated time stamps received from the boatbox
	{
		triggerStationState[s].triggerTimeL=time;
		
		// generate new timestamp
		if(s==BoxStates::START)
		{
            emitNewTimeStamp(time, TimeStamp::LS);
		}
		else if(s==BoxStates::GOAL)
		{
            emitNewTimeStamp(time, TimeStamp::LGL);
		}
	}
}

void BoxStates::setTsTriggerTimeR(Station s, QTime time)
{
    triggerStationState[s].lastAliveTime=MainWindow::app()->getTimeBaseTime();
	
	if(time > MainWindow::app()->getTimeBaseTime().addSecs(10))
	{
		qDebug() << "received timestamp from future!";
		return;
	}
	
	if(time!=triggerStationState[s].triggerTimeR)
	{
		triggerStationState[s].triggerTimeR=time;
		
		// generate new timestamp
		if(s==BoxStates::START)
		{
            emitNewTimeStamp(time, TimeStamp::LS);
		}
		else if(s==BoxStates::GOAL)
		{
            emitNewTimeStamp(time, TimeStamp::LGR);
		}
	}
}

void BoxStates::setBoatBoxTriggerTime(int ID, Station s, QTime time, TimeStamp::Source source)
{
    boatBoxState[ID].lastAliveTime[s]=MainWindow::app()->getTimeBaseTime();
	
	if(time > MainWindow::app()->getTimeBaseTime().addSecs(10))
	{
		qDebug() << "received timestamp from future!";
		return;
	}
	
	// check, if this time-stamp wasn't received recently
	if(!boatBoxState[ID].recentTriggerTimes.contains(time))
	{
		boatBoxState[ID].recentTriggerTimes.prepend(time);
		if(boatBoxState[ID].recentTriggerTimes.size()>10){
			boatBoxState[ID].recentTriggerTimes.removeLast();
		}

        boatBoxState[ID].triggerSource=source;
        emitNewTimeStamp(time, source, ID);
	}
}


// converts a voltage given in Volts into a battery state,
// which is in the range from 0 to 1. (for Li-Ion batterys)
double BoxStates::voltageToBatteryState(double voltage)
{
    double best = 4.0; // V
    double worst = 3.3; // V
    return std::max(0.0,std::min(1.0, (voltage-worst)*1.0/(best-worst)));
}

// converts a voltage given in Volts into a battery state,
// which is in the range from 0 to 1. (for AGM Pb batterys)
double BoxStates::voltageToBatteryStatePb(double voltage)
{
	double best = 12.4; // V
	double worst = 10.7; // V
	return std::max(0.0,std::min(1.0, (voltage-worst)*1.0/(best-worst)));
}

// converts a RSSI value given in dBm into a signal strength,
// which is in the range from 0 to 1.
double BoxStates::rssiToSignalStrength(double rssi)
{
    double best = -20; // best signal at -20dBm
    double worst = -90; // out of range at -90dBm
    return std::max(0.0,std::min(1.0, (rssi-worst)*1.0/(best-worst)));
}

// slot for Competition::boatboxBindingChanged()
void BoxStates::updateBoatNames()
{
    Competition* c = MainWindow::competition();
    for(int boatboxID=1; boatboxID<N_BOATBOXES+1; boatboxID++)
	{
		QString name=c->getBoatName(c->getBoatIdOfBoatbox(boatboxID));
		if(name.contains(QRegExp("[A-Z]")))
		{
			name.append(tr(" (Zille %1)").arg(c->getBoatIdOfBoatbox(boatboxID)));
		}
        boatBoxState[boatboxID].txtBoatName.setText(name);
    }
}

// TODO: test this function!
// slot for 5Hz timer
void BoxStates::refreshAbsentTimes()
{
	checkAbsentTimes();
	
    QTime t;

	// timebase
    t = timeBaseState.lastAliveTime;
    if(t.isValid()){
        timeBaseState.txtAbsentSince.setText(getAbsentText(t));
        t = timeBaseState.baseTime;
        if(t.isValid()){
            QTime now = MainWindow::app()->getTimeBaseTime();
            int msecs = timeBaseState.lastAliveTime.msecsTo(now); // milliseconds
            QTime extrapolatedBaseTime(timeBaseState.baseTime);
            extrapolatedBaseTime = extrapolatedBaseTime.addMSecs(msecs);
            timeBaseState.txtBaseTime.setText(MainWindow::convertTimeToString(extrapolatedBaseTime) + QString(" (%1ms)").arg(MainWindow::app()->localTimeOffset_ms));
        }
    }

	// triggerstations
    t = triggerStationState[START].lastAliveTime;
    if(t.isValid()){
        triggerStationState[START].txtAbsentSince.setText(getAbsentText(t));
    }

    t = triggerStationState[GOAL].lastAliveTime;
    if(t.isValid()){
		triggerStationState[GOAL].txtAbsentSince.setText(getAbsentText(t));
    }
	
	// matrix display
	t = matrixDisplayState[1].lastAliveTime;
    if(t.isValid()){
        matrixDisplayState[1].txtAbsentSince.setText(getAbsentText(t));
    }

    t = matrixDisplayState[2].lastAliveTime;
    if(t.isValid()){
		matrixDisplayState[2].txtAbsentSince.setText(getAbsentText(t));
    }

	// boatboxes
    for(int id=1; id<=N_BOATBOXES; id++){
        t = boatBoxState[id].lastAliveTime[START];
        if(t.isValid()){
            boatBoxState[id].txtAbsentSince[START].setText(getAbsentText(t));
        }
        t = boatBoxState[id].lastAliveTime[GOAL];
        if(t.isValid()){
            boatBoxState[id].txtAbsentSince[GOAL].setText(getAbsentText(t));
        }
    }
}

// calculates the time between time and now, and returns a
// human readable string
// e.g. 3.4s, 57s, 2m34s, 5h5m
QString BoxStates::getAbsentText(QTime time)
{
    QTime now = MainWindow::app()->getTimeBaseTime();
    int msecs = time.msecsTo(now); // milliseconds

    if(msecs < 10000){ // until 10s
        return QString("%1.%2s").arg(QString::number(msecs/1000)).arg(QString::number((msecs%1000)/100)); // e.g. "3.4 s"
    }else if(msecs < 60000){ // until 1 minute
        return QString("%1s").arg(QString::number(msecs/1000)); // e.g. "37 s"
    }else if(msecs < 36000000){
        return QString("%1m%2s").arg(QString::number(msecs/60000)).arg(QString::number((msecs%60000)/1000)); // e.g. "5m34s"
    }else{
        return QString("%1h%2m").arg(QString::number(msecs/3600000)).arg(QString::number((msecs%3600000)/60000)); // e.g. "1h59m"
    }
}


void BoxStates::checkAbsentTimes()
{
	// check timebase absent time
	if(timeBaseState.lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())>ABSENT_WARNING_LEVEL && !timeBaseState.absent)
	{
		timeBaseState.absent=true;
		MainWindow::app()->infoscreen()->appendError(tr("Verbindung zur Zeitbasis abgebrochen"));
	}
	
	// check start absent time
	if(triggerStationState[START].lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())>ABSENT_WARNING_LEVEL && !triggerStationState[START].absent)
	{
		triggerStationState[START].absent=true;
		MainWindow::app()->infoscreen()->appendError(tr("Verbindung zur Startbox abgebrochen"));
	}
	if(triggerStationState[START].lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())<ABSENT_WARNING_LEVEL && triggerStationState[START].absent)
	{
		triggerStationState[START].absent=false;
		MainWindow::app()->infoscreen()->appendInfo(tr("Verbindung zur Startbox hergestellt"));
	}
	
	// check goal absent time
	if(triggerStationState[GOAL].lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())>ABSENT_WARNING_LEVEL && !triggerStationState[GOAL].absent)
	{
		triggerStationState[GOAL].absent=true;
		MainWindow::app()->infoscreen()->appendError(tr("Verbindung zur Zielbox abgebrochen"));
	}
	if(triggerStationState[GOAL].lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())<ABSENT_WARNING_LEVEL && triggerStationState[GOAL].absent)
	{
		triggerStationState[GOAL].absent=false;
		MainWindow::app()->infoscreen()->appendInfo(tr("Verbindung zur Zielbox hergestellt"));
    }
	
	// check matrix display absent time
	for(int display=1; display<=2; display++)
	{
		if(matrixDisplayState[display].lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())>ABSENT_WARNING_LEVEL && !matrixDisplayState[display].absent)
		{
			matrixDisplayState[display].absent=true;
			MainWindow::app()->infoscreen()->appendError(tr("Verbindung zur Kiboko Matrix %1 abgebrochen").arg(display));
		}
		if(matrixDisplayState[display].lastAliveTime.msecsTo(MainWindow::app()->getTimeBaseTime())<ABSENT_WARNING_LEVEL && matrixDisplayState[display].absent)
		{
			matrixDisplayState[display].absent=false;
			MainWindow::app()->infoscreen()->appendInfo(tr("Verbindung zur Kiboko Matrix %1 hergestellt").arg(display));
		}
	}
}

void BoxStates::emitNewTimeStamp(QTime time, TimeStamp::Source source, int boatBoxID)
{
    if(time > firstTimeBaseTimeReceived){
        TimeStamp* ts=new TimeStamp(0, time, source);
        ts->setBoatboxID(boatBoxID);
        emit receivedNewTimeStamp(ts);
    }
}
