#ifndef DIALOGGENERALSETTINGS_H
#define DIALOGGENERALSETTINGS_H

#include <QDialog>

// defines for the names of the SettingS:
#define SS_GS_PREFIX               "GeneralSettings"
#define SS_GS_PUBLISH_INTERVAL     "PublishInterval"
#define SS_GS_START_INTERVAL       "CountdownInterval"
#define SS_GS_SIGNAL_ON_TRIGGER    "SignalOnTrigger"
#define SS_GS_SIGNAL_ON_START      "SignalOnStart"
#define SS_GS_ASSIGN_ONLY_FORWARD      "AssignOnlyForward"
#define SS_GS_ASSIGN_TO_CURRENT_ROUND  "AssignToCurrentRound"

namespace Ui {
    class DialogGeneralSettings;
}

class DialogGeneralSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogGeneralSettings(QWidget *parent = 0);
    ~DialogGeneralSettings();
	int publishInterval();
    int startInterval();


private:


private slots:
    //void on_lineEdit_publishInterval_editingFinished();
	
    void on_radioButton_assignOnlyForward_toggled(bool checked);

    void on_radioButton_assignToCurrentRound_toggled(bool checked);

private:
    Ui::DialogGeneralSettings *ui;
	int myPublishInterval;
};

#endif // DIALOGGENERALSETTINGS_H
