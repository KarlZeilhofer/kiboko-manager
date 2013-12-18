#ifndef FORMTIMESTAMPLIST_H
#define FORMTIMESTAMPLIST_H

#include <QWidget>

// widget for debugging, listing all time-stamps in the competition.

namespace Ui {
    class FormTimeStampList;
}

class TimeStamp;

class FormTimeStampList : public QWidget
{
    Q_OBJECT

public:
    explicit FormTimeStampList(QWidget *parent = 0);
    ~FormTimeStampList();
	void regenerateTable();

public slots:
	void updateTableRow(TimeStamp* ts);

private slots:
	void on_pushButton_refresh_clicked();

private:
    Ui::FormTimeStampList *ui;
};

#endif // FORMTIMESTAMPLIST_H
