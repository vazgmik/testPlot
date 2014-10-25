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
		, time_constant2(0)
		, gain(1)
		, PV(0)
		,delay(0)
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
	void setData(int t1,int t2,int g,int d)
	{
		time_constant1 = t1;
		time_constant2 = t2;
		gain = g;
		delay = d;
	}
};


class testPlot : public QMainWindow
{
	Q_OBJECT

public:
	short io_delay;
	testPlot(QWidget *parent = 0);
	void setupRealtimeData1(QCustomPlot *,QString ,QString);
	void setupRealtimeData2(QCustomPlot *,QString ,QString);
	void onStart();
	~testPlot();
	QString Nx_string, Nu_string, PVstring, OPstring;
	QString TimeConst2,EpsConst;
	bool b_Stop;
	double x_value;
	bool zoom;
private:
	Ui::testPlotClass ui;
	QTimer dataTimer;
	QTimer *timer;
	QString demoName;
	double Nu_code, Nx_code;
	double OPn, PVn;
	QCPAxisRect *wideAxisRect;
	QCPAxisRect *wideAxisRect2;
	QCPItemText *textLabel;
	QCPItemText *textLabel2;
	transfer_function_t transfer_function;
	
private:
	unsigned short readFromDevice();
	void writeToDevice(unsigned short PV_code);

public slots:
	void SaveCSV();
	//void ();
	void OnZoom();
	void SaveSettings();
	void LoadSettings();
	void on_cbModel_changed(int index);
	void on_sbNx_changed(int);
	void setRangeOver(QCPRange,QCPRange);
	void setRangeOver2(QCPRange,QCPRange);
private slots:
    void realtimeDataSlot();
	void timeout_one_second();
};

#endif // TESTPLOT_H
