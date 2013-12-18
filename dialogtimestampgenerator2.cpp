#include "dialogtimestampgenerator2.h"
#include "ui_dialogtimestampgenerator2.h"

#include "crc8.h"
#include <QtNetwork>

#define UDP_LISTEN_PORT (10002) // from XPORT to PC

DialogTimeStampGenerator2::DialogTimeStampGenerator2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTimeStampGenerator2)
{
    ui->setupUi(this);

	ui->comboBox_triggerStationID->addItem("Start-Box", 1);
	ui->comboBox_triggerStationID->addItem("Ziel-Box", 2);

	for(int n=1; n<=20; n++)
	{
		ui->comboBox_boatBoxID->addItem(QString("BB%1").arg(n), n);
	}

	ui->comboBox_stationTriggeredAt->addItem("Start", 1);
	ui->comboBox_stationTriggeredAt->addItem("Ziel Links", 2);
	ui->comboBox_stationTriggeredAt->addItem("Ziel Rechts", 3);
}

DialogTimeStampGenerator2::~DialogTimeStampGenerator2()
{
    delete ui;
}

void DialogTimeStampGenerator2::on_DialogTimeStampGenerator2_accepted()
{


}

void DialogTimeStampGenerator2::on_pushButton_clicked()
{
	static TB2PC_PACKET p;
	static ALIVE_PACKET ap;

	p.magicNumber1 = MAGIC_NUMBER1;
	p.magicNumber2 = MAGIC_NUMBER2;

	p.packetVersion = ui->lineEdit_packetVersion->text().toInt();
	p.batteryVoltageTB = ui->lineEdit_batteryVoltageTB->text().toInt();
	p.triggerStationID = ui->comboBox_triggerStationID->itemData(ui->comboBox_triggerStationID->currentIndex()).toInt();
	p.binRssiTS = ui->lineEdit_binRssiTS->text().toInt();

	QTime t0(0,0,0);
	ap.triggerTimeBB = t0.msecsTo(ui->timeEdit_triggerTimeBB->time())*10;
	ap.triggerTimeTS_L = t0.msecsTo(ui->timeEdit_triggerTimeTS_L->time())*10;
	ap.triggerTimeTS_R = t0.msecsTo(ui->timeEdit_triggerTimeTS_R->time())*10;
	ap.batteryVoltageTS = ui->lineEdit_batteryVoltageTS->text().toInt();
	ap.batteryVoltageBB = ui->lineEdit_batteryVoltageBB->text().toInt();
	ap.boatBoxID = ui->comboBox_boatBoxID->itemData(ui->comboBox_boatBoxID->currentIndex()).toInt();
	ap.binRssiBB = ui->lineEdit_binRssiBB->text().toInt();
	ap.stationTriggeredAt = ui->comboBox_stationTriggeredAt->itemData(ui->comboBox_stationTriggeredAt->currentIndex()).toInt();

	p.tsPacket = ap;

	p.baseTime_100us = t0.msecsTo(ui->timeEdit_baseTime->time())*10;

	p.crc = CRC8_run((uint8_t*)(&p), sizeof(p)-1);

	//printPacket(p);

	QByteArray datagram;
	datagram.setRawData((char*)(&p), sizeof(p));

	QUdpSocket* udpSocket = new QUdpSocket(this);
	udpSocket->writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, UDP_LISTEN_PORT);
}

void DialogTimeStampGenerator2::printPacket(TB2PC_PACKET p)
{
	qDebug() << "MAGIC_NUMBER1" << p.magicNumber1;
	qDebug() << "MAGIC_NUMBER2" << p.magicNumber2;
	qDebug() << "packetVersion" << p.packetVersion;
	qDebug() << "batteryVoltageTB" << p.batteryVoltageTB;
	qDebug() << "triggerStationID" << p.triggerStationID;
	qDebug() << "binRssiTS" << p.binRssiTS;

	qDebug() << "tsPacket.triggerTimeBB" << p.tsPacket.triggerTimeBB;
	qDebug() << "tsPacket.triggerTimeTS_L" << p.tsPacket.triggerTimeTS_L;
	qDebug() << "tsPacket.triggerTimeTS_R" << p.tsPacket.triggerTimeTS_R;
	qDebug() << "tsPacket.batteryVoltageTS" << p.tsPacket.batteryVoltageTS;
	qDebug() << "tsPacket.batteryVoltageBB" << p.tsPacket.batteryVoltageBB;
	qDebug() << "tsPacket.boatBoxID" << p.tsPacket.boatBoxID;
	qDebug() << "tsPacket.binRssiBB" << p.tsPacket.binRssiBB;
	qDebug() << "tsPacket.stationTriggeredAt" << p.tsPacket.stationTriggeredAt;

	qDebug() << "baseTime_100us" << p.baseTime_100us;
	qDebug() << "crc" << p.crc;
}
