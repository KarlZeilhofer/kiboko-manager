#ifndef DIALOGBINDBOATBOX_H
#define DIALOGBINDBOATBOX_H

#include <QDialog>
#include <QTimer>
#include <QTime>
#include <timestamp.h>
#include <defs.h>


namespace Ui {
    class DialogBindBoatBox;
}

class DialogBindBoatBox : public QDialog
{
    Q_OBJECT

public:
	explicit DialogBindBoatBox(QWidget *parent = 0, int numOfBoatsPerRound = N_BOATBOXES-N_SPARE_BOATBOXES, int numOfReplacementBoats = N_SPARE_BOATBOXES);
    ~DialogBindBoatBox();

	int getSelectedBoatId();
	QString getSelectedBoatName();
	void setFixedBoatID(int boatID); // can be set temporarily, until the dialog is closed (TODO: reset the fixedBoatID on window-close-button)
    void setTimeStamp(TimeStamp* ts); // is used, to show correct duration since timestamp was triggered
    void setNumOfBoatsPerRound(int bpr);
    void setNumOfReplacementBoats(int rb);

private:


public slots:
	void updateBinding(int boatBoxID);
	void refreshAllBindings();
    void updateDialogText();
    void resetTimeStampPointer();

private slots:
    void on_comboBox_boatName_currentIndexChanged(int currentIndex);
	void on_pushButton_later_clicked();
	void on_pushButton_OK_clicked();

private:
    Ui::DialogBindBoatBox *ui;
	int numBPR; // number of boats per round
	int numRB; // number of replacement boats
	int fixedBoatID;
    TimeStamp* timeStamp;
    QTimer timer; // refresh-Timer for timestamptime
};

#endif // DIALOGBINDBOATBOX_H
