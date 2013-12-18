#include "boxstatesview.h"
#include <QLabel>
#include <QtGui>
#include "boxstates.h"

BoxStatesView::BoxStatesView( BoxStates* bs, QWidget *parent) :
    QWidget(parent)
{


    // generate the layout:
    QGridLayout* l = new QGridLayout(this);

	int r0 = 0;

    // Zeitbasis:
    topHeadings.append(new QLabel(tr("Basiszeit")));
	l->addWidget(topHeadings.last(), r0, 1);
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Letzte IP-Nachricht\nvon Zeitbasis vor")));
	l->addWidget(topHeadings.last(), r0, 3);

    leftHeadings.append(new QLabel(tr("Zeitbasis")));
	l->addWidget(leftHeadings.last(), r0+1,0);

    entries.append(&(bs->timeBaseState.txtBaseTime));
	l->addWidget(entries.last(), r0+1,1);
    entries.append(&(bs->timeBaseState.txtVoltage));
	l->addWidget(entries.last(), r0+1,2);
    entries.append(&(bs->timeBaseState.txtAbsentSince));
	l->addWidget(entries.last(), r0+1,3);


	r0 = 2;

    // Start/Ziel
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Signalstärke\nzur Zeitbasis")));
	l->addWidget(topHeadings.last(), r0, 3);
    topHeadings.append(new QLabel(tr("Letzte Nachricht\nüber Zeitbasis vor")));
	l->addWidget(topHeadings.last(), r0, 4);

    leftHeadings.append(new QLabel(tr("Start")));
	l->addWidget(leftHeadings.last(), r0+1,0);
    leftHeadings.append(new QLabel(tr("Ziel")));
	l->addWidget(leftHeadings.last(), r0+2,0);

    int id=BoxStates::START;
    entries.append(&(bs->triggerStationState[id].txtVoltage));
	l->addWidget(entries.last(), r0+1,2);
    entries.append(&(bs->triggerStationState[id].txtSignalStrength));
	l->addWidget(entries.last(), r0+1,3);
    entries.append(&(bs->triggerStationState[id].txtAbsentSince));
	l->addWidget(entries.last(), r0+1,4);
    id=BoxStates::GOAL;
    entries.append(&(bs->triggerStationState[id].txtVoltage));
	l->addWidget(entries.last(), r0+2,2);
    entries.append(&(bs->triggerStationState[id].txtSignalStrength));
	l->addWidget(entries.last(), r0+2,3);
    entries.append(&(bs->triggerStationState[id].txtAbsentSince));
	l->addWidget(entries.last(), r0+2,4);
	

	r0 = 5;
	
	// Matrix displays
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Signalstärke\nzur Zeitbasis")));
	l->addWidget(topHeadings.last(), r0, 3);
    topHeadings.append(new QLabel(tr("Letzte Nachricht\nüber Zeitbasis vor")));
	l->addWidget(topHeadings.last(), r0, 4);

    leftHeadings.append(new QLabel(tr("Kiboko Matrix 1")));
	l->addWidget(leftHeadings.last(), r0+1,0);
    leftHeadings.append(new QLabel(tr("Kiboko Matrix 2")));
	l->addWidget(leftHeadings.last(), r0+2,0);

    int d=1;
    entries.append(&(bs->matrixDisplayState[d].txtVoltage));
	l->addWidget(entries.last(), r0+1,2);
    entries.append(&(bs->matrixDisplayState[d].txtSignalStrength));
	l->addWidget(entries.last(), r0+1,3);
    entries.append(&(bs->matrixDisplayState[d].txtAbsentSince));
	l->addWidget(entries.last(), r0+1,4);
    d=2;
    entries.append(&(bs->matrixDisplayState[d].txtVoltage));
	l->addWidget(entries.last(), r0+2,2);
    entries.append(&(bs->matrixDisplayState[d].txtSignalStrength));
	l->addWidget(entries.last(), r0+2,3);
    entries.append(&(bs->matrixDisplayState[d].txtAbsentSince));
	l->addWidget(entries.last(), r0+2,4);

	
	r0 = 8;

    // Boat-Boxes
    topHeadings.append(new QLabel(tr("Zille")));
	l->addWidget(topHeadings.last(), r0, 1);
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Signalstärke\nzum Start")));
	l->addWidget(topHeadings.last(), r0, 3);
    topHeadings.append(new QLabel(tr("Letzte Nachricht\nüber Start vor")));
	l->addWidget(topHeadings.last(), r0, 4);
    topHeadings.append(new QLabel(tr("Signalstärke\nzum Ziel")));
	l->addWidget(topHeadings.last(), r0, 5);
    topHeadings.append(new QLabel(tr("Letzte Nachricht\nüber Ziel vor")));
	l->addWidget(topHeadings.last(), r0, 6);

    for(int id=1; id<=N_BOATBOXES; id++){
        leftHeadings.append(new QLabel(QString(tr("Boat-Box "))+QString::number(id)));
		l->addWidget(leftHeadings.last(), r0+id,0);

        entries.append(&(bs->boatBoxState[id].txtBoatName));
		l->addWidget(entries.last(), r0+id,1);
        entries.append(&(bs->boatBoxState[id].txtVoltage));
		l->addWidget(entries.last(), r0+id,2);
        entries.append(&(bs->boatBoxState[id].txtSignalStrength[BoxStates::START]));
		l->addWidget(entries.last(), r0+id,3);
        entries.append(&(bs->boatBoxState[id].txtAbsentSince[BoxStates::START]));
		l->addWidget(entries.last(), r0+id,4);
        entries.append(&(bs->boatBoxState[id].txtSignalStrength[BoxStates::GOAL]));
		l->addWidget(entries.last(), r0+id,5);
        entries.append(&(bs->boatBoxState[id].txtAbsentSince[BoxStates::GOAL]));
		l->addWidget(entries.last(), r0+id,6);
    }

    this->setBackgroundRole(QPalette::Light);

	// set headings to bold font:
	QFont f = topHeadings.first()->font();
	f.setBold(true);

	for(int n=0; n<topHeadings.size(); n++){
		topHeadings.at(n)->setFont(f);
	}
	for(int n=0; n<leftHeadings.size(); n++){
		leftHeadings.at(n)->setFont(f);
	}

}
