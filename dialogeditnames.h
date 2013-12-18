#ifndef DIALOGEDITNAMES_H
#define DIALOGEDITNAMES_H

#include <QDialog>
#include <QAbstractButton>

class RunData;

namespace Ui {
    class DialogEditNames;
}

class DialogEditNames : public QDialog
{
    Q_OBJECT

public:
	explicit DialogEditNames(QWidget *parent = 0);
	void setRunData(RunData* run);
    ~DialogEditNames();

signals:
	void namesEdited(RunData* run);

private slots:
	void on_buttonBox_accepted();

	void on_buttonBox_clicked(QAbstractButton *button);
	
private:
    Ui::DialogEditNames *ui;
	RunData* run;
};

#endif // DIALOGEDITNAMES_H
