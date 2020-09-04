#include "Calculator.h"
#include "widget.h"
#include <QDebug>

Calculator::Calculator(QSize size, REALNUM(realCenter), REALNUM(imagCenter), REALNUM(realWidth))
{
	this->size=size;
	this->realCenter=realCenter;
	this->imagCenter=imagCenter;
	this->realWidth=realWidth;
	imagHeight=realWidth/size.width()*size.height();	

	// allocate memory
	matrix=new int* [size.width()];
	for(int i=0; i<size.width(); i++)
	{
		matrix[i] = new int[size.height()];
	}
	
	realZMatrix=new REALNUM* [size.width()];
	for(int i=0; i<size.width(); i++)
	{
		realZMatrix[i] = new REALNUM[size.height()];
	}
	
	imagZMatrix=new REALNUM* [size.width()];
	for(int i=0; i<size.width(); i++)
	{
		imagZMatrix[i] = new REALNUM[size.height()];
	}
	
	
	//qDebug("size=%d x %d", size.width(), size.height());
	
	//qDebug() << "realCenter:" << realCenter.get_d() << "imagCenter:" << imagCenter.get_d();
	qDebug() << "realCenter:" <<  Widget::qStringFromLongDouble(realCenter) << "imagCenter:" << Widget::qStringFromLongDouble(imagCenter);
	
	
	// divide the whole picture in N_SPLIT x N_SPLIT subpictures and calculate them in seperate threads
	
	int left=0;
	for(int i=0; i<N_SPLIT; i++)
	{
		int right=(i+1)*size.width()/N_SPLIT;
		int top=0;
		for(int j=0; j<N_SPLIT; j++)
		{
			int bottom=(j+1)*size.height()/N_SPLIT;
			
			threads[i][j]=new SubCalculator(size, left, right, top, bottom, realCenter, imagCenter, realWidth, imagHeight, realZMatrix, imagZMatrix, matrix);
			connect(threads[i][j], SIGNAL(runFinished(int,int,int)), this, SLOT(checkThreadProgress(int,int,int)));
			
			top=bottom;
		}
		left=right;
	}
	
	pixelsNotInSet=0;
		
	//qDebug("new calculator generated");
}

void Calculator::run()
{
	//qDebug("calculation started...");
	
	emit progressChanged(0);
	
	for(int i=0; i<N_SPLIT; i++)
	{
		for(int j=0; j<N_SPLIT; j++)
		{
			threads[i][j]->start();
		}
	}
	
	
	// wait for all threads to be finished
	for(int i=0; i<N_SPLIT; i++)
	{
		for(int j=0; j<N_SPLIT; j++)
		{
			threads[i][j]->wait();
		}
	}
	
	emit progressChanged(100);
	
	//qDebug("calculation finished");
}

void Calculator::checkThreadProgress(int run, int nFound, int nTotalFound)
{
	pixelsNotInSet+=nFound;
	emit progressChanged(100*pixelsNotInSet/(size.width()*size.height()));
	
	
	int finishedThreads=0;
	int activeThreads=0;		// active means that it has already found something but has not finished yet
	
	for(int i=0; i<N_SPLIT; i++)
	{
		for(int j=0; j<N_SPLIT; j++)
		{	
			if(threads[i][j]->isFinished())
			{
				finishedThreads++;
			}
			if(threads[i][j]->isRunning() && threads[i][j]->getTotalFound()!=0)
			{
				activeThreads++;
			}
		}
	}
	
	//qDebug() << "active threads:" << activeThreads;
	
	// if more than half of the threads are finished and there are no active threads, stop calculation
	if(finishedThreads>N_SPLIT*N_SPLIT/4 && activeThreads==0)
	{
		stop();
		//qDebug() << "no active threads left, stop";
	}
}


QSize Calculator::matrixSize()
{
	return size;
}


Calculator::~Calculator()
{
	//qDebug() << "deconstructor called";
	
	// stop all threads and wait for them to finish
	stop();
	while(isRunning());
	
	for(int i=0; i<N_SPLIT; i++)
	{
		for(int j=0; j<N_SPLIT; j++)
		{
			delete threads[i][j];
		}
	}
	
	// free memory
	for(int i=0; i<size.width(); i++)
	{
		delete[] matrix[i];
	}
	delete[] matrix;
	
	for(int i=0; i<size.width(); i++)
	{
		delete[] realZMatrix[i];
	}
	delete[] realZMatrix;
	
	for(int i=0; i<size.width(); i++)
	{
		delete[] imagZMatrix[i];
	}
	delete[] imagZMatrix;
	
}

void Calculator::stop()
{
	for(int i=0; i<N_SPLIT; i++)
	{
		for(int j=0; j<N_SPLIT; j++)
		{
			threads[i][j]->stop();
		}
	}
}
