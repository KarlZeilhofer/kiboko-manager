#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogcompetitionproperties.h"
#include "dialogcsvuploadcommand.h"
#include "dialognewcompetition.h"
#include "dialoggeneralsettings.h"
#include "dialogbindboatbox.h"
#include "displaypreviewwidget.h"
#include "dialogdisplaysettings.h"
#include "displayrenderer.h"
#include "competition.h"
#include "dialogtimestampgenerator.h"
#include "dialogtimestampgenerator2.h"
#include "formtimestamplist.h"
#include "dialogbindboatbox.h"
#include "assignmentmanager.h"
#include "../../eclipse/rfsw_0.1/packet.h"
#include "crc8.h"
#include "welcomescreen.h"
#include "maintablewidget.h"
#include "timestampstablewidget.h"
#include "publisher.h"
#include "publicwidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QtNetwork>
#include <QProcess> // used for the csv upload command

#include <QtGui>

#include <unistd.h>

#define UDP_TRANSMIT_PORT (10001) // from PC to XPORT
#define UDP_LISTEN_PORT (10002) // from XPORT to PC

#define BACKUP_INTERVAL (15*60*1000) // in ms

// Defines for settings:
#define SS_RECENT_FILES "RecentFiles"
#define SS_RECENT_PATH "RecentPath" // this is used for loading/saving competitions, when no recent file was selected in the welcome screen



Competition* MainWindow::myCompetition=0;
MainWindow* MainWindow::myMainWindow=0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	setWindowIcon(QIcon(QCoreApplication::applicationDirPath()+"/misc/kiboko-logo.svg"));
	
    QSettings set;
    myMainWindow = this;
    localTimeOffset_ms = 0;
	
	startUpTime=QDateTime::currentDateTime();

    ui->setupUi(this);

	showMaximized();

	dialogNewCompetition = new DialogNewCompetition(this);
	dialogGeneralSettings = new DialogGeneralSettings(this);
	dialogDisplaySettings = new DialogDisplaySettings(this);
	dialogCsvUploadCommand = new DialogCsvUploadCommand(this);
	dialogTimeStampGenerator = 0;
	dialogTimeStampGenerator2 = new DialogTimeStampGenerator2(this);
	formTimeStampList = 0;
	serverConnected = false;
    myCompetition=0;
    publisher=0;
    udpSocket = 0;
	
	initTimeBaseListener();


    boxStatesView = new BoxStatesView(&boxStates, 0);
    if(boxStatesView){
        ui->scrollArea_boxStates->setWidget(boxStatesView);

    }else{
        qDebug("Error on creating boxStatesView");
        qApp->exit();
    }

	connect(dialogNewCompetition, SIGNAL(accepted()), this, SLOT(newCompetition()));

	// show default images on the flipdot displays
	QImage im1(":/flipdot1_default.bmp");
	ui->flipdotDisplayWidget_1->setImage(im1);
	QImage im2(":/flipdot2_default.bmp");
	ui->flipdotDisplayWidget_2->setImage(im2);


	connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


	connect(&boxStates, SIGNAL(receivedNewTimeStamp(TimeStamp*)), this, SLOT(addNewTimeStamp(TimeStamp*)));
	
    renderer = new DisplayRenderer(ui->flipdotDisplayWidget_1,
                                   ui->flipdotDisplayWidget_2,
                                   dialogDisplaySettings, this);
    if(renderer == 0){
        qDebug("Error on creating renderer-object in MainWindow-contructor");
        qApp->exit();
    }
	publisher=new Publisher(this);
    if(publisher == 0){
        qDebug("Error on creating publisher-object in MainWindow-contructor");
        qApp->exit();
    }
	
    connect(publisher, SIGNAL(publishRunNow(RunData*)), renderer, SLOT(render(RunData*)));
	connect(renderer, SIGNAL(sendDatagram(QByteArray)), this, SLOT(sendPC2TB_Packet(QByteArray)));
	connect(dialogDisplaySettings->getRenderer(), SIGNAL(sendDatagram(QByteArray)), this, SLOT(sendPC2TB_Packet(QByteArray)));

    // show second screen (public)
    publicWidget = new PublicWidget();
    if(publisher == 0){
        qDebug("Error on creating publicWideget in MainWindow-contructor");
        qApp->exit();
    }
    QRect screenres = QApplication::desktop()->screenGeometry(1); // get coordinates of the external monitor
    publicWidget->move(QPoint(screenres.x(), screenres.y()));
    //publicWidget->showFullScreen();
    //publicWidget->showMaximized(); // DO NOT MAXIMIZE HERE, so that the "Außenmonitor aktivieren" works properly
    publicWidget->setWindowTitle(tr("Öffentlicher Monitor"));
    // use closeEvent for closing this widget

    currentFile = QFileInfo(set.value(SS_RECENT_PATH).toString() + "/"); // set current file to the recent path, with no filename
    newCompetition(); // create new competition with standard values from the dialogNewCompetition
	ui->infoscreen->clear();

	connect(ui->timeline, SIGNAL(selectionChanged(TimeStamp*)), ui->tableWidget_main, SLOT(selectRun(TimeStamp*)));
	connect(ui->tableWidget_main, SIGNAL(runSelectionChanged(RunData*)), ui->timeline, SLOT(selectRun(RunData*)));
	connect(ui->tableWidget_main, SIGNAL(selectTimeStamp(TimeStamp*)), ui->timeline, SLOT(selectTimeStamp(TimeStamp*)));
	connect(ui->timeline, SIGNAL(selectionChanged(TimeStamp*)), ui->tableWidget_timeStamps, SLOT(selectTimeStamp(TimeStamp*)));
	connect(&assignmentManager, SIGNAL(unassignedTimeStamp(TimeStamp*)), ui->tableWidget_timeStamps, SLOT(selectTimeStamp(TimeStamp*)));
	connect(ui->tableWidget_timeStamps, SIGNAL(selectionChanged(TimeStamp*)), ui->timeline, SLOT(selectTimeStamp(TimeStamp*)));
	connect(ui->tableWidget_timeStamps, SIGNAL(assign()), this, SLOT(on_pushButton_assignTime_clicked()));
	
	connect(this, SIGNAL(showFoundRun(RunData*)), ui->tableWidget_main, SLOT(selectRun(RunData*)));
	connect(this, SIGNAL(showFoundRun(RunData*)), ui->timeline, SLOT(selectRun(RunData*)));

    connect(&backupTimer, SIGNAL(timeout()), this, SLOT(backupCurrentCompetition()));

	ui->tableWidget_timeStamps->init();
	ui->tableWidget_main->regenerateTable();
	
	manualTrigger=new QShortcut(QKeySequence("CTRL+Space"), this);
	connect(manualTrigger, SIGNAL(activated()), this, SLOT(on_pushButton_manualTrigger_clicked()));
#ifdef ENABLE_FDISK
	connect(&fdisk, SIGNAL(finished()), this, SLOT(writeBetweenRating()));
#endif

    setOnlineMode(false);
	
	ui->infoscreen->appendInfo(tr("Willkommen!"));

    // show welcome-screen:
    WelcomeScreen* w = new WelcomeScreen(this);
    w->show();
}

MainWindow::~MainWindow()
{
	delete ui;
}
// singelton:
Competition* MainWindow::competition()
{
	if(myCompetition == 0){
		// return dummy competition
		myCompetition = new Competition(0,"", N_BOATBOXES-N_SPARE_BOATBOXES, QString("rot grün blau").split(' '),"", "");
        myCompetition->setModified(false);
    }
	return myCompetition;
}

// use this method for a global access
MainWindow* MainWindow::app(){
	return myMainWindow;
}


InfoScreen* MainWindow::infoscreen()
{
	return ui->infoscreen;
}

//### UI-Slots ###

void MainWindow::on_pushButton_search_clicked()
{
    on_lineEdit_searchText_returnPressed();
}


void MainWindow::on_lineEdit_searchText_returnPressed()
{
	/*
	// create segfault
	int a[10];
	a[10]=0;
	*/
	
	static int ID=0;
	if(!ui->lineEdit_searchText->text().isEmpty())
	{
	
		for(int i=0; i<myCompetition->getNumOfRuns(); i++)		// give up searching after one complete cycle
		{
			// try next ID
			ID++;
			
			// when at the end of the list, continue at the beginning
			if(ID>myCompetition->getNumOfRuns())
			{
				ID=1;
			}
			
			RunData* r=myCompetition->getRun(ID);
			QString pattern=ui->lineEdit_searchText->text();
			
			switch(ui->comboBox_searchTask->currentIndex())
			{
				case 0:		// search by name
				{
					if((r->getFullName1()+" "+r->getFullName2()).contains(pattern, Qt::CaseInsensitive))
					{
						emit showFoundRun(r);
						return;
					}
				}
				break;
				case 1:		// search by ID
				{
					if(pattern.toInt() == r->getID())
					{
						emit showFoundRun(r);
						return;
					}
				}
				break;
				case 2:		// search by round
				{
					if(pattern.toInt() == r->getRound())
					{
						emit showFoundRun(r);
						return;
					}
				}
				break;
				case 3:		// search by boatID
				{
					if(pattern.toInt() == r->getBoatID())
					{
						emit showFoundRun(r);
						return;
					}
				}
				break;
				case 4:		// search by color
				{
					if(r->getColor().compare(pattern, Qt::CaseInsensitive)==0)
					{
						emit showFoundRun(r);
						return;
					}
				}
				break;
				case 5:		// search by notes
				{
					if(r->getNotes().contains(pattern, Qt::CaseInsensitive))
					{
						emit showFoundRun(r);
						return;
					}
				}
				break;
			}
		}
	}
}


void MainWindow::on_radioButton_centerNothing_clicked()
{
    ui->timeline->setCentering(TimeLine::Nothing);
}

void MainWindow::on_radioButton_centerStart_clicked()
{
    ui->timeline->setCentering(TimeLine::Start);
}

void MainWindow::on_radioButton_centerGoal_clicked()
{
    ui->timeline->setCentering(TimeLine::Goal);
}

void MainWindow::on_radioButton_centerUnassigned_clicked()
{
    ui->timeline->setCentering(TimeLine::Unassigned);
}

void MainWindow::on_pushButton_assignTime_clicked()
{
	int tsID = ui->tableWidget_timeStamps->getSelectedID();
	int runID = ui->tableWidget_main->currentRow()+1;

	RunData* run = competition()->getRun(runID);
	TimeStamp* ts = competition()->getTimeStamp(tsID);

	if(ts && run){
		assignmentManager.tryToAssignTimeStampToRun(tsID, runID);
	}
}

void MainWindow::on_pushButton_disassignTime_clicked()
{
	//qDebug("MainWindow::on_pushButton_disassignTime_clicked()");
	int runID = ui->tableWidget_main->currentRow()+1;

	assignmentManager.tryToUnassignStartGoalTimeStamp(runID);
}

void MainWindow::on_pushButton_manualTrigger_clicked()
{
	TimeStamp* ts = new TimeStamp(0, getTimeBaseTime(), TimeStamp::M);
    competition()->addTimeStamp(ts);

	assignmentManager.tryToAssignTimeStampToRun(ts->getID());
}


//#### ACTION-SLOTS #####
void MainWindow::on_actionCreateCompetition_triggered()
{
	dialogNewCompetition->exec();
}

void MainWindow::on_actionShowGeneralSettings_triggered()
{
	dialogGeneralSettings->exec();
}

void MainWindow::on_actionShowColumnsSettings_triggered()
{
	ui->tableWidget_main->showColumnsSettings();
}

void MainWindow::on_actionTimeStampGenerator_triggered()
{
	if(dialogTimeStampGenerator == 0){
		dialogTimeStampGenerator = new DialogTimeStampGenerator(this);
		connect(dialogTimeStampGenerator, SIGNAL(timeStampCreated(TimeStamp*)), this, SLOT(addNewTimeStamp(TimeStamp*)));
	}
	dialogTimeStampGenerator->show();
}

void MainWindow::on_actionViewTimeStampList_triggered()
{
	if(formTimeStampList == 0){
		formTimeStampList = new FormTimeStampList(this);
        connect(competition(), SIGNAL(timeStampChanged(TimeStamp*)), formTimeStampList, SLOT(updateTableRow(TimeStamp*)));
	}

	formTimeStampList->show();
}



// opens a file with the given fileName and onlineMode-flag
// this is the common backend for view/edit and continue open modes
bool MainWindow::openFile(QString fileName, bool onlineMode)
{
    if(competition()->isModified()){
        askForSave();
    }
#ifdef ENABLE_FDISK
	fdisk.wait();
#endif
	
    QFileInfo fi(fileName);

    if(competition()->load(fileName))			// try to load file
    {
        currentFile = fi;
        updateWindowTitle();
        //loading file was successfull
		
		// TODO: why are these connects here? they are already in newCompetition.
        connect(myCompetition, SIGNAL(runChanged(RunData*)), ui->tableWidget_main, SLOT(updateTableRow(RunData*)));
        connect(myCompetition, SIGNAL(timeStampChanged(TimeStamp*)), ui->tableWidget_timeStamps, SLOT(updateTimeStamp(TimeStamp*)));
		
        qDebug()<<"regenerating main-table...";
        ui->tableWidget_main->regenerateTable();
        qDebug()<<"regenerating timestamplist...";
        ui->tableWidget_timeStamps->regenerateList(); // SLOW_BUG
        qDebug()<<"refreshing runs...";
        ui->widget_public->refreshRuns();
        publicWidget->refreshRuns();
		boxStates.updateBoatNames();
		
        if(onlineMode){
            qDebug() << "load next runs";
            loadNextRuns();
            qDebug() << "finished";
        }
        setOnlineMode(onlineMode);
        publisher->clear();

        if(onlineMode){
            ui->infoscreen->appendInfo(tr("Bewerb zum Fortsetzen geladen"));
        }else{
            ui->infoscreen->appendInfo(tr("Bewerb zum Betrachten und Editieren geladen"));
        }
        prependToRecentFiles(currentFile.absoluteFilePath());
        return true;
    }
    else
    {
		ui->infoscreen->appendError(tr("Bewerb konnte nicht geladen werden"));
        return false;
    }
}


void MainWindow::on_actionViewAndEdit_triggered()
{
    qDebug() << __func__;
	QString fileName = QFileDialog::getOpenFileName(this,
         tr("Bewerb zum Editieren/Betrachten laden"), currentFile.absolutePath(), tr("Bewerbsdateien (*.cpt)"));


	if(fileName!=0)		// if the user selected a file
	{
        openFile(fileName, false);
    }
}


void MainWindow::on_actionSaveAs_triggered()
{
	QString fn;

    fn = QFileDialog::getSaveFileName(this,	tr("Bewerb speichern unter..."), currentFile.absolutePath(), tr("Bewerbsdateien (*.cpt)"));
    qDebug() << QString("Bewerb speichern unter: \"%1\" (line %2)").arg(fn).arg(__LINE__).toAscii();

    if(fn.isEmpty()==false){		// if the user selected a file
        if(fn.toLower().endsWith(".cpt")==false){ // append extension if not existing
            fn.append(".cpt");
		}

        bool ok = competition()->save(fn); // save file
        if(ok)
		{
			ui->infoscreen->appendInfo("Bewerb gespeichert");
            currentFile = QFileInfo(fn); // update currentFile only on success
            prependToRecentFiles(currentFile.absoluteFilePath());
            competition()->setModified(false);
        }
		else
		{
			ui->infoscreen->appendError("Bewerb konnte nicht gespeichert werden");
		}
        updateWindowTitle();
	}
}

void MainWindow::on_actionSaveCompetition_triggered()
{
	if(currentFile.isFile()){
		if(competition()->isModified()){
            if(competition()->save(currentFile.absoluteFilePath())){
                prependToRecentFiles(currentFile.absoluteFilePath());
                competition()->setModified(false);
            }
            qDebug() << QString("Bewerb speichern: \"%1\" (line %2)").arg(currentFile.absoluteFilePath()).arg(__LINE__).toAscii();
		}
	}else{
		on_actionSaveAs_triggered();
	}
    updateWindowTitle();
}

void MainWindow::on_actionExportCompetition_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this,
         tr("Bewerb exportieren"), currentFile.absolutePath(), tr("CSV-Dateien (*.csv)"));
	
	if(!fileName.isEmpty())		// if the user selected a file
	{
		if(!fileName.toLower().endsWith(".csv")) // append extension if not existing
		{
			fileName.append(".csv");
		}
		if(competition()->exportCSV(fileName))
		{
			ui->infoscreen->appendInfo(tr("Bewerb exportiert"));
		}
		else
		{
			ui->infoscreen->appendError(tr("Bewerb konnte nicht exportiert werden"));
		}
	}
}



void MainWindow::on_actionShowTimeBaseDialog_triggered()
{
	sendPC2TB_Packet();
}


// ### OTHER SLOTS ###
// slot for accepted dialogNewCompetition
void MainWindow::newCompetition()
{
	qDebug("newCompetition()");

	if(myCompetition && myCompetition->isModified()){
		askForSave();
	}
#ifdef ENABLE_FDISK
	fdisk.wait();
#endif

    if(publisher)
	{
        publisher->clear();
    }
	
	delete(myCompetition);
	myCompetition = 0;

	myCompetition = new Competition(this, dialogNewCompetition->description(), dialogNewCompetition->boatsPerRound(),
									dialogNewCompetition->colors(), "",
								  dialogNewCompetition->databaseName());

    connect(competition(), SIGNAL(timeStampChanged(TimeStamp*)), ui->tableWidget_timeStamps, SLOT(updateTimeStamp(TimeStamp*)));
	connect(competition(), SIGNAL(boatboxBindingChanged(int)), &boxStates, SLOT(updateBoatNames()));
    connect(competition(), SIGNAL(runChanged(RunData*)), ui->tableWidget_main, SLOT(updateTableRow(RunData*)));
    connect(competition(), SIGNAL(isModifiedChanged()), this, SLOT(updateWindowTitle()));
    connect(competition(), SIGNAL(modified()), &autoSaveManager, SLOT(doAutoSave()));
    connect(competition(), SIGNAL(timeStampAdded(TimeStamp*)), this, SLOT(playTriggerSound(TimeStamp*)));
    connect(competition(), SIGNAL(highestValidRunChanged(int)), this, SLOT(loadNextRuns())); // ensure, that the table is always long enough!
	connect(competition(), SIGNAL(manualPublished(RunData*)), publisher, SLOT(addToQueueManual(RunData*)));
    connect(competition(), SIGNAL(runChanged(RunData*)), ui->widget_public, SLOT(refreshRuns()));
    connect(competition(), SIGNAL(runChanged(RunData*)), publicWidget, SLOT(refreshRuns()));
	connect(competition(), SIGNAL(destroyed()), &assignmentManager, SLOT(reset()));
    connect(competition(), SIGNAL(runStarted(RunData*)), ui->widget_public, SLOT(reloadCountDown()));
    connect(competition(), SIGNAL(runStarted(RunData*)), publicWidget, SLOT(reloadCountDown()));


    if(assignmentManager.dialogBindBoatBox){
        assignmentManager.dialogBindBoatBox->setNumOfBoatsPerRound(dialogNewCompetition->boatsPerRound());
    }

	ui->tableWidget_main->regenerateTable();
    ui->tableWidget_timeStamps->regenerateList();

    loadNextRuns();
    ui->tableWidget_main->setCurrentCell(0,0); // select first row

    // add empty runs, if competition starts at an other run than runID=1
    if(dialogNewCompetition->startRunID() > 1){
        while(competition()->getNumOfRuns() < dialogNewCompetition->startRunID()){
            RunData* run = new RunData();
            competition()->addRun(run);
        }
        TimeStamp* ts1 = new TimeStamp(0, getTimeBaseTime(), TimeStamp::M);
        TimeStamp* ts2 = new TimeStamp(0, getTimeBaseTime().addMSecs(100), TimeStamp::M);
        competition()->addTimeStamp(ts1);
        competition()->addTimeStamp(ts2);

        assignmentManager.assignToStartOrGoal(ts1->getID(), dialogNewCompetition->startRunID()-1);
        assignmentManager.assignToStartOrGoal(ts2->getID(), dialogNewCompetition->startRunID()-1);

        ui->tableWidget_main->regenerateTable();
        ui->tableWidget_main->setCurrentCell(dialogNewCompetition->startRunID()-1,0); // select first row
    }


	setOnlineMode(true);
    competition()->setModified(false);
	
    QFileInfo oldFile = currentFile;
    currentFile.setFile(oldFile.absolutePath()+"/"); // set current file to no-file, but keep currend path!

    updateWindowTitle();
	boxStates.updateBoatNames();
	
	ui->infoscreen->appendInfo("Neuer Bewerb gestartet");

    backupTimer.stop();
    backupTimer.start(BACKUP_INTERVAL);
}

// slot for udpSocket::readyRead()
void MainWindow::processTimeBaseDatagrams()
{
	//qDebug("got UDP data");

    while (udpSocket->hasPendingDatagrams()) {

		// read data from UDP
		QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

		// append to input buffer
		udpInputBuffer.append(datagram);

		// while there is enough data in buffer, process next packet
		while(udpInputBuffer.size() >= (int)(sizeof(TB2PC_PACKET)))
		{
			// crop invalid bytes at the beginning
			while(udpInputBuffer.size() >= 2 &&
				  udpInputBuffer.at(0) != MAGIC_NUMBER1 &&
				  udpInputBuffer.at(1) != MAGIC_NUMBER2){
				udpInputBuffer.remove(0, 1);
				qDebug("discard incoming byte (out of sync)");
			}

			// if there is enough data for one packet and magic-number is correct
			if(udpInputBuffer.size() >= (int)(sizeof(TB2PC_PACKET)) &&
					udpInputBuffer.at(0) == MAGIC_NUMBER1 &&
					udpInputBuffer.at(1) == MAGIC_NUMBER2){

				// extract packet data from input buffer
				TB2PC_PACKET inPacket = *((TB2PC_PACKET*)udpInputBuffer.data());
				udpInputBuffer.remove(0, sizeof(TB2PC_PACKET));

				//qDebug("TB2PC_PACKET received");
				//qDebug() << "size of packet:" << sizeof(inPacket);

				// check crc
				if(CRC8_run((const uint8_t*)(&inPacket), sizeof(inPacket))==0){

					// convert endianess
	#if Q_BYTE_ORDER == Q_BIG_ENDIAN
					qDebug("Change endianness");
					changeEndianness(inPacket);
	#endif

					// check version
					if(inPacket.packetVersion == PACKET_VERSION){

						// Packet crc and version is ok

						/*
						qDebug(QString("Time-Base Voltage: %1V")
							   .arg((double)(inPacket.batteryVoltageTB)/1000.0)
							   .toAscii());*/

						boxStates.setTimeBaseVoltage(inPacket.batteryVoltageTB/1000.0);

						QTime baseTime;
						if(inPacket.baseTime_100us==TIME_INVALID)
						{
							baseTime=QTime();
							localTimeOffset_ms = 0;
						}
						else
						{
							baseTime = baseTime.addMSecs(inPacket.baseTime_100us/10);
							localTimeOffset_ms = QTime::currentTime().msecsTo(baseTime); // localTimeOffset_ms = timeBaseTime - localTime;
						}

						boxStates.setTimeBaseTime(baseTime);

						//qDebug() << "TSID:" << inPacket.triggerStationID;


						if(inPacket.triggerStationID==1 || inPacket.triggerStationID==2)
						{
							// this packet contains information about the TS

							//qDebug("packet from TS");
							BoxStates::Station s=(BoxStates::Station)BOX_PACKET_ID_INVALID;
							if(inPacket.triggerStationID==1)
							{
								s=BoxStates::START;
							}
							else
							{
								s=BoxStates::GOAL;
							}

							/*qDebug(QString("Trigger-Station Voltage: %1V")
								   .arg((double)(inPacket.tsPacket.batteryVoltageTS)/1000.0)
								   .toAscii());*/

							boxStates.setTriggerStationVoltage(s, inPacket.tsPacket.batteryVoltageTS/1000.0);
							boxStates.setTriggerStationRssi(s, num2rssi(inPacket.binRssiTS));

							// lightsensor
							if(inPacket.tsPacket.triggerTimeTS_L != TIME_INVALID)
							{
								// there is a valid trigger-time for the left side
								QTime time(0,0,0,0);
								time=time.addMSecs(inPacket.tsPacket.triggerTimeTS_L/10);
								
								boxStates.setTsTriggerTimeL(s, time);
							}

							if(inPacket.tsPacket.triggerTimeTS_R != TIME_INVALID)
							{
								// there is a valid trigger-time for the right side
								QTime time(0,0,0,0);
								time=time.addMSecs(inPacket.tsPacket.triggerTimeTS_R/10);

								boxStates.setTsTriggerTimeR(s, time);
							}


							if(inPacket.tsPacket.boatBoxID != BOX_PACKET_ID_INVALID)
							{
								// there is information from a boatbox

								int ID=inPacket.tsPacket.boatBoxID;
								
								if(ID>=1 && ID<=N_BOATBOXES)
								{
									boxStates.setBoatBoxVoltage(ID, s, inPacket.tsPacket.batteryVoltageBB/1000.0);
									boxStates.setBoatBoxRssi(ID, s, num2rssi(inPacket.tsPacket.binRssiBB));
	
									/*qDebug(QString("Boatbox Voltage: %1V")
										   .arg((double)(inPacket.tsPacket.batteryVoltageBB)/1000.0)
										   .toAscii());*/
	
									if(inPacket.tsPacket.triggerTimeBB != TIME_INVALID)
									{
										// there is valid trigger-time for boatbox
	
										QTime time(0,0,0,0);
										time=time.addMSecs((inPacket.tsPacket.triggerTimeBB+5)/10); // correct rounding to 0.01s
	
										switch(inPacket.tsPacket.stationTriggeredAt)
										{
											case 1: boxStates.setBoatBoxTriggerTime(ID, s, time, TimeStamp::TSS); break;
											case 2: boxStates.setBoatBoxTriggerTime(ID, s, time, TimeStamp::TSGL); break;
											case 3: boxStates.setBoatBoxTriggerTime(ID, s, time, TimeStamp::TSGR); break;
										}
									}
								}
								else
								{
									qDebug() << "packet from invalid boat box";
								}
							}
						}
						// packet from matrix display
						else if(inPacket.triggerStationID==3 || inPacket.triggerStationID==4)
						{
							int display=inPacket.triggerStationID-2;
							boxStates.setMatrixDisplayVoltage(display, inPacket.tsPacket.batteryVoltageTS/1000.0);
							boxStates.setMatrixDisplayRssi(display, num2rssi(inPacket.binRssiTS));
						}
					}
					else
					{
						qDebug("incorrect packet version");
					}
				}
				else
				{
					qDebug("CRC Error on inPacket");
				}
			}
		}
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	int ret = QMessageBox::question(this, tr("Beenden?"), tr("Soll das Programm wirklich beendet werden?"),
						  QMessageBox::No, QMessageBox::Yes);
	if(ret == QMessageBox::Yes){
		if(competition()->isModified()){
			askForSave();
		}
#ifdef ENABLE_FDISK
		fdisk.wait();
#endif
        publicWidget->close();
		event->accept();
	}else{
		event->ignore();
    }
}

void MainWindow::prependToRecentFiles(QString filepath)
{
    QSettings set;
    QStringList list = set.value(SS_RECENT_FILES).toStringList();

    list.prepend(filepath);
    list.removeDuplicates();

    QStringList newList;
    for(int n=0; n<10 && n<list.length(); n++){
        newList.append(list.at(n));
    }

    set.setValue(SS_RECENT_FILES, newList);

    QFileInfo info(filepath);
    set.setValue(SS_RECENT_PATH, info.absolutePath());
}

void MainWindow::sourcesStatistics()
{
    Competition* c = competition();
    TimeStamp* ts=0;

    int ls=0;
    int lgl=0;
    int lgr=0;
    int tss=0;
    int tsgl=0;
    int tsgr=0;

    int bbStart[N_BOATBOXES+1]={0};
    int bbGoalL[N_BOATBOXES+1]={0};
    int bbGoalR[N_BOATBOXES+1]={0};

    for(int rID=1; rID<=c->getNumOfRuns(); rID++){
        ts = c->getTimeStamp(c->getRun(rID)->getStartTimeID());
        if(ts){
            if(ts->getSourceName() == "LS") ls++;
            if(ts->getSourceName() == "TSS"){
                tss++;
                bbStart[ts->getBoatboxID()]++;
            }
        }

        ts = c->getTimeStamp(c->getRun(rID)->getGoalTimeID());
        if(ts){
            if(ts->getSourceName() == "LGL") lgl++;
            if(ts->getSourceName() == "LGR") lgr++;
            if(ts->getSourceName() == "TSGL"){
                tsgl++;
                bbGoalL[ts->getBoatboxID()]++;
            }
            if(ts->getSourceName() == "TSGR"){
                tsgr++;
                bbGoalR[ts->getBoatboxID()]++;
            }
        }
    }

    qDebug() << "LS =" << ls << ", TSS = " << tss << ", LGL =" << lgl << ", LGR" << lgr << ", TSGL = " << tsgl << ", TSGR =" << tsgr;

    for(int n=1; n<=N_BOATBOXES; n++){
        qDebug() << QString("BB%1: Start(%2), GoalL(%3), GoalR(%4)").arg(n).arg(bbStart[n]).arg(bbGoalL[n]).arg(bbGoalR[n]);
    }
}

// slot for Competition::isModifiedChanged()
void MainWindow::updateWindowTitle()
{
    QString str("Kiboko Manager - ");

	QString date = competition()->getCreationDate().toString("dd.MM.yyyy");

	QString file = currentFile.fileName();
	if(file.isEmpty()){
		file = QString(tr("unbenannt"));
	}

	str.append(file + " vom " + date);

    str.append(QString(" (%1)").arg(QHostInfo::localHostName()));

	if(competition()->isModified()){
		str.prepend("*");
	}

	setWindowTitle(str);
}

// slot for timestampgenerator and boxstates:
void MainWindow::addNewTimeStamp(TimeStamp* ts)
{
	competition()->addTimeStamp(ts);
	assignmentManager.tryToAssignTimeStampToRun(ts->getID());
}

// slot for resizing the main-table
void MainWindow::loadNextRuns()
{
    //qDebug("loadNextRuns()");
    if(competition() &&
            (competition()->getNumOfRuns() - competition()->getHighestValidRunID()) < competition()->getBoatsPerRound()){
        // generate continuing run-entries in the runs-table
        for(int n=0; n<competition()->getBoatsPerRound(); n++){
            RunData* run = new RunData();
            competition()->addRun(run);
        }
#ifdef ENABLE_FDISK
		// start fdisk thread, update data
		fdisk.start();
#endif
        ui->tableWidget_main->regenerateTable();
    }
}



// ### INIT-FUNCTIONS ###
void MainWindow::initTimeBaseListener()
{
    if(udpSocket == 0){
        udpSocket = new QUdpSocket(this);
        if(udpSocket == 0){
            qDebug("Error on creating UDP port object");
        }
		// the final binding happens in setOnlineMode()!
        //udpSocket->bind(UDP_LISTEN_PORT, QUdpSocket::ShareAddress);
        connect(udpSocket, SIGNAL(readyRead()),
                this, SLOT(processTimeBaseDatagrams()));
    }
}


// ### FUNCTIONS ###
void MainWindow::askForSave()
{
	//qDebug("askForSave()");
	int ret = QMessageBox::question(this, tr("Speichern?"), tr("Soll der aktuelle Bewerb vorher gespeichert werden?"),
						  QMessageBox::No, QMessageBox::Yes);
	if(ret == QMessageBox::Yes){
		on_actionSaveCompetition_triggered();
	}
}

void MainWindow::changeEndianness(TB2PC_PACKET* p) {
	flipBytes(&p->packetVersion, sizeof(p->packetVersion));
	flipBytes(&p->batteryVoltageTB, sizeof(p->batteryVoltageTB));

	flipBytes(&(p->tsPacket.triggerTimeBB), sizeof(p->tsPacket.triggerTimeBB));
	flipBytes(&(p->tsPacket.triggerTimeTS_L), sizeof(p->tsPacket.triggerTimeTS_L));
	flipBytes(&(p->tsPacket.triggerTimeTS_R), sizeof(p->tsPacket.triggerTimeTS_R));
	flipBytes(&(p->tsPacket.batteryVoltageTS), sizeof(p->tsPacket.batteryVoltageTS));
	flipBytes(&(p->tsPacket.batteryVoltageBB), sizeof(p->tsPacket.batteryVoltageBB));
}

// invert the order of bytes in the buffer start with the length len
void MainWindow::flipBytes(void* start, int len) {
	int n;
	uint8_t* buf = (uint8_t*) start;
	uint8_t h; // helper

	for (n = 0; n < len / 2; n++) {
		h = buf[n];
		buf[n] = buf[len - 1 - n];
		buf[len - 1 - n] = h;
	}
}

// returns the rssi value in dB of a given binary rssi value
double MainWindow::num2rssi(uint8_t num)
{
	const uint8_t rssi_offset = 71;
	double rssi=num;
	if (rssi >= 128)
	{
		rssi = (rssi - 256) / 2 - rssi_offset;
	}
	else
	{
		rssi = rssi / 2 - rssi_offset;
	}
	return rssi;
}

// use this function, to get a time-string with 0.01s precision
QString MainWindow::convertTimeToString(QTime t)
{
	QString str;

	if(t.hour() == 0){
		str = t.toString("m:ss.zzz");
	}else{
		str = t.toString("h:mm:ss.zzz");
	}
	str.remove(str.size()-1, 1); // remove ms-digit
    return str;
}

QFileInfo MainWindow::getCurrentFile()
{
    return currentFile;
}


void MainWindow::playTriggerSound(TimeStamp* ts)
{
	if(dialogGeneralSettings->soundOnTrigger())
	{
		switch(ts->getSource())
		{
			case TimeStamp::LS: case TimeStamp::LGL: case TimeStamp::LGR: playSoundFile("misc/blitz.wav"); break;
			case TimeStamp::TSS: case TimeStamp::TSGL: case TimeStamp::TSGR: playSoundFile("misc/napf.wav"); break;
			default: break; // no sound for manual and edited
		}
	}
}

void MainWindow::playErrorSound()
{
	playSoundFile("misc/dingding.wav");
}

void MainWindow::playStartSound()
{
	if(dialogGeneralSettings->soundOnStart())
	{
		playSoundFile("misc/klappe.wav");
	}
}

void MainWindow::playSoundFile(QString filename)
{
	if(!filename.isEmpty())
	{
		#ifndef Q_OS_WIN
		if(fork()==0)
		{
			execl("/usr/bin/aplay", "aplay", "-q", filename.toAscii().data(), NULL);
		}
		#endif
	}
}


void MainWindow::sendPC2TB_Packet()
{
	// send a dummy packet (to synchronice the timebase)
	
	PC2TB_PACKET outPacket;

	// pack tha packet:
	outPacket.magicNumber1 = MAGIC_NUMBER1;
	outPacket.magicNumber2 = MAGIC_NUMBER2;
	outPacket.packetVersion = PACKET_VERSION;
	QTime midnight(0,0);
	/*
	 * after restarting the timebase it should get the same time after synchonicing again
	 * so this is a bug, not a feature
	 */
    int time_ms = midnight.msecsTo(MainWindow::app()->getTimeBaseTime());
	uint32_t time_100us = 10*time_ms;
	outPacket.pcTime_100us = time_100us;
	
	// text for ALGE display
	//                     123456789ABCDEFGHIJKLMNO
	QString textA=QString("A          :  :  .     \r");
	//                     123456789ABCDEFGHIJKLMNO
	QString textB=QString("B          :  :  .     \r");
	memcpy(outPacket.algeText, (textA+textB).toAscii().data(), ALGE_TEXT_LEN);
	
	// set all unused variables to invalid
	outPacket.matrixDataValid=false;
	outPacket.cmdReceiver=COMMAND_INVALID_RECEIVER;
	outPacket.command=COMMAND_INVALID_CMD;
	
	
	
	outPacket.crc = CRC8_run((uint8_t*)(&outPacket), (uint16_t)(sizeof(PC2TB_PACKET)-1));
	
	QByteArray datagram;
	datagram.setRawData((char*)(&outPacket), sizeof(PC2TB_PACKET));
    sendPC2TB_Packet(datagram);
}

void MainWindow::sendPC2TB_Packet(QByteArray datagram)
{
	//qDebug() << "sizeof(PC2TB_PACKET):" << sizeof(PC2TB_PACKET);
	udpSocket->writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, UDP_TRANSMIT_PORT);
}


void MainWindow::on_actionShowDisplaySettings_triggered()
{
	dialogDisplaySettings->exec();
}



void MainWindow::on_actionCompetitionProperties_triggered()
{
    DialogCompetitionProperties dlg(this);
    dlg.exec();
}


// this is the central function for setting online mode
void MainWindow::setOnlineMode(bool online)
{
	if(online)
	{
		if(udpSocket->state()!=QAbstractSocket::BoundState)
		{
			qDebug() << "open UDP";
			udpSocket->bind(UDP_LISTEN_PORT, QUdpSocket::ShareAddress);
		}
	}
	else
	{
		qDebug() << "close UDP";
		udpSocket->close();
	}
    ui->timeline->setOnlineMode(online);
    ui->pushButton_manualTrigger->setEnabled(online);
    ui->tableWidget_main->setOnlineMode(online);
    ui->tableWidget_timeStamps->setOnlineMode(online);
}

// provide an up to date access to the time of the time-base
// use this in every place, where you would use QTime::currentTime();
QTime MainWindow::getTimeBaseTime()
{
    //qDebug(__func__);
    //qDebug() << "localOffset = ";// << localTimeOffset_ms;
    QTime t0(0,0);
    uint32_t ms=0;
    ms = t0.msecsTo(QTime::currentTime());
    ms += localTimeOffset_ms;

    ms = ((ms+5)/10)*10; // round to 0.01s
    QTime t(0,0);
    t = t.addMSecs(ms);

    return t;
}

void MainWindow::on_pushButton_autopublish_clicked(bool checked)
{
	// if the autopublish button is checked, send a finished run directly to the publisher
	if(checked)
	{
		connect(competition(), SIGNAL(runFinished(RunData*)), publisher, SLOT(addToQueueAuto(RunData*)));
	}
	else
	{
		disconnect(competition(), SIGNAL(runFinished(RunData*)), publisher, SLOT(addToQueueAuto(RunData*)));
    }
}

// slot of backupTimer, comes every 15 minutes
// and creates repeatedly backups.
// don't mix it up with autosave - which saves a parallel copy on every change of the competition
void MainWindow::backupCurrentCompetition()
{
    QString fileName;
    fileName.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    fileName.append("_backup_of_");
    if(currentFile.fileName().isEmpty()){
        fileName.append(tr("unbenannt.cpt"));
    }else{
        fileName.append(currentFile.fileName());
    }

    QString path = qApp->applicationDirPath();
    path.append("/backups/");

    // make the directory, if it does not exist
    QFileInfo dirInfo(path);
    if(dirInfo.exists() == false){
        QDir dir(path);
        dir.mkpath(path);
    }

    bool modified = competition()->isModified(); // backup modified-flag
    if(competition()->save(path+fileName)){
        qDebug() << "wrote backup to:" << path+fileName;
        infoscreen()->appendInfo(tr("Bewerbsdatei wurde gesichert"));
    }else{
        qDebug() << "ERROR on writing backup to:" << path+fileName;
        infoscreen()->appendWarning(tr("Sichern der Bewerbsdatei gescheitert!"));
    }
    competition()->setModified(modified);
}

// TODO: relative path to pdf-file
void MainWindow::on_actionOpenDocumentation_triggered()
{
    QDesktopServices desk;
    desk.openUrl(QUrl::fromLocalFile(QDir::currentPath() + "/misc/doku.pdf"));
}

void MainWindow::on_actionCheatSheet_triggered()
{
	QDesktopServices desk;
    desk.openUrl(QUrl::fromLocalFile(QDir::currentPath() + "/misc/cheetsheet.pdf"));
}

void MainWindow::on_actionDebugStatistics_triggered()
{
    sourcesStatistics();
}

#ifdef ENABLE_FDISK
void MainWindow::on_actionReadFdiskData_triggered()
{
    fdisk.start();
}
#endif

void MainWindow::on_actionPublish_all_triggered()
{
    Competition* c=competition();
	
	for(int ID=1; ID<=c->getNumOfRuns(); ID++)
	{
		c->getRun(ID)->setPublished(true);
	}
}

void MainWindow::on_actionSetupPublicWidget_triggered()
{
	// call the config-screens script to setup external monitor
	QProcess* process=new QProcess(this);
	connect(process, SIGNAL(finished(int)), this, SLOT(configScreenFinished()));
	#ifndef Q_OS_WIN
	process->start("/bin/bash", QStringList() << QDir::currentPath()+"/misc/config-screens.sh");
	#endif
}

void MainWindow::configScreenFinished()
{
	// put the public widget on the external monitor
	QRect screenres = QApplication::desktop()->screenGeometry(1); // get coordinates of the external monitor
    publicWidget->move(QPoint(screenres.x(), screenres.y()));
    //publicWidget->showFullScreen();
	publicWidget->showMaximized();
}

void MainWindow::writeBetweenRating()
{
	// export to CSV-file
	competition()->exportCSV(QDir::homePath()+"/Bewerbe/Zwischenwertung.csv");

    // run the upload command:
    static QProcess* process = 0;

    if(process == 0){
        process = new QProcess(this);
    }

    if(process != 0){
		QString cmd = dialogCsvUploadCommand->commandText();

		QStringList params = dialogCsvUploadCommand->parameters();
		params.replaceInStrings("%fdisk_id", competition()->getDatabaseName()); // replace the formatstring

		process->start(cmd, params);
		qDebug() << "Run command: " << cmd << params;
    }else{
        qDebug() << "Error on running the CSV upload command";
    }
}

void MainWindow::on_actionCSV_Upload_Command_triggered()
{
    dialogCsvUploadCommand->show();
}

void MainWindow::on_actionTimeStamp_Generator_2_triggered()
{
	dialogTimeStampGenerator2->show();
}

void MainWindow::on_actionReset_Startbox_triggered()
{
    sendRadioCommand(0x0000 | 2, RCmd_RESET);
}

void MainWindow::on_actionReset_Goalbox_triggered()
{
    sendRadioCommand(0x0000 | 3, RCmd_RESET);
}

void MainWindow::sendRadioCommand(uint16_t cmdReceiver, uint8_t command)
{
	// send a command packet
	
	qDebug() << "send radio command";
	
	PC2TB_PACKET outPacket;

	// the rest of the packet is filled with dummy data
	outPacket.magicNumber1 = MAGIC_NUMBER1;
	outPacket.magicNumber2 = MAGIC_NUMBER2;
	outPacket.packetVersion = PACKET_VERSION;
	QTime midnight(0,0);
	/*
	 * after restarting the timebase it should get the same time after synchonicing again
	 * so this is a bug, not a feature
	 */
    int time_ms = midnight.msecsTo(MainWindow::app()->getTimeBaseTime());
	uint32_t time_100us = 10*time_ms;
	outPacket.pcTime_100us = time_100us;
	
	// text for ALGE display
	//                     123456789ABCDEFGHIJKLMNO
	QString textA=QString("A          :  :  .     \r");
	//                     123456789ABCDEFGHIJKLMNO
	QString textB=QString("B          :  :  .     \r");
	memcpy(outPacket.algeText, (textA+textB).toAscii().data(), ALGE_TEXT_LEN);
	
	// set all unused variables to invalid
	outPacket.matrixDataValid=0;
	outPacket.cmdReceiver=cmdReceiver;
	outPacket.command=command;
	
	
	
	outPacket.crc = CRC8_run((uint8_t*)(&outPacket), (uint16_t)(sizeof(PC2TB_PACKET)-1));
	
	QByteArray datagram;
	datagram.setRawData((char*)(&outPacket), sizeof(PC2TB_PACKET));
    sendPC2TB_Packet(datagram);
}


QDateTime MainWindow::getStartUpTime()
{
	return startUpTime;
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("Über Kiboko-Manager"), tr("von:\nKarl Zeilhofer\nFriedrich Feichtinger\n\nkompiliert am: ")+QString(BUILDDATE)+"  "+QString(BUILDTIME));
}
