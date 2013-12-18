#ifndef DIALOGVISIBLECOLUMNS_H
#define DIALOGVISIBLECOLUMNS_H

#include <QDialog>
#include <QList>

namespace Ui {
    class DialogVisibleColumns;
}

class DialogVisibleColumns : public QDialog
{
    Q_OBJECT

public:
    explicit DialogVisibleColumns(QWidget *parent = 0);
    ~DialogVisibleColumns();

	enum ColumnType{CT_None=0, CT_Run=1, CT_Round, CT_Color, CT_BoatNumber, CT_Names,
				CT_FireBrigade, CT_ValuationClass, CT_StartTime, CT_GoalTime,
				CT_RunTime, CT_Dsq, CT_Errors, CT_Published, CT_Notes};

	bool isColumnVisible(ColumnType ct);
	ColumnType columnType(int index); // index starts with 0; returns CT_None if index is out of bounds
	int columnIndex(ColumnType ct);
	QString nameOfType(ColumnType ct);
	QString nameOfIndex(int index);
	int numberOfVisibleColumns(){return visibleColumns.size();}
	QStringList nameList();
	static QStringList getAllColumnNames();

signals:
	void columnsChanged(); // this signal should be used, to regenerate the table

private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();

private:
    Ui::DialogVisibleColumns *ui;

	static QStringList ColumnNames;

	int myNumberOfVisibleColumns;
	QList<ColumnType> visibleColumns;

	bool myRun;
	bool myRound;
	bool myColor;
	bool myBoatNumber;
	bool myNames;
	bool myFireBrigade;
	bool myValuationClass;
	bool myStartTime;
	bool myGoalTime;
	bool myRunTime;
	bool myDsq;
	bool myErrors;
	bool myPublished;
	bool myNotes;
};

#endif // DIALOGVISIBLECOLUMNS_H
