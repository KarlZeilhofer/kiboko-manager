#include "fdisk.h"

#include <memory>
#include <iostream>
#include <staff/utils/Log.h>
#include <staff/common/logoperators.h>
#include <staff/common/Exception.h>
#include <staff/client/ServiceFactory.h>

#include <QDebug>
#include <QDir>

#include "competition.h"
#include "mainwindow.h"
#include "infoscreen.h"

Fdisk::Fdisk(QObject *parent) :
	QThread(parent)
{
}

void Fdisk::run()
{
	try
	{
		Competition* comp=MainWindow::competition();
		
		if(comp->getDatabaseName().isEmpty())
		{
			return;
		}
		
		MainWindow::app()->infoscreen()->appendInfo("FDISK-Daten werden gelesen...");
		
		std::auto_ptr< ::FdExport > pFdExport(::staff::ServiceFactory::Inst().GetService< ::FdExport >());
		
		STAFF_ASSERT(pFdExport.get(), "Cannot get client for service FdExport!");
		
		//std::string bewerbsID("912"); // ID aus orignal Beispiel von ACP
		//std::string bewerbsID("1641"); // Testbewerb, den Zani angelegt hat.	
		//std::string bewerbsID("1503"); // BewerbsID des Amstettner Bewerbs in Zell 2013
		//std::string bewerbsID("1574"); // BewerbsID WLA-Gold, Marchegg (funktioniert nicht)
		//std::string bewerbsID("1609"); // BewerbsID des Kremser Bewerbs in Hollenburg 2013
		std::string bewerbsID=comp->getDatabaseName().toStdString();
		
		
		staff::Optional<std::string> user =  staff::Optional<std::string>("BewerbsexportZeilhofer");
		staff::Optional<std::string> password = staff::Optional<std::string>("Ubu6Son6");
		staff::Optional<std::string> instanz = staff::Optional<std::string>("LFKDO Niederösterreich");
		const staff::Optional< ::FdWsAuthorizeReturn >& AuthorizeResponse = pFdExport->Authorize(user, password, instanz);
		//qDebug() << "authorize: " << strConv(AuthorizeResponse->statusMessage);
		
		staff::Optional<std::string> md5session = AuthorizeResponse->md5session; // save the session key!
		
		staff::Optional< ::FdWsPrepareDataRequest > xmlRequest;
		xmlRequest->type = FdWsExportType_Wasserdienstleistungsbewerbe;
		FdWsWasserdienstleistungsbewerbeRequest	wdlbReq;
		wdlbReq.filterGroup = createFilter(bewerbsID);
		xmlRequest->wasserdienstleistungsbewerbe = wdlbReq;
		
		//qDebug() << "Prepare Data on Server for BewerbsID = " << strConv(bewerbsID) << "...";
		
		const staff::Optional< ::FdWsPrepareDataReturn >& PrepareDataResponse = pFdExport->PrepareData(md5session, xmlRequest);
		//qDebug() << "PrepareData result: " << strConv(PrepareDataResponse->statusMessage);
		
		
		//qDebug() << "fetching data";
		std::list< staff::Nillable< ::FdWsWasserdienstleistungsbewerbeStruct > > list;
		
		while(true)
		{
			const staff::Optional< ::FdWsGetNextDataBlockReturn >& getNextDataBlockResponse =
					pFdExport->getNextDataBlock(md5session);
			
			//staff::LogInfo() << "getNextDataBlock result: " << getNextDataBlockResponse->statusMessage;
			
			std::list< staff::Nillable< ::FdWsWasserdienstleistungsbewerbeStruct > > blockList =
					getNextDataBlockResponse->wasserdienstleistungsbewerbe->FdWsWasserdienstleistungsbewerbeStruct;
			
			// append blockList to list
			list.insert(list.end(), blockList.begin(), blockList.end());
			
			//std::cout << "blockList.size() = " << blockList.size() << "\n";
			
			if(getNextDataBlockResponse->statusCode != StatusCodes_OK)
			{
				break;
			}
			//qDebug() << ".";
		}
		//qDebug() << "done\n";
		
		
		while(list.size())
		{
			staff::Nillable< ::FdWsWasserdienstleistungsbewerbeStruct > itemHelper = list.front();
			list.pop_front();
			FdWsWasserdienstleistungsbewerbeStruct item = itemHelper.Get();
			
			int round=strConv(item.Durchgang.Get()).toInt();
			int boatID=strConv(item.Zillennummer.Get()).toInt();
			
			int ID=(round-1)*comp->getBoatsPerRound()+boatID;
			
			if(ID>comp->getNumOfRuns())
			{
				break;
			}
			else
			{
				RunData* run=comp->getRun(ID);
				
				run->startEditing();
				
				run->setColor(strConv(item.Farbe.Get()));
				run->setFireBrigade(strConv(item.Instanzname.Get()));
				run->setValuationClass(strConv(item.Wertungsklasse.Get()));
				run->setAgePoints(strConv(item.AlterspunkteWert.Get()));
				
				QStringList name1=strConv(item.Name1.Get()).split(" ", QString::SkipEmptyParts);
				QStringList name2=strConv(item.Name2.Get()).split(" ", QString::SkipEmptyParts);
				
				//qDebug()<<round<<" "<<boatID<<" "<<name1<<" "<<name2<<"\n";
				
				if(!name1.isEmpty())
				{
					// the last substring is the lastname
					run->setLastName1(name1.takeLast());
					if(!name1.isEmpty())
					{
						// the substring next to last is the firstname
						run->setFirstName1(name1.takeLast());
						if(!name1.isEmpty())
						{
							// the rest of the string (at the front) is the title
							run->setTitle1(name1.join(" "));
						}
					}
				}
				
				if(!name2.isEmpty())
				{
					// the last substring is the lastname
					run->setLastName2(name2.takeLast());
					if(!name2.isEmpty())
					{
						// the substring next to last is the firstname
						run->setFirstName2(name2.takeLast());
						if(!name2.isEmpty())
						{
							// the rest of the string (at the front) is the title
							run->setTitle2(name2.join(" "));
						}
					}
				}
				run->editingFinished();
			}
		}
		
		MainWindow::app()->infoscreen()->appendInfo(tr("FDISK-Daten wurden erfolgreich gelesen"));
		
		
		pFdExport->closeSession(md5session);
		//qDebug() << "session closed";
	}
	catch(...)
	{
		MainWindow::app()->infoscreen()->appendError(tr("FDISK-Daten konnten nicht gelesen werden"));
	}

	//STAFF_CATCH_ALL
}

void Fdisk::test(QString databaseName)
{
	try
	{
		//qDebug() << "test fdisk connection";
		std::auto_ptr< ::FdExport > pFdExport(::staff::ServiceFactory::Inst().GetService< ::FdExport >());
		
		STAFF_ASSERT(pFdExport.get(), "Cannot get client for service FdExport!");
		
		std::string bewerbsID=databaseName.toStdString();
		
		
		staff::Optional<std::string> user =  staff::Optional<std::string>("BewerbsexportZeilhofer");
		staff::Optional<std::string> password = staff::Optional<std::string>("Ubu6Son6");
		staff::Optional<std::string> instanz = staff::Optional<std::string>("LFKDO Niederösterreich");
		const staff::Optional< ::FdWsAuthorizeReturn >& AuthorizeResponse = pFdExport->Authorize(user, password, instanz);
		//qDebug() << "authorize: " << strConv(AuthorizeResponse->statusMessage);
		
		staff::Optional<std::string> md5session = AuthorizeResponse->md5session; // save the session key!
		
		staff::Optional< ::FdWsPrepareDataRequest > xmlRequest;
		xmlRequest->type = FdWsExportType_Wasserdienstleistungsbewerbe;
		FdWsWasserdienstleistungsbewerbeRequest	wdlbReq;
		wdlbReq.filterGroup = createFilter(bewerbsID);
		xmlRequest->wasserdienstleistungsbewerbe = wdlbReq;
		
		//qDebug() << "Prepare Data on Server for BewerbsID = " << strConv(bewerbsID) << "...";
		
		const staff::Optional< ::FdWsPrepareDataReturn >& PrepareDataResponse = pFdExport->PrepareData(md5session, xmlRequest);
		//qDebug() << "PrepareData result: " << strConv(PrepareDataResponse->statusMessage);
		
		pFdExport->closeSession(md5session);
		//qDebug() << "session closed";
		
		if(PrepareDataResponse->statusCode==StatusCodes_OK)
		{
			MainWindow::app()->infoscreen()->appendInfo(tr("FDISK-Verbindung erfolgreich getestet"));
		}
		else
		{
			MainWindow::app()->infoscreen()->appendError(tr("FDISK-Verbindung konnte nicht hergestellt werden"));
		}
	}
	catch(...)
	{
		MainWindow::app()->infoscreen()->appendError(tr("FDISK-Verbindung konnte nicht hergestellt werden"));
	}
}

ArrayOfArrayOfFdWsWasserdienstleistungsbewerbeRequestFilter Fdisk::createFilter(std::string FdiskBewerbsID)
{
	ArrayOfArrayOfFdWsWasserdienstleistungsbewerbeRequestFilter filterGroup; // filter-lists in groups are OR-combined
	ArrayOfFdWsWasserdienstleistungsbewerbeRequestFilter filterList; // filter in lists are AND-combined
	FdWsWasserdienstleistungsbewerbeRequestFilter filter;
	
	filter.type = FdWsWasserdienstleistungsbewerbeExpFilter_FdiskIdBewerbe;
	filter.value = FdiskBewerbsID;
	
	filterList.FdWsWasserdienstleistungsbewerbeRequestFilter.push_front(filter);
	filterGroup.ArrayOfFdWsWasserdienstleistungsbewerbeRequestFilter.push_front(filterList);
	
	return filterGroup;
}

QString Fdisk::strConv(std::string str)
{
	return QString::fromUtf8(str.c_str());
}


