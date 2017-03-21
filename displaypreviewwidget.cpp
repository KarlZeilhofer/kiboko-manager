#include "displaypreviewwidget.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QDebug>
#include "packet.h"

DisplayPreviewWidget::DisplayPreviewWidget(QWidget *parent) :
	QLabel(parent)
{
    this->dotSize = QSize(MATRIX_MODULES*MATRIX_COLS_PER_MOD, MATRIX_ROWS);

	QImage im("anzeige1.bmp");
	QBitmap bm = QBitmap::fromImage(im);
	QPixmap pm = QPixmap::fromImage(im);

	SCALE = 4; // default scale

	textColor = QColor::fromRgb(0xd0,0xf3,0x05);
	backColor = QColor::fromRgb(0x20,0x20,0x20);

	setImage(im);

    show();
}

// imitates the flipdots
// images are croped to the size of the dotSize
void DisplayPreviewWidget::setImage(QImage &image)
{
    bwImage = image.copy(0,0,dotSize.width(), dotSize.height()); // crop the given image
	QImage im;
	QImage bm; // bitmap (source of data)


	if(SCALE < 3){
        im = QImage(SCALE*bwImage.size().width(), SCALE*bwImage.size().height(), QImage::Format_RGB32); // image (drawing area)
        bm = QImage(bwImage); // bitmap (source of data)
		im.fill(backColor.rgba());

        QPainter p(&im);
		//p.setRenderHint(QPainter::Antialiasing);
		p.setPen(textColor);



		for(int x=0; x<bm.size().width(); x++){
			for(int y=0; y<bm.size().height(); y++){
                quint32 col = bm.pixel(x,y)&0xffffff;
                if( ( ((col>>16)&0xff) + ((col>>8)&0xff) + ((col>>0)&0xff) )>= 128){ // convert to grayscale
					p.setBrush(QBrush(textColor));
					p.fillRect(x*(SCALE), y*(SCALE), SCALE, SCALE, p.brush());
				}
			}
		}
	}else{
        im = QImage(SCALE*bwImage.size().width()+1, SCALE*bwImage.size().height()+1, QImage::Format_RGB32); // image (drawing area)
        bm = QImage(bwImage); // bitmap (source of data)
		im.fill(Qt::black);

        QPainter p(&im);
		p.setRenderHint(QPainter::Antialiasing);



		for(int x=0; x<bm.size().width(); x++){
			for(int y=0; y<bm.size().height(); y++){
				quint32 col = bm.pixel(x,y)&0xffffff;
                if( ( ((col>>16)&0xff) + ((col>>8)&0xff) + ((col>>0)&0xff) )>= 128){ // convert to grayscale
					p.setBrush(QBrush(textColor));
					p.setPen(textColor);
					p.drawEllipse(x*(SCALE)+1, y*(SCALE)+1, SCALE-2, SCALE-2);
				}else{
					p.setBrush(QBrush(backColor));
					p.setPen(backColor);
					p.drawEllipse(x*(SCALE)+1, y*(SCALE)+1, SCALE-2, SCALE-2);
				}
			}
		}
	}

	setPixmap(QPixmap::fromImage(im));
	setMinimumSize(im.size());

	update();
}

void DisplayPreviewWidget::setScale(int s)
{
	SCALE = s;
	setImage(bwImage); // repaint the image
}
