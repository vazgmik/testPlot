#include "testplot.h"
#include "HostUsbHelper.h"

#include <usb.h>



testPlot::testPlot(QWidget *parent)
	: QMainWindow(parent),cat(0)
	, PVn(0)
	, OPn(0)
	, Nu_code(0)
	, Nx_code(0)
{
	ui.setupUi(this);
	connect(ui.pbStart,SIGNAL(clicked()),SLOT(on_bStart_clicked()));
	connect(ui.pbStop,SIGNAL(clicked()),SLOT(on_bStop_clicked()));

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout_one_second()));
	timer->start(100);



}

testPlot::~testPlot()
{

}

void testPlot::on_bStart_clicked()
{
	setupRealtimeDataDemo(ui.plot_wid1);
	setupRealtimeDataDemo2(ui.plot_wid2);
	ui.plot_wid1->setInteraction(QCP::iRangeDrag,false);
	ui.plot_wid1->setInteraction(QCP::iRangeZoom,false);
	ui.plot_wid2->setInteraction(QCP::iRangeDrag,false);
	ui.plot_wid2->setInteraction(QCP::iRangeZoom,false);
	setWindowTitle("QCustomPlot: "+demoName);
	statusBar()->clearMessage();
	ui.plot_wid1->replot();
}

void testPlot::on_bStop_clicked()
{
	ui.plot_wid1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	ui.plot_wid2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	dataTimer.stop();
}

void testPlot::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif
	demoName = "Real Time Data Demo";

	customPlot->addGraph(); // blue line
	customPlot->graph(0)->setPen(QPen(Qt::blue));
	customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
	customPlot->graph(0)->setAntialiasedFill(false);
	customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
	customPlot->xAxis->setDateTimeFormat("ss");
	customPlot->xAxis->setAutoTickStep(false);
	customPlot->xAxis->setTickStep(2);
	customPlot->axisRect()->setupFullAxesBox();

	// make left and bottom axes transfer their ranges to right and top axes:
	connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

	connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
	dataTimer.start(10); // Interval 0 means to refresh as fast as possible
	ui.plot_wid1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void testPlot::setupRealtimeDataDemo2(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif
	customPlot->addGraph(); 
	customPlot->graph(0)->setPen(QPen(Qt::red));
	customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
	customPlot->graph(0)->setAntialiasedFill(false);
	customPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
	customPlot->xAxis->setAutoTickStep(false);
	customPlot->xAxis->setTickStep(1);
	customPlot->axisRect()->setupFullAxesBox();

	// make left and bottom axes transfer their ranges to right and top axes:
	connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

	connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot2()));

}

void testPlot::realtimeDataSlot()
{
	// calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	double key = 0;
#else
	double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
	static double lastPointKey = 0;
	if (key-lastPointKey > 0.01) 
	{
		double value1 = Nu_code;//sin(key*1.3+cos(key*1.2)*1.2)*7 - sin(key*0.9+0.26)*24 + 26;
		ui.plot_wid1->graph(0)->addData(key, value1);
		ui.plot_wid1->graph(0)->rescaleValueAxis(true);
		lastPointKey = key;
	}
	ui.plot_wid1->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
	ui.plot_wid1->replot();

	static double lastFpsKey;
	static int frameCount;
	++frameCount;
	if (key-lastFpsKey > 2) 
	{
		statusBar()->showMessage(
			QString("%1 FPS, Total Data points: %2")
			.arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
			.arg(ui.plot_wid1->graph(0)->data()->count()+ui.plot_wid2->graph(0)->data()->count())
			, 0);
		lastFpsKey = key;
		frameCount = 0;
	}
}


void testPlot::realtimeDataSlot2()
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	double key = 0;
#else
	double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
	static double lastPointKey = 0;
	if (key-lastPointKey > 0.01) // at most add point every 10 ms
	{
		double value1 = Nx_code;//(rand()%7-rand()%8); 
		ui.plot_wid2->graph(0)->addData(cat, value1);
		ui.plot_wid2->graph(0)->rescaleValueAxis(true);
		lastPointKey = cat;
		cat += 0.01;
	}
	
	ui.plot_wid2->xAxis->setRange(cat,2, Qt::AlignRight);
	ui.plot_wid2->replot();

}

void testPlot::timeout_one_second()
{
	QString Nx_string, Nu_string, PVstring, OPstring;
	try {
		Nu_code = readFromDevice();
		writeToDevice(Nx_code);
		Nx_string = QString::number(Nx_code);
		Nu_string = QString::number(Nu_code);

		OPn = 100.0 * Nu_code / 1024.0;
		PVn = transfer_function(time_step, OPn);

		Nx_code = (PVn / 100.0) * 256; 

		PVstring = QString::number(PVn);
		OPstring = QString::number(OPn);
	}
	catch (std::runtime_error& e) {
		Nx_string = Nu_string = PVstring = OPstring = "IO error";
	}

	static int filter = 0;
	if (filter == 0) {
		ui.lbNx->setText(Nx_string);
		ui.lbNu->setText(Nu_string);
		ui.lbPVn->setText(PVstring);
		ui.lbOPn->setText(OPstring);
	}
	filter = (filter + 1) % 10;
}

unsigned short testPlot::readFromDevice()
{
	static bool no_link_before = true;
	
	usb_dev_handle *handle;
	handle = usbOpenDevice(0x16C0, "obdev.at", 0x05DC, "template");

	if (!handle) {
		no_link_before = true;
		throw std::runtime_error("ошибка связи");		
	}

	char buffer[255];
	int nBytes = usb_control_msg(handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
		USB_DATA_OUT, 0, 0, (char *)buffer, sizeof(buffer), 5000);

	unsigned raw_data = *((unsigned *) buffer);
	double adc_data = 0xFFFF & raw_data; // АЦП в Tiny 10-разрядный
	adc_data *= 0.25;
	if (!no_link_before) {
		// Если в прошлый раз был сигнал, фильтруем
		adc_data = 0.05 * (adc_data) + 0.95 * Nu_code;
	}
	usb_close(handle);



	no_link_before = false;
	return adc_data;
}

void testPlot::writeToDevice(unsigned short PV_code)
{
	usb_dev_handle *handle;
	handle = usbOpenDevice(0x16C0, "obdev.at", 0x05DC, "template");
	if (!handle)
		throw std::runtime_error("ошибка связи");


	unsigned data_packet = PV_code << 16;

	int nBytes = usb_control_msg(handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
		USB_DATA_IN, 0, 0, (char *) &data_packet, sizeof(data_packet), 5000);

	usb_close(handle);

}
