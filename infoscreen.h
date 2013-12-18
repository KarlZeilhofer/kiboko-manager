#ifndef INFOSCREEN_H
#define INFOSCREEN_H

#include <QListWidget>

class InfoScreen : public QListWidget
{
	Q_OBJECT
public:
	explicit InfoScreen(QWidget *parent = 0);
	
signals:
	
public slots:
	void appendInfo(QString text);
	void appendWarning(QString text);
	void appendError(QString text);
	
private:
	
};

#endif // INFOSCREEN_H
