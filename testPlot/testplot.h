#ifndef TESTPLOT_H
#define TESTPLOT_H

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_testplot.h"


const double time_step = 0.1;

struct transfer_function_t {
	enum tfType {FirstOrder = 0, SecondOrderAperiodic = 1, SecondOrderOscillating = 2} ;
	tfType function_type;
	transfer_function_t()
		: function_type(FirstOrder)
		, time_constant1(100)
		, gain(1)
		, PV(0)
	{}
	double time_constant1, time_constant2, gain, delay;

	double PV;
	double operator()(double dt, double OP) {
		double u = std::max(0.0, std::min(1.0, OP / 100));
		double y = PV / 100;

		switch (function_type) {
		case FirstOrder:
			y = (time_constant1 * y + dt * gain * u) / (time_constant1 + dt);
			break;
		default:
			throw std::runtime_error("not implemented");
		}
		y = std::max(0.0, std::min(1.0, y));
		PV = y * 100;
		return PV;
	}
};


class testPlot : public QMainWindow
{
	Q_OBJECT

public:
	testPlot(QWidget *parent = 0);
	void setupRealtimeDataDemo(QCustomPlot *customPlot);
	void setupRealtimeDataDemo2(QCustomPlot *customPlot);
	~testPlot();
	double cat;
private:
	Ui::testPlotClass ui;
	QTimer dataTimer;
	QString demoName;
	double Nu_code, Nx_code;
	double OPn, PVn;

	transfer_function_t transfer_function;

private:
	unsigned short readFromDevice();
	void writeToDevice(unsigned short PV_code);

public slots:
    void on_bStart_clicked();
	void on_bStop_clicked();
private slots:
    void realtimeDataSlot();
	void realtimeDataSlot2();
	void timeout_one_second();
};

#endif // TESTPLOT_H
