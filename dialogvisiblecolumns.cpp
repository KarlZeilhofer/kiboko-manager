#include "dialogvisiblecolumns.h"
#include "ui_dialogvisiblecolumns.h"


QStringList DialogVisibleColumns::ColumnNames =
	QString(tr(",Bew#,DG#,Farbe,Zillen#,Namen,Feuerwehr,BWKL,"
			   "Startzeit,Zielzeit,Laufzeit,DSQ,Fehler,Publ.,Anmerkungen")).split(',');


DialogVisibleColumns::DialogVisibleColumns(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVisibleColumns)
{
    ui->setupUi(this);
	setModal(true);
	// todo: load from settings

	visibleColumns.clear();
	on_buttonBox_accepted();
}

DialogVisibleColumns::~DialogVisibleColumns()
{
    delete ui;
	// todo: save to settings
}

void DialogVisibleColumns::on_buttonBox_accepted()
{
	QList<ColumnType> oldColumns = visibleColumns;
	visibleColumns.clear();
	if((myRun = ui->checkBox_run->isChecked())) visibleColumns.append(CT_Run);
	if((myRound = ui->checkBox_round->isChecked())) visibleColumns.append(CT_Round);
	if((myColor = ui->checkBox_color->isChecked())) visibleColumns.append(CT_Color);
	if((myBoatNumber = ui->checkBox_boatNumber->isChecked())) visibleColumns.append(CT_BoatNumber);
	if((myNames = ui->checkBox_names->isChecked())) visibleColumns.append(CT_Names);
	if((myFireBrigade = ui->checkBox_fireBrigade->isChecked())) visibleColumns.append(CT_FireBrigade);
	if((myValuationClass = ui->checkBox_valuationClass->isChecked())) visibleColumns.append(CT_ValuationClass);
	if((myStartTime = ui->checkBox_startTime->isChecked())) visibleColumns.append(CT_StartTime);
	if((myGoalTime = ui->checkBox_goalTime->isChecked())) visibleColumns.append(CT_GoalTime);
	if((myRunTime = ui->checkBox_runTime->isChecked())) visibleColumns.append(CT_RunTime);
	if((myDsq = ui->checkBox_dsq->isChecked())) visibleColumns.append(CT_Dsq);
	if((myErrors = ui->checkBox_errors->isChecked())) visibleColumns.append(CT_Errors);
	if((myPublished = ui->checkBox_published->isChecked())) visibleColumns.append(CT_Published);
	if((myNotes = ui->checkBox_notes->isChecked())) visibleColumns.append(CT_Notes);

	hide();
	emit accepted();
	if(oldColumns != visibleColumns){
		emit columnsChanged();
	}
}

void DialogVisibleColumns::on_buttonBox_rejected()
{
	ui->checkBox_run->setChecked(myRun);
	ui->checkBox_round->setChecked(myRound);
	ui->checkBox_color->setChecked(myColor);
	ui->checkBox_boatNumber->setChecked(myBoatNumber);
	ui->checkBox_names->setChecked(myNames);
	ui->checkBox_fireBrigade->setChecked(myFireBrigade);
	ui->checkBox_valuationClass->setChecked(myValuationClass);
	ui->checkBox_startTime->setChecked(myStartTime);
	ui->checkBox_goalTime->setChecked(myGoalTime);
	ui->checkBox_runTime->setChecked(myRunTime);
	ui->checkBox_dsq->setChecked(myDsq);
	ui->checkBox_errors->setChecked(myErrors);
	ui->checkBox_published->setChecked(myPublished);
	ui->checkBox_notes->setChecked(myNotes);

	hide();
	emit rejected();
}

bool DialogVisibleColumns::isColumnVisible(ColumnType ct)
{
	for(int n=0; n<visibleColumns.size(); n++){
		if(visibleColumns.at(n) == ct){
			return true;
		}
	}
	return false;
}

// index starts with 0; returns CT_None if index is out of bounds
DialogVisibleColumns::ColumnType DialogVisibleColumns::columnType(int index)
{
	if(index < visibleColumns.size()){
		return visibleColumns.at(index);
	}else{
		return CT_None;
	}
}

// returns -1, if column-type isn't visible
int DialogVisibleColumns::columnIndex(ColumnType ct)
{
	for(int n=0; n<visibleColumns.size(); n++){
		if(visibleColumns.at(n) == ct){
			return n;
		}
	}
	return -1;
}

QString DialogVisibleColumns::nameOfType(ColumnType ct)
{
	return ColumnNames.at(ct);
}

// static function,
// returns all column-names, extept the "none"-string.
QStringList DialogVisibleColumns::getAllColumnNames()
{
	QStringList list = ColumnNames;
	list.removeFirst();
	return list;
}

// if index is out of bounds, return an empty string
QString DialogVisibleColumns::nameOfIndex(int index)
{
	ColumnType ct = columnType(index);
	if(ct>=0){
		return nameOfType(ct);
	}else{
		return QString("");
	}
}

// returns a stringlist with the visible columns
QStringList DialogVisibleColumns::nameList()
{
	QStringList list;
	for(int n=0; n<visibleColumns.size(); n++){
		list.append(nameOfIndex(n));
	}
	return list;
}
