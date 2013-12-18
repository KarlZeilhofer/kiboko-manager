#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "assignmentmanager.h"
#include "autosavemanager.h"
#include "../../eclipse/rfsw_0.1/packet.h"
#include "boxstates.h"
#include "boxstatesview.h"

#ifndef Q_OS_WIN
	#include "fdisk.h"
#endif

#include <typeinfo>

#include <QFileInfo>
#include <QTimer>
#include <QShortcut>


#define __CLASS__ typeid(*this).name()

namespace Ui {
    class MainWindow;
}

class DialogCsvUploadCommand;
class DialogNewCompetition;
class DialogGeneralSettings;
class Competition;
class DialogVisibleColumns;
class RunData;
class DialogEditNames;
class DialogEditTime;
class DialogTimeStampGenerator;
class DialogTimeStampGenerator2;
class TimeStamp;
class FormTimeStampList;
class DialogBindBoatBox;
class QUdpSocket;
class DialogDisplaySettings;
class Publisher;
class DisplayRenderer;
class InfoScreen;
class PublicWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	static Competition* competition();
	static MainWindow* app();
	InfoScreen* infoscreen();
	static QString convertTimeToString(QTime t);
    QFileInfo getCurrentFile();
    bool openFile(QString fileName, bool onlineMode);
    void askForSave();

private:
	void initTimeBaseListener();
	void changeEndianness(TB2PC_PACKET* packet);
	void flipBytes(void* start, int len);
	double num2rssi(uint8_t num);
    void closeEvent(QCloseEvent *event);
    void prependToRecentFiles(QString filepath);
    void sourcesStatistics();


private slots:

	void newCompetition();
	void addNewTimeStamp(TimeStamp* ts);
	void updateWindowTitle();

	void on_lineEdit_searchText_returnPressed();
	void on_pushButton_assignTime_clicked();
	void on_pushButton_disassignTime_clicked();
	void on_pushButton_manualTrigger_clicked();

	void on_actionShowColumnsSettings_triggered();
	void on_actionShowGeneralSettings_triggered();
	void on_actionTimeStampGenerator_triggered();
	void on_actionViewTimeStampList_triggered();
	void on_actionSaveCompetition_triggered();
    void on_actionShowTimeBaseDialog_triggered();

    void on_actionSaveAs_triggered();
	void on_radioButton_centerNothing_clicked();
	void on_radioButton_centerStart_clicked();
    void on_radioButton_centerGoal_clicked();
	void on_radioButton_centerUnassigned_clicked();
	
	void on_pushButton_search_clicked();
    void on_actionShowDisplaySettings_triggered();
	void on_actionExportCompetition_triggered();
    void on_actionCompetitionProperties_triggered();
	void on_pushButton_autopublish_clicked(bool checked);
    void backupCurrentCompetition();
	
    void on_actionOpenDocumentation_triggered();
	void on_actionCheatSheet_triggered();
	void on_actionDebugStatistics_triggered();
#ifndef Q_OS_WIN
	void on_actionReadFdiskData_triggered();
#endif
	void on_actionPublish_all_triggered();
	void on_actionMaximicePublicWidget_triggered();
	
	void writeBetweenRating();
	
    void on_actionCSV_Upload_Command_triggered();

	void on_actionTimeStamp_Generator_2_triggered();

	void on_actionReset_Startbox_triggered();
	
	void on_actionReset_Goalbox_triggered();
	
public slots:
	void processTimeBaseDatagrams();
	void playSound();
	void playErrorSound();
    void loadNextRuns();
	void sendPC2TB_Packet();
	void sendPC2TB_Packet(QByteArray datagram);
    void on_actionCreateCompetition_triggered();
    void on_actionViewAndEdit_triggered();

signals:
	void showFoundRun(RunData* run);

public:
    DialogGeneralSettings* dialogGeneralSettings;
    DialogCsvUploadCommand* dialogCsvUploadCommand;
    int localTimeOffset_ms; // localTimeOffset_ms =  timeBaseTime - localTime;
        // --> timeBaseTime = localTime + offset
    QTime getTimeBaseTime();
    QFileInfo currentFile;

private:
	Ui::MainWindow *ui;


    DialogNewCompetition* dialogNewCompetition;
	DialogDisplaySettings* dialogDisplaySettings;
	static Competition* myCompetition;
	static MainWindow* myMainWindow;

	DialogTimeStampGenerator* dialogTimeStampGenerator;
	DialogTimeStampGenerator2* dialogTimeStampGenerator2;


	FormTimeStampList* formTimeStampList;

	bool serverConnected;
	AssignmentManager assignmentManager;
    AutoSaveManager autoSaveManager;

    QUdpSocket* udpSocket;
	QByteArray udpInputBuffer;
	BoxStates boxStates;
    BoxStatesView* boxStatesView;
	
    DisplayRenderer* renderer;
	Publisher* publisher;
    PublicWidget* publicWidget;
	
	void setOnlineMode(bool online);
	void sendRadioCommand(uint16_t cmdReceiver, uint8_t command);

    QTimer backupTimer;
	
#ifndef Q_OS_WIN
	Fdisk fdisk;
#endif
	QShortcut* manualTrigger;
	
};

#endif // MAINWINDOW_H
