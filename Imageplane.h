#ifndef IMAGEPLANE_H
#define IMAGEPLANE_H

#include <QWidget>
#include "defs.h"

class Imageplane : public QWidget
{
	Q_OBJECT
public:
	explicit Imageplane(QWidget *parent = 0);

	enum COLORMAP {WHITE, HEAT, GREEN, RAINBOW, OCEAN};

	void makeImage(QSize size, int **matrix);
	void setColorMap(COLORMAP cm);
	bool save(QString filename);
	void stopSelecting();

private:
	void paintEvent(QPaintEvent *event);			// is draws the image
	void mouseReleaseEvent(QMouseEvent *event);		// detect mouse clicks
	void mouseMoveEvent(QMouseEvent *event);		// detect mouse movement
	void keyPressEvent(QKeyEvent *event);

	QImage *image;		// the image generated from matrix-data
	bool selecting;		// true if the user is currently selecting a subregion
	QPoint center;		// centerpoint of selection
	QRect selection;	// the selected subregion

	COLORMAP colorMap;

	
signals:
	
public slots:
	
};

#endif // IMAGEPLANE_H
