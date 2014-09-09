#ifndef TESTPLOT_H
#define TESTPLOT_H

#include <QtWidgets/QMainWindow>
#include "ui_testplot.h"

class testPlot : public QMainWindow
{
	Q_OBJECT

public:
	testPlot(QWidget *parent = 0);
	~testPlot();

private:
	Ui::testPlotClass ui;
};

#endif // TESTPLOT_H
