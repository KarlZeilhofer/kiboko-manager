#include "boxstatesview.h"
#include <QLabel>
#include <QtGui>
#include "boxstates.h"

BoxStatesView::BoxStatesView( BoxStates* bs, QWidget *parent) :
    QWidget(parent)
{


    // generate the layout:
    QGridLayout* l = new QGridLayout(this);
	l->setSpacing(0);	// remove spacing between widgets
	
	QString lightStyle=QString("QLabel { background-color : white; }");
	QString darkStyle=QString("QLabel { background-color : lightgray; }");


	int r0 = 0;
	QString style=darkStyle;

    // Zeitbasis:
    topHeadings.append(new QLabel(tr("Basiszeit")));
	l->addWidget(topHeadings.last(), r0, 1);
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Letzte IP-Nachricht\nvon Zeitbasis vor")));
	l->addWidget(topHeadings.last(), r0, 3);

    leftHeadings.append(new QLabel(tr("Zeitbasis")));
	leftHeadings.last()->setStyleSheet(style);
	l->addWidget(leftHeadings.last(), r0+1,0);

    entries.append(&(bs->timeBaseState.txtBaseTime));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,1);
    entries.append(&(bs->timeBaseState.txtVoltage));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,2);
    entries.append(&(bs->timeBaseState.txtAbsentSince));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,3);
	
	l->addWidget(new QLabel(), r0+1,4);
	l->itemAtPosition(r0+1,4)->widget()->setStyleSheet(darkStyle);
	l->addWidget(new QLabel(), r0+1,5);
	l->itemAtPosition(r0+1,5)->widget()->setStyleSheet(darkStyle);
	l->addWidget(new QLabel(), r0+1,6);
	l->itemAtPosition(r0+1,6)->widget()->setStyleSheet(darkStyle);

	r0 = 2;

    // Start/Ziel
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Signalstärke\nzur Zeitbasis")));
	l->addWidget(topHeadings.last(), r0, 3);
    topHeadings.append(new QLabel(tr("Letzte Nachricht\nüber Zeitbasis vor")));
	l->addWidget(topHeadings.last(), r0, 4);

    leftHeadings.append(new QLabel(tr("Start")));
	leftHeadings.last()->setStyleSheet(style);
	l->addWidget(leftHeadings.last(), r0+1,0);
    leftHeadings.append(new QLabel(tr("Ziel")));
	leftHeadings.last()->setStyleSheet(style);
	l->addWidget(leftHeadings.last(), r0+2,0);
	

    int id=BoxStates::START;
	l->addWidget(new QLabel(), r0+1,1);
	l->itemAtPosition(r0+1,1)->widget()->setStyleSheet(darkStyle);
    entries.append(&(bs->triggerStationState[id].txtVoltage));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,2);
    entries.append(&(bs->triggerStationState[id].txtSignalStrength));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,3);
    entries.append(&(bs->triggerStationState[id].txtAbsentSince));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,4);
	l->addWidget(new QLabel(), r0+1,5);
	l->itemAtPosition(r0+1,5)->widget()->setStyleSheet(darkStyle);
	l->addWidget(new QLabel(), r0+1,6);
	l->itemAtPosition(r0+1,6)->widget()->setStyleSheet(darkStyle);
	
    id=BoxStates::GOAL;
	l->addWidget(new QLabel(), r0+2,1);
	l->itemAtPosition(r0+2,1)->widget()->setStyleSheet(darkStyle);
    entries.append(&(bs->triggerStationState[id].txtVoltage));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+2,2);
    entries.append(&(bs->triggerStationState[id].txtSignalStrength));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+2,3);
    entries.append(&(bs->triggerStationState[id].txtAbsentSince));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+2,4);
	l->addWidget(new QLabel(), r0+2,5);
	l->itemAtPosition(r0+2,5)->widget()->setStyleSheet(darkStyle);
	l->addWidget(new QLabel(), r0+2,6);
	l->itemAtPosition(r0+2,6)->widget()->setStyleSheet(darkStyle);
	

	r0 = 5;
	
	// Matrix displays
    topHeadings.append(new QLabel(tr("Akkustand")));
	l->addWidget(topHeadings.last(), r0, 2);
    topHeadings.append(new QLabel(tr("Signalstärke\nzur Zeitbasis")));
	l->addWidget(topHeadings.last(), r0, 3);
    topHeadings.append(new QLabel(tr("Letzte Nachricht\nüber Zeitbasis vor")));
	l->addWidget(topHeadings.last(), r0, 4);

    leftHeadings.append(new QLabel(tr("Kiboko Matrix 1")));
	leftHeadings.last()->setStyleSheet(style);
	l->addWidget(leftHeadings.last(), r0+1,0);
    leftHeadings.append(new QLabel(tr("Kiboko Matrix 2")));
	leftHeadings.last()->setStyleSheet(style);
	l->addWidget(leftHeadings.last(), r0+2,0);

    int d=1;
	l->addWidget(new QLabel(), r0+1,1);
	l->itemAtPosition(r0+1,1)->widget()->setStyleSheet(darkStyle);
    entries.append(&(bs->matrixDisplayState[d].txtVoltage));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,2);
    entries.append(&(bs->matrixDisplayState[d].txtSignalStrength));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,3);
    entries.append(&(bs->matrixDisplayState[d].txtAbsentSince));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+1,4);
	l->addWidget(new QLabel(), r0+1,5);
	l->itemAtPosition(r0+1,5)->widget()->setStyleSheet(darkStyle);
	l->addWidget(new QLabel(), r0+1,6);
	l->itemAtPosition(r0+1,6)->widget()->setStyleSheet(darkStyle);
	
    d=2;
	l->addWidget(new QLabel(), r0+2,1);
	l->itemAtPosition(r0+2,1)->widget()->setStyleSheet(darkStyle);
    entries.append(&(bs->matrixDisplayState[d].txtVoltage));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+2,2);
    entries.append(&(bs->matrixDisplayState[d].txtSignalStrength));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+2,3);
    entries.append(&(bs->matrixDisplayState[d].txtAbsentSince));
	entries.last()->setStyleSheet(style);
	l->addWidget(entries.last(), r0+2,4);
	l->addWidget(new QLabel(), r0+2,5);
	l->itemAtPosition(r0+2,5)->widget()->setStyleSheet(darkStyle);
	l->addWidget(new QLabel(), r0+2,6);
	l->itemAtPosition(r0+2,6)->widget()->setStyleSheet(darkStyle);

	
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

    for(int id=1; id<=N_BOATBOXES; id++)
	{
		QString style;
		if(id%2==0)
		{
			style=lightStyle;
		}
		else
		{
			style=darkStyle;
		}
		
        leftHeadings.append(new QLabel(QString(tr("Boat-Box "))+QString::number(id)));
		leftHeadings.last()->setStyleSheet(style);
		l->addWidget(leftHeadings.last(), r0+id,0);

        entries.append(&(bs->boatBoxState[id].txtBoatName));
		entries.last()->setStyleSheet(style);
		l->addWidget(entries.last(), r0+id,1);
        entries.append(&(bs->boatBoxState[id].txtVoltage));
		entries.last()->setStyleSheet(style);
		l->addWidget(entries.last(), r0+id,2);
        entries.append(&(bs->boatBoxState[id].txtSignalStrength[BoxStates::START]));
		entries.last()->setStyleSheet(style);
		l->addWidget(entries.last(), r0+id,3);
        entries.append(&(bs->boatBoxState[id].txtAbsentSince[BoxStates::START]));
		entries.last()->setStyleSheet(style);
		l->addWidget(entries.last(), r0+id,4);
        entries.append(&(bs->boatBoxState[id].txtSignalStrength[BoxStates::GOAL]));
		entries.last()->setStyleSheet(style);
		l->addWidget(entries.last(), r0+id,5);
        entries.append(&(bs->boatBoxState[id].txtAbsentSince[BoxStates::GOAL]));
		entries.last()->setStyleSheet(style);
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
