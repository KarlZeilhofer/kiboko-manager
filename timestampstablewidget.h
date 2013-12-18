#ifndef TIMESTAMPSTABLEWIDGET_H
#define TIMESTAMPSTABLEWIDGET_H

#include <QTableWidget>
#include <QList>
#include <QTimer>

class TimeStamp;

class TimeStampsTableWidget : public QTableWidget
{
    Q_OBJECT
public:
	explicit TimeStampsTableWidget(QWidget *parent = 0);
	void init();
	int getSelectedID(); // returns 0, if none selected

private:
	void moveRowDown(int row);
	void moveRowUp(int row);
	void printHashMaps();


public slots:
	void updateTimeStamp(TimeStamp* ts);
	void regenerateList();
	void on_customContextMenuRequested( const QPoint & pos );
	void selectTimeStamp(TimeStamp* ts);
	void setOnlineMode(bool online);

private slots:
	void sendSelectSignal(int row, int col, int lastRow, int lastCol);
	void updateAgeTimes();
	void on_cellDoubleClicked(int row, int col);

signals:
	void selectionChanged(TimeStamp* ts);
	void assign();		// is emitted when the user double clicks on a timestamp

private:
	QHash<int,int> idToRow; // stores the row-number for each timeStampID
	QHash<int,int> rowToID; // stores the timeStampIDs for each row
	
	QTimer updateTimer;
	bool onlineMode;
};

#endif // TIMESTAMPSTABLEWIDGET_H
