#ifndef FDISK_H
#define FDISK_H

#include <QObject>
#include <QThread>
#include "rundata.h"
#include "FdExport.h"

class Fdisk : public QThread
{
	Q_OBJECT
public:
	explicit Fdisk(QObject *parent = 0);
	
signals:
	
public slots:
	void test(QString databaseName);
	
private:
	void run();
	
	ArrayOfArrayOfFdWsWasserdienstleistungsbewerbeRequestFilter createFilter(std::string FdiskBewerbsID);
	QString strConv(std::string str);
};

#endif // FDISK_H
