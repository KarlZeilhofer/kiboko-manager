#ifndef DISPLAYRENDERER_H
#define DISPLAYRENDERER_H

#include <QObject>
#include "displaypreviewwidget.h"
#include "dialogdisplaysettings.h"
#include "rundata.h"
#include <stdint.h>

/*
  generates out of a given run-id, and the display-settings a QImage,
  which can be painted with the DisplayPreviewWidget, and also transfered to the
  new sports-displays aswell.
  */

class DisplayRenderer : public QObject
{
    Q_OBJECT
public:
    explicit DisplayRenderer(DisplayPreviewWidget* widget1, DisplayPreviewWidget* widget2,
                             DialogDisplaySettings* settings, QObject *parent = 0);


private:
    QString buildString(QString format);
	void sendDisplayData(QImage im1, QImage im2);
	void genMatrixData(QImage im, int displayNumber, uint8_t *data);
	QString algeText();


signals:
	void sendDatagram(QByteArray datagram);
	
public slots:
    void render(RunData *runData, bool realDisplayData=true);

private:
    DisplayPreviewWidget* widget1;
    DisplayPreviewWidget* widget2;
    DialogDisplaySettings* settingsDialog;
    RunData* runData;
};

#endif // DISPLAYRENDERER_H
