#include "Imageplane.h"
#include "ui_widget.h"
#include "widget.h"
#include <QtGui>
#include <stdlib.h>

Imageplane::Imageplane(QWidget *parent) :
	QWidget(parent)
{
	image=new QImage();
	selecting=false;
	setMouseTracking(true);
}


void Imageplane::paintEvent(QPaintEvent *event)
{
	QPainter p(this);

	// draw a black background
	p.setBrush(Qt::black);
	p.drawRect(0, 0, size().width()-1, size().height()-1);

	// draw the image
	p.drawImage(0, 0, *image);

	if(selecting)
	{
		p.setBrush(Qt::NoBrush);

		QPen pen;

		// draw a green solidline
		pen.setColor(Qt::green);
		p.setPen(pen);
		p.drawRect(selection);

		// draw a red dashline
		pen.setColor(Qt::red);
		pen.setStyle(Qt::DashLine);
		p.setPen(pen);
		p.drawRect(selection);
	}
}


void Imageplane::setColorMap(COLORMAP cm)
{
	colorMap=cm;
}


void Imageplane::makeImage(QSize size, int **matrix)
{
	delete image;
	image=new QImage(size, QImage::Format_RGB32);

	if(image->isNull())
	{
		//printf("Out of memory\n");
		return;
	}

	// get histogram
	//int histogram[maxiter+1];
	//memset(histogram, 0, (maxiter+1)*sizeof(int));
	QVector<int> histogram;
	int numPixels=0;

	for(int y=0; y<image->size().height(); y++)
	{
		for(int x=0; x<image->size().width(); x++)
		{
			int index=matrix[x][y];
			if(index>=0)
			{
				// make shure that there is enough space in the histogram 
				if(index>=histogram.size())
				{
					histogram.resize(index+1);
				}
				histogram[index]++;
				numPixels++;
			}
		}
	}
	
	int maxiter=histogram.size()-1;

	// get minvalue and maxvalue
	int minValue=maxiter, maxValue=0;
	int sum=0;

	for(int i=0; i<=maxiter; i++)
	{
		sum+=histogram.at(i);

		if(histogram.at(i)!=0 && minValue==maxiter)
		{
			minValue=i;
		}

		if(sum<numPixels*0.995)
		{
			maxValue=i;
		}
	}

	qDebug() << "minValue: " << minValue;
	qDebug() << "maxValue: " << maxValue;


	// transform values to colors
	for(int y=0; y<image->size().height(); y++)
	{
		for(int x=0; x<image->size().width(); x++)
		{
			if(matrix[x][y]>=0)
			{

				//float value=histmap[matrix[x][y]];
				float value = (float)(matrix[x][y]-minValue)/(maxValue-minValue);

				if(value>1)
				{
					value=1;
				}
				if(value<0)
				{
					value=0;
				}

				// map all values to a range of 0-255;
				//value=(value-minValue)*767/(maxValue-minValue);

				switch(colorMap)
				{
					case WHITE:
						image->setPixel(x, y, qRgb(255, 255, 255)); break;
					case GREEN:
						image->setPixel(x, y, qRgb(0, value*255, 0)); break;
					case HEAT:
						// 0-1/3: only red
						// 1/3-2/3: add green
						// 2/3-1: add blue
						if(value<=(float)1/3)
						{
							image->setPixel(x, y, qRgb(3*255*value, 0, 0));
						}
						else if(value<=(float)2/3)
						{
							image->setPixel(x, y, qRgb(255, 3*255*value-255, 0));
						}
						else
						{
							image->setPixel(x, y, qRgb(255, 255, 3*255*value-2*255));
						}
						break;
					case RAINBOW:
						image->setPixel(x, y, QColor::fromHsv((int)(value*1000+180)%360, 255, 255).rgb()); break;
					case OCEAN:
						image->setPixel(x, y, QColor::fromHsl(240, 255, (int)(value*255)).rgb()); break;
				}
			}
			else
			{
				// pixels inside the set are black
				image->setPixel(x, y, qRgb(0,0,0));
			}
		}
	}
}


void Imageplane::mouseReleaseEvent(QMouseEvent *event)
{
	//qDebug("mouse click");
	if(!Widget::widget->calc->isRunning())
	{
		if(!selecting)
		{
			qDebug("start of selection");
			// start of selection
			selecting=true;
			center=event->pos();
		}
		else
		{
			qDebug("end of selection");
			// end of selection
			selecting=false;

			// calculate parameters for selected region

			int width=selection.width();				// width of selection (in pixels)

			QSize size=Widget::widget->calc->matrixSize();	// total size of matrix (in pixels)

			INIT_REALNUM(realWidth);
			realWidth=Widget::widget->getRealWidth();
			INIT_REALNUM(imagHeight);
			imagHeight=realWidth/size.width()*size.height();
			
			INIT_REALNUM(x_);
			x_=center.x();
			INIT_REALNUM(y_);
			y_=center.y();

			INIT_REALNUM(realC);
			realC = (x_/size.width() - 1.0/2) * realWidth + Widget::widget->getRealCenter();
			INIT_REALNUM(imagC);
			imagC = -(y_/size.height() - 1.0/2) * imagHeight + Widget::widget->getImagCenter();

			Widget::widget->setRealCenter(realC);
			Widget::widget->setImagCenter(imagC);

			Widget::widget->setRealWidth(width*realWidth/size.width());

			Widget::widget->startCalc();
		}
	}
}

void Imageplane::mouseMoveEvent(QMouseEvent *event)
{
	if(selecting)
	{
		// get difference from center to mouse position
		int dx=abs(event->pos().x()-center.x());
		int dy=abs(event->pos().y()-center.y());

		// create a rectangle proportional to the window
		int propX=dy*size().width()/size().height();	// proportional dx to dy
		int propY=dx*size().height()/size().width();	// proportional dy to dx

		if(propX>dx)
		{
			dx=propX;	// mouse is pulling at bottom (or top) edge of selection
		}
		else
		{
			dy=propY;	// mouse is pulling at the right (or left) edge of selection
		}

		selection=QRect(center.x()-dx, center.y()-dy, 2*dx, 2*dy);

		update();
	}
}


bool Imageplane::save(QString filename)
{
	return image->save(filename, "PNG");
}



void Imageplane::stopSelecting()
{
	selecting=false;
	update();	// remove the selecting rectangle
}



void Imageplane::keyPressEvent(QKeyEvent *event)
{
	switch(event->key())
	{
		case Qt::Key_F11:
			if(isFullScreen())
			{
				showNormal();
				setWindowFlags(Qt::Widget);
				show();
			}
			break;
		default:
			Widget::widget->keyPressEvent(event);
	}
}
