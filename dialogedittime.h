#ifndef DIALOGEDITTIME_H
#define DIALOGEDITTIME_H

#include <QDialog>

class RunData;

namespace Ui {
    class DialogEditTime;
}

class DialogEditTime : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditTime(QWidget *parent = 0);
    ~DialogEditTime();

	enum TimeType {TT_START, TT_GOAL};

	void setRunData(RunData* run, TimeType timeType);

signals:
	void timeEdited(RunData* run);


private slots:
	void on_buttonBox_accepted();

	void on_buttonBox_rejected();

private:
    Ui::DialogEditTime *ui;
	RunData* myRun;
	TimeType myTimeType;
};

#endif // DIALOGEDITTIME_H
