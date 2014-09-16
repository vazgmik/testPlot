#ifndef TESTPLOT_H
#define TESTPLOT_H

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_testplot.h"

class testPlot : public QMainWindow
{
	Q_OBJECT

public:
	testPlot(QWidget *parent = 0);
	void setupRealtimeDataDemo(QCustomPlot *customPlot);
	void setupRealtimeDataDemo2(QCustomPlot *customPlot);
	~testPlot();

private:
	Ui::testPlotClass ui;
	QTimer dataTimer;
	QString demoName;
public slots:
    void on_bStart_clicked();
	void on_bStop_clicked();
private slots:
    void realtimeDataSlot();
	void realtimeDataSlot2();
};

#endif // TESTPLOT_H
