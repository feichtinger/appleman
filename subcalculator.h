#ifndef SUBCALCULATOR_H
#define SUBCALCULATOR_H

#include <QThread>
#include <QSize>
#include "defs.h"


class SubCalculator : public QThread
{
	Q_OBJECT
public:
	explicit SubCalculator(QSize size, int left, int right, int top, int bottom,
						   REALNUM realCenter, REALNUM imagCenter, REALNUM realWidth, REALNUM imagHeight,
						   REALNUM **realZMatrix, REALNUM **imagZMatrix, int **matrix,
						   QObject *parent = 0);
	
	void run();
	int getRuns();
	int getTotalFound();
	
signals:
	void runFinished(int run, int nFound, int nTotalFound);
	
public slots:
	void stop();
	
private:
	
	QSize size;				// total size of picture in pixels
	int left, right, top, bottom;	// section to be calculated by this thread exclusive right and bottom!
	
	
	REALNUM realCenter;		// real part of image center
	REALNUM imagCenter;		// imag part of image center
	REALNUM realWidth;		// real part from left image border to right image border
	REALNUM imagHeight;		// imag part from bottom image border to top image border
	
	REALNUM **realZMatrix;	// matrix for real z values
	REALNUM **imagZMatrix;	// matrix for imag z values
	int **matrix;			// result matrix (number of iterations)
	
	bool running;
	int runs;
	int nTotalFound;
	
};

#endif // SUBCALCULATOR_H
