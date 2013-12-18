#include "dialoggeneralsettings.h"
#include "ui_dialoggeneralsettings.h"
#include <QSettings>
#include <QDebug>



DialogGeneralSettings::DialogGeneralSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGeneralSettings)
{
    QSettings set;
    set.beginGroup(SS_GS_PREFIX);

    ui->setupUi(this);
	setModal(true);

    qDebug() << "Settings filename: " << set.fileName();

    bool ok;
    int v;
    v = set.value(SS_GS_PUBLISH_INTERVAL, 15).toInt(&ok);
    if(!ok){
        qDebug() << "Error on loading Setting: " << SS_GS_PUBLISH_INTERVAL << ". Using Default Value 15";
    }else{
        qDebug() << "Loaded Setting: " << SS_GS_PUBLISH_INTERVAL << ": " << v;
    }
    ui->spinBox_publishInterval->setValue(v);

    v = set.value(SS_GS_START_INTERVAL, 60).toInt(&ok);
    if(!ok){
        qDebug() << "Error on loading Setting: " << SS_GS_START_INTERVAL << ". Using Default Value 60";
    }else{
        qDebug() << "Loaded Setting: " << SS_GS_START_INTERVAL << ": " << v;
    }
    ui->spinBox_startInterval->setValue(v);


    ui->checkBox_soundOnTrigger->setChecked(set.value(SS_GS_SIGNAL_ON_TRIGGER, true).toBool());
    ui->checkBox_soundOnStart->setChecked(set.value(SS_GS_SIGNAL_ON_START, true).toBool());

    ui->radioButton_assignOnlyForward->setChecked(set.value(SS_GS_ASSIGN_ONLY_FORWARD, true).toBool()); // lower austria
    ui->radioButton_assignToCurrentRound->setChecked(set.value(SS_GS_ASSIGN_TO_CURRENT_ROUND, false).toBool()); // upper austria
}

DialogGeneralSettings::~DialogGeneralSettings()
{
    QSettings set;
    set.beginGroup(SS_GS_PREFIX);

    set.setValue(SS_GS_PUBLISH_INTERVAL, ui->spinBox_publishInterval->value());
    set.setValue(SS_GS_START_INTERVAL, ui->spinBox_startInterval->value());
    set.setValue(SS_GS_SIGNAL_ON_TRIGGER, ui->checkBox_soundOnTrigger->isChecked());
    set.setValue(SS_GS_SIGNAL_ON_START, ui->checkBox_soundOnStart->isChecked());
    set.setValue(SS_GS_ASSIGN_ONLY_FORWARD, ui->radioButton_assignOnlyForward->isChecked());
    set.setValue(SS_GS_ASSIGN_TO_CURRENT_ROUND, ui->radioButton_assignToCurrentRound->isChecked());
    qDebug()<< "wrote Settings " << SS_GS_PREFIX;

    delete ui;
}

int DialogGeneralSettings::publishInterval()
{
    return ui->spinBox_publishInterval->value();
}


int DialogGeneralSettings::startInterval()
{
    return ui->spinBox_startInterval->value();
}


void DialogGeneralSettings::on_radioButton_assignOnlyForward_toggled(bool checked)
{
    QSettings set;
    set.beginGroup(SS_GS_PREFIX);
    set.setValue(SS_GS_ASSIGN_ONLY_FORWARD, ui->radioButton_assignOnlyForward->isChecked());
}

void DialogGeneralSettings::on_radioButton_assignToCurrentRound_toggled(bool checked)
{
    QSettings set;
    set.beginGroup(SS_GS_PREFIX);
    set.setValue(SS_GS_ASSIGN_TO_CURRENT_ROUND, ui->radioButton_assignToCurrentRound->isChecked());

}
