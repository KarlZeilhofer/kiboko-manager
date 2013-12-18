#ifndef DIALOGTIMESTAMPGENERATOR_H
#define DIALOGTIMESTAMPGENERATOR_H

#include <QDialog>

namespace Ui {
    class DialogTimeStampGenerator;
}

class TimeStamp;

class DialogTimeStampGenerator : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTimeStampGenerator(QWidget *parent = 0);
    ~DialogTimeStampGenerator();

private slots:
	void on_comboBox_source_currentIndexChanged(QString str);

	void on_buttonBox_accepted();

signals:
	void timeStampCreated(TimeStamp* ts);

private:
    Ui::DialogTimeStampGenerator *ui;
};

#endif // DIALOGTIMESTAMPGENERATOR_H
