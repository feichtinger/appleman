#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include "Calculator.h"
#include "defs.h"

namespace Ui {
class Widget;
}


class Widget : public QWidget
{
	Q_OBJECT
	
public:
	explicit Widget(QWidget *parent = 0);
	~Widget();

	static Widget *widget;		// for global access

	Calculator *calc;

	void startCalc();
	void stepBack();

	REALNUM getRealCenter();
	REALNUM getImagCenter();
	REALNUM getRealWidth();

	void setRealCenter(REALNUM rc);
	void setImagCenter(REALNUM ic);
	void setRealWidth(REALNUM rw);
	
	void keyPressEvent(QKeyEvent *event);
	
	
	static QString qStringFromLongDouble(const long double myLongDouble);
	static long double longDoubleFromQString(QString string);

public slots:
	void updateProgressBar(int percent);
	
private slots:
	void on_pushButton_run_clicked();
	void on_pushButton_stepBack_clicked();
	void on_pushButton_restart_clicked();
	void on_comboBox_Color_currentIndexChanged(const QString &arg1);
	void calcFinished();
	
	void on_actionSave_triggered();
	void on_actionOpen_triggered();
	
	void on_actionExport_triggered();
	
private:
	Ui::Widget *ui;
	REALNUM realCenter;
	REALNUM imagCenter;
	REALNUM realWidth;

	void closeEvent(QCloseEvent *event);

};

#endif // WIDGET_H
