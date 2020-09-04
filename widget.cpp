#include "widget.h"
#include "ui_widget.h"
#include "Calculator.h"
#include "Imageplane.h"

#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
#include <iomanip>



Widget *Widget::widget=NULL;

Widget::Widget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Widget)
{
	ui->setupUi(this);
	Widget::widget=this;
	setWindowIcon(QIcon("logo.xpm"));
	

	setRealCenter(-0.5);
	setImagCenter(0);
	setRealWidth(4);

	calc=new Calculator(ui->imageplane->size(), realCenter, imagCenter, realWidth);
	ui->imageplane->setColorMap(Imageplane::HEAT);
	//startCalc();
	
	connect(ui->pushButton_save, SIGNAL(clicked()), ui->actionSave, SIGNAL(triggered()));
	connect(ui->pushButton_load, SIGNAL(clicked()), ui->actionOpen, SIGNAL(triggered()));
	connect(ui->pushButton_export, SIGNAL(clicked()), ui->actionExport, SIGNAL(triggered()));
}

Widget::~Widget()
{
	delete calc;
	delete ui;
}


REALNUM Widget::getRealCenter()
{
	return realCenter;
}

REALNUM Widget::getImagCenter()
{
	return imagCenter;
}

REALNUM Widget::getRealWidth()
{
	return realWidth;
}

void Widget::setRealCenter(REALNUM rc)
{
	realCenter=rc;
}

void Widget::setImagCenter(REALNUM ic)
{
	imagCenter=ic;
}

void Widget::setRealWidth(REALNUM rw)
{
	realWidth=rw;
	//ui->lineEdit_scale->setText(QString::number(realWidth.get_d(), 'e', 3));
	ui->lineEdit_scale->setText(QString::number(realWidth, 'e', 3));
}


void Widget::startCalc()
{
	if(calc->isRunning())
	{
		//calc->terminate();
		calc->stop();
		ui->pushButton_run->setText("Run");
		ui->pushButton_stepBack->setEnabled(true);
		ui->pushButton_restart->setEnabled(true);
	}
	else
	{
		delete calc;
		calc=new Calculator(ui->imageplane->size(), realCenter, imagCenter, realWidth);
		connect(calc, SIGNAL(finished()), this, SLOT(calcFinished()));
		connect(calc, SIGNAL(progressChanged(int)), this, SLOT(updateProgressBar(int)));
		calc->start();
		ui->pushButton_run->setText("Abort");
		ui->pushButton_stepBack->setEnabled(false);
		ui->pushButton_restart->setEnabled(false);
	}
}

void Widget::stepBack()
{
	setRealWidth(realWidth*3);
	startCalc();
}




void Widget::updateProgressBar(int percent)
{
	ui->progressBar->setValue(percent);
}


void Widget::calcFinished()
{
	ui->pushButton_run->setText("Run");
	ui->pushButton_stepBack->setEnabled(true);
	ui->pushButton_restart->setEnabled(true);
	ui->imageplane->makeImage(calc->matrixSize(), calc->matrix);
	ui->imageplane->update();
}


void Widget::on_pushButton_run_clicked()
{
	startCalc();
}


void Widget::on_pushButton_stepBack_clicked()
{
	stepBack();
}


void Widget::on_pushButton_restart_clicked()
{
	setRealCenter(-0.5);
	setImagCenter(0);
	setRealWidth(4);
	
	startCalc();
}


void Widget::on_comboBox_Color_currentIndexChanged(const QString &arg1)
{
	//qDebug("colorMap changed");
	if(arg1=="White")
	{
		ui->imageplane->setColorMap(Imageplane::WHITE);
	}
	else if(arg1=="Heat")
	{
		ui->imageplane->setColorMap(Imageplane::HEAT);
	}
	else if(arg1=="Green")
	{
		ui->imageplane->setColorMap(Imageplane::GREEN);
	}
	else if(arg1=="Rainbow")
	{
		ui->imageplane->setColorMap(Imageplane::RAINBOW);
	}
	else if(arg1=="Ocean")
	{
		ui->imageplane->setColorMap(Imageplane::OCEAN);
	}

	if(!calc->isRunning())
	{
		ui->imageplane->makeImage(calc->matrixSize(), calc->matrix);
		ui->imageplane->update();
	}
}


void Widget::keyPressEvent(QKeyEvent *event)
{
	//qDebug("key pressed");
	switch(event->key())
	{
		case Qt::Key_Return:
			startCalc(); break;
		case Qt::Key_Backspace:
			stepBack(); break;
		case Qt::Key_Escape:
			ui->imageplane->stopSelecting(); break;
		case Qt::Key_F11:
			if(!ui->imageplane->isFullScreen())
			{
				//qDebug() << "show fullscreen";
				ui->imageplane->setWindowFlags(Qt::Window);
				ui->imageplane->showFullScreen();
			}
			break;
		default:
			event->ignore();
	}
}


void Widget::on_actionSave_triggered()
{
	QString filename=QFileDialog::getSaveFileName(this, tr("Save Coordinates"), QDir::currentPath(), tr("Mandelbrot-Coordinates *.mbc"));
	
	if(!filename.isEmpty())
	{
		if(!filename.endsWith(".mbc", Qt::CaseInsensitive))
		{
			filename.append(".mbc");
		}
		
		QFile file(filename);
		if(!file.open(QFile::WriteOnly | QFile::Text))
		{
			QMessageBox::warning(0, tr("Error"), tr("File could not be opened for writing!"));
			return;
		}
		QDir::setCurrent(QFileInfo(file).path());
	
		QXmlStreamWriter writer(&file);
		writer.setAutoFormatting(true);
		writer.setCodec("UTF-8");
		writer.writeStartDocument();
		writer.writeStartElement("mandelbrot-coordinates");
		writer.writeTextElement("realCenter", qStringFromLongDouble(realCenter));
		writer.writeTextElement("imagCenter", qStringFromLongDouble(imagCenter));
		writer.writeTextElement("realWidth", qStringFromLongDouble(realWidth));
		writer.writeEndElement();
		writer.writeEndDocument();
		file.close();
	}
}

void Widget::on_actionOpen_triggered()
{
	QString filename=QFileDialog::getOpenFileName(this, tr("Load Coordinates"), QDir::currentPath(), tr("Mandelbrot-Coordinates *.mbc"));
	
	if(!filename.isEmpty())
	{
		QFile file(filename);
		if(!file.open(QFile::ReadOnly | QFile::Text))
		{
			QMessageBox::warning(0, tr("Error"), tr("File could not be opened for reading!"));
			return;
		}
		QDir::setCurrent(QFileInfo(file).path());
		
		QXmlStreamReader reader(&file);
		
		if(reader.readNextStartElement() && reader.name()=="mandelbrot-coordinates")
		{
			while(reader.readNextStartElement())
			{
				if(reader.name()=="realCenter")
				{
					realCenter=longDoubleFromQString(reader.readElementText());
				}
				else if(reader.name()=="imagCenter")
				{
					imagCenter=longDoubleFromQString(reader.readElementText());
				}
				else if(reader.name()=="realWidth")
				{
					setRealWidth(longDoubleFromQString(reader.readElementText()));
				}
				else
				{
					qDebug() << "unknown XML-tag, ignored";
					reader.readElementText();
				}
			}
		}
		else
		{
			reader.raiseError(tr("invalid file!"));
		}
		
		file.close();
	
		if(reader.hasError())
		{
			QMessageBox::warning(0, tr("Error"), tr("Error parsing file:\n") + reader.errorString());
			return;
		}
		else if(file.error() != QFile::NoError)
		{
			QMessageBox::warning(0, tr("Error"), tr("File could not be read"));
			return;
		}
		
		startCalc();
	}
}


void Widget::on_actionExport_triggered()
{
	QString filename=QFileDialog::getSaveFileName(this, tr("Export Picture"), QDir::currentPath(), tr("Portable Network Graphics *.png"));
	
	if(!filename.isEmpty())
	{
		if(!filename.endsWith(".png", Qt::CaseInsensitive))
		{
			filename.append(".png");
		}
		QDir::setCurrent(QFileInfo(filename).path());
		
		if(!ui->imageplane->save(filename))
		{
			QMessageBox::warning(0, tr("Error"), tr("File could not be written"));
			return;
		}
	}
}


void Widget::closeEvent(QCloseEvent *event)
{
	calc->stop();
}



QString Widget::qStringFromLongDouble(const long double myLongDouble)
{
	std::stringstream ss;
	ss << std::setprecision(30);
	ss << myLongDouble;
	
	return QString::fromStdString(ss.str());
}


long double Widget::longDoubleFromQString(QString string)
{
	long double myLongDouble;
	std::stringstream ss(string.toStdString());
	ss >> myLongDouble;
	
	return myLongDouble;
}
