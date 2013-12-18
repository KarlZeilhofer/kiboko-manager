#include "infoscreen.h"
#include "mainwindow.h"
#include "QDebug"

#include <QScrollBar>

InfoScreen::InfoScreen(QWidget *parent) :
	QListWidget(parent)
{
	setSelectionMode(QAbstractItemView::NoSelection);
	verticalScrollBar()->setSingleStep(1);
}

void InfoScreen::appendInfo(QString text)
{
	QTime time=MainWindow::app()->getTimeBaseTime();
	QListWidgetItem* item = new QListWidgetItem(QString(time.toString("hh:mm:ss")+" Info: "+text));
	item->setForeground(QBrush(QColor(20, 200, 20)));
	addItem(item);
	setCurrentItem(item);
	//qDebug() << "Info:" << text;
}

void InfoScreen::appendWarning(QString text)
{
	QTime time=MainWindow::app()->getTimeBaseTime();
	QListWidgetItem* item = new QListWidgetItem(QString(time.toString("hh:mm:ss")+" Warnung: "+text));
	item->setForeground(QBrush(QColor(255, 132, 0)));
	addItem(item);
	setCurrentItem(item);
	//qDebug() << "Warning:" << text;
}

void InfoScreen::appendError(QString text)
{
	QTime time=MainWindow::app()->getTimeBaseTime();
	QListWidgetItem* item = new QListWidgetItem(QString(time.toString("hh:mm:ss")+" Fehler: "+text));
	item->setForeground(QBrush(QColor(255, 0, 0)));
	addItem(item);
	setCurrentItem(item);
	
	// play error sound
	MainWindow::app()->playErrorSound();
	//qDebug() << "Error:" << text;
}
