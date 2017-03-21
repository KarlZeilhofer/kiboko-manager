#ifndef DIALOGTIMESTAMPGENERATOR2_H
#define DIALOGTIMESTAMPGENERATOR2_H

#include <QDialog>
#include "packet.h"

namespace Ui {
    class DialogTimeStampGenerator2;
}


class DialogTimeStampGenerator2 : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTimeStampGenerator2(QWidget *parent = 0);
    ~DialogTimeStampGenerator2();

private:
	void printPacket(TB2PC_PACKET p);

private slots:
	void on_DialogTimeStampGenerator2_accepted();

	void on_pushButton_clicked();

private:
    Ui::DialogTimeStampGenerator2 *ui;
};

#endif // DIALOGTIMESTAMPGENERATOR2_H
