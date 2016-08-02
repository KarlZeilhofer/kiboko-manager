#ifndef MAINTABLEWIDGET_H
#define MAINTABLEWIDGET_H

#include <QTableWidget>
#include <QTimer>
#include "competition.h"
#include "dialogvisiblecolumns.h"
#include "dialogeditnames.h"
#include "dialogedittime.h"

class MainTableWidget : public QTableWidget
{
    Q_OBJECT
public:
	explicit MainTableWidget(QWidget *parent = 0);
    ~MainTableWidget();


private:
	void resizeColumnsToContents(); // override function
    void resizeAllRows();
    QTimer updateTimer;
    bool onlineMode;

private slots:
	void on_cellDoubleClicked(int row, int col);
	void processCellChanged(int row, int col);
	void sendSelectSignal(int row, int col, int lastRow, int lastCol); // for current cell
	void updateRunTimes();

signals:
    void runSelectionChanged(RunData* run); // selects the run in the TimeLine.
    void selectTimeStamp(TimeStamp* ts); // triggered via context menu --> only selects the TS in the TimeLine, but doesn't necessary show it.
    void showTimeStamp(TimeStamp* ts); // triggered via context menu --> scrolls the TimeLine to the timestamp

public slots:
	void updateTableRow(RunData* run);
	void regenerateTable();
	void selectRun(TimeStamp* ts); // selects the line in the table, which contains the TS.
	void selectRun(RunData* run);  // selects the line in the table, which contains the run.
	void showColumnsSettings();
	void setOnlineMode(bool online);

public:
	DialogVisibleColumns dialogVisibleColumns;
	DialogEditNames dialogEditNames;
    DialogEditTime dialogEditTime;
};

#endif // MAINTABLEWIDGET_H
