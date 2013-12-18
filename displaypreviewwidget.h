#ifndef DISPLAYPREVIEWWIDGET_H
#define DISPLAYPREVIEWWIDGET_H

/*
  Description:
  Simulates the design of the flipdot displays with yellow/black round pixels.
 */

#include <QLabel>

class QBitmap;
class QColor;
class QImage;

class DisplayPreviewWidget : public QLabel
{
    Q_OBJECT
public:
    explicit DisplayPreviewWidget(QWidget *parent = 0);
    void setTextColor(QColor &color);
	void setBackColor(QColor &color);
	void setScale(int s);
    // TODO: setDotSize(); // make it changeable
    QSize getDotSize(){return dotSize;}

signals:

public slots:
	void setImage(QImage &bitmap);


private:
    QSize dotSize; // size of the virtial flipdot display in dots
	QColor textColor; // color of the text
	QColor backColor; // color of the background
	QImage bwImage; // data source (each pixel is one flipdot)

	int SCALE;
};

#endif // DISPLAYPREVIEWWIDGET_H
