#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QSize>
#include <QThread>
#include <QWidget>

#include "defs.h"
#include "subcalculator.h"

#define N_SPLIT	8


class Calculator:public QThread
{
	Q_OBJECT

public:
	Calculator(QSize size, REALNUM realCenter, REALNUM imagCenter, REALNUM scale);

	~Calculator();

	void run();

	int **matrix;		// number of iterations

	QSize matrixSize();

signals:
	void progressChanged(int percent);
	
public slots:
	void checkThreadProgress(int run, int nFound, int nTotalFound);
	void stop();

private:
	QSize size;
	REALNUM realCenter;		// real part of image center
	REALNUM imagCenter;		// imag part of image center
	REALNUM realWidth;		// real part from left image border to right image border
	REALNUM imagHeight;		// imag part from bottom image border to top image border
	
	REALNUM **realZMatrix;	// matrix for real z values
	REALNUM **imagZMatrix;	// matrix for imag z values	
	
	SubCalculator *threads[N_SPLIT][N_SPLIT];
	
	int pixelsNotInSet;
};

#endif // CALCULATOR_H
