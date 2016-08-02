#ifndef INFOSCREEN_H
#define INFOSCREEN_H

#include <QTextEdit>

class InfoScreen : public QTextEdit
{
	Q_OBJECT
public:
	explicit InfoScreen(QWidget *parent = 0);
	
signals:
	
public slots:
	void appendInfo(QString text);
	void appendWarning(QString text);
	void appendError(QString text);
	void writeLogFile();
	
private:
	
};

#endif // INFOSCREEN_H
