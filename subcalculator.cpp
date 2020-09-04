#include "subcalculator.h"
#include <QDebug>

#define STEP_ITER		500		// maximum number of iterations in one step
//#define STEP_ITER		5000

SubCalculator::SubCalculator(QSize size, int left, int right, int top, int bottom,
							 REALNUM realCenter, REALNUM imagCenter, REALNUM realWidth, REALNUM imagHeight,
						     REALNUM **realZMatrix, REALNUM **imagZMatrix, int **matrix,
						     QObject *parent) :
	QThread(parent)
{
	this->size=size;
	this->left=left;
	this->right=right;
	this->top=top;
	this->bottom=bottom;
	
	this->realCenter=realCenter;
	this->imagCenter=imagCenter;
	this->realWidth=realWidth;
	this->imagHeight=imagHeight;
	
	this->realZMatrix=realZMatrix;
	this->imagZMatrix=imagZMatrix;
	this->matrix=matrix;
	
	running=true;
	runs=0;
	
	nTotalFound=0;		// total number of found pixels in all runs up to now that are not part of the set
}


void SubCalculator::run()
{
	//qDebug() << "thread started";
	
	int nFound;			// number of found pixels in current run that are not part of the set

	do
	{	
		nFound=0;
		for(int y=top; y<bottom; y++)
		{
			for(int x=left; x<right; x++)
			{
				// transform pixel coordinates to real and imag part
				INIT_REALNUM(x_);
				x_=x;
				INIT_REALNUM(y_);
				y_=y;
				
				INIT_REALNUM(realC);
				realC = (x_/size.width() - 1.0/2) * realWidth + realCenter;
				INIT_REALNUM(imagC);
				imagC = -(y_/size.height() - 1.0/2) * imagHeight + imagCenter;
	
				INIT_REALNUM(realZ);
				INIT_REALNUM(imagZ);
				
				if(runs==0)
				{
					// initialize
					realZ=realC;
					imagZ=imagC;
					matrix[x][y]=0;
				}
				else
				{
					// load values from old run
					realZ=realZMatrix[x][y];
					imagZ=imagZMatrix[x][y];
				}
				
				// if pixel is already signed as 'not in set' don't calculate again
				if(matrix[x][y]<=0)
				{
					REALNUM realZ_2;
					REALNUM imagZ_2;
					
					int iter=1;
					while(true)
					{	
						// do the iteration z=z^2+c
						realZ_2=realZ*realZ;
						imagZ_2=imagZ*imagZ;
						imagZ = 2*realZ*imagZ + imagC;
						realZ = realZ_2 - imagZ_2 + realC;
						
						if(iter >= STEP_ITER)
						{
							// (still) part of set, store negative iter value to sign this
							matrix[x][y]-=iter;
							break;
						}
						
						if(realZ_2+imagZ_2 > 4)
						{
							// not in set, store positive iter value
							nFound++;
							nTotalFound++;
							matrix[x][y]=-matrix[x][y]+iter;
							break;
						}
						
						iter++;
					}
					realZMatrix[x][y]=realZ;
					imagZMatrix[x][y]=imagZ;
				}
			}
		}
		
		runs++;
		emit runFinished(runs, nFound, nTotalFound);
		//qDebug() << "run:" << runs << "nFound:" << nFound;
		
	}
	while((nTotalFound==0 || nFound>0) && running);
	
	//qDebug() << "thread ended";
}


void SubCalculator::stop()
{
	//qDebug() << "received stop";
	running=false;
}

int SubCalculator::getRuns()
{
	return runs;
}

int SubCalculator::getTotalFound()
{
	return nTotalFound;
}
