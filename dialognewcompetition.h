#ifndef DIALOGNEWCOMPETITION_H
#define DIALOGNEWCOMPETITION_H

#include <QDialog>
#include <QTimer>

namespace Ui {
    class DialogNewCompetition;
}

class DialogNewCompetition : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewCompetition(QWidget *parent = 0);
    ~DialogNewCompetition();
	QString description();
    int boatsPerRound();
    int startRunID();
    QStringList colors(); // returns empty list, when colors are from server
	QString databaseName();


private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void showInfos();


    void on_spinBox_startAtRun_valueChanged(int runID);
	
	void on_pushButton_testDatabase_clicked();
	
signals:

private:
    Ui::DialogNewCompetition *ui;

	QString myDescription;
	int myBoatsPerRound;
    int myStartRunID;
	QStringList myColors; // returns empty list, when colors are from server
	QString myDatabaseName;

	QTimer* infoTimer; // this timer is used, to show incorrect inputs to the user.
};

#endif // DIALOGNEWCOMPETITION_H
