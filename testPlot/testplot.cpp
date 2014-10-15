#include "testplot.h"
#include "HostUsbHelper.h"
#include <usb.h>

testPlot::testPlot(QWidget *parent)
	: QMainWindow(parent),x_value(0)
	, PVn(0)
	, OPn(0)
	, Nu_code(0)
	, Nx_code(0)
	,b_Stop(false)
{
	ui.setupUi(this);
	demoName = "Model";
	setWindowTitle("QCustomPlot: "+demoName);
	ui.lb_TimeEps->setVisible(false);
	ui.sbTimeEps->setVisible(false);
	timer = new QTimer(this);

	connect(ui.cbModelType,SIGNAL(currentIndexChanged (int)),SLOT(on_cbModel_changed(int)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)) , this, SLOT(on_sbNx_changed(int)));
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout_one_second()));
	
	onStart();
	setupRealtimeData(ui.plot_wid);
}

testPlot::~testPlot()
{

}
void testPlot::onStart()
{
	b_Stop = false;
	connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
	dataTimer.start(100);
	timer->start(100);
	ui.plot_wid->setInteraction(QCP::iRangeDrag,false);
	ui.plot_wid->setInteraction(QCP::iRangeZoom,false);
	statusBar()->clearMessage();
	ui.plot_wid->replot();
}

void testPlot::onStop()
{
	b_Stop = true;
	prev = wideAxisRect->axis(QCPAxis::atLeft)->range();
	ui.plot_wid->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	timer->stop();
	dataTimer.stop();
}

void testPlot::on_cbModel_changed(int index)
{
	if(index == 0)
	{
		ui.lb_TimeEps->setVisible(false);
		ui.sbTimeEps->setVisible(false);
	}
	else if(index == 1)
	{
		ui.lb_TimeEps->setText("<html><head/><body><p align='center'><span style=' font-size:14pt;font-weight:400;'>T</span><span style=' font-size:14pt; vertical-align:sub;'>2</span><span style=' font-size:14pt;'>:</span></p></body></html>");
		ui.lb_TimeEps->setVisible(true);
		ui.sbTimeEps->setVisible(true);
	}
	else if(index == 2)
	{
		ui.lb_TimeEps->setText("<html><head/><body><p align='center'><span style=' font-size:14pt; font-weight:400;'>x:</span></p></body></html>");
		ui.lb_TimeEps->setVisible(true);
		ui.sbTimeEps->setVisible(true);
	}
	
	
}

void testPlot::on_sbNx_changed(int s)
{
	ui.lbNx->setText(QString::number(s));
	try {
		writeToDevice(s);
	}
	catch (std::runtime_error& e) {
		Nx_string = Nu_string = PVstring = OPstring = "IO error";
		QMessageBox::critical(this,QTextCodec::codecForLocale()->toUnicode( "ОШИБКА СВЯЗИ"), QTextCodec::codecForLocale()->toUnicode( "Ошибка устройства ввода/вывода" ));
	}
}



void testPlot::setRangeOver(QCPRange y1)
{
	if(b_Stop)
	{
		QCPRange y2 = wideAxisRect->axis(QCPAxis::atLeft,1)->range();
		double delta1 = y1.upper -  y1.lower;
		double delta2 = y2.upper -  y2.lower;
		double delta3 = prev.upper-  y1.upper;
		double delta4 =prev.lower-  y1.lower;
		double new_y2_upper =  -delta2*delta3/delta1 +y2.upper;
		double new_y2_lower =  -delta2*delta4/delta1 +y2.lower;
		prev = y1;
		wideAxisRect->axis(QCPAxis::atLeft,1)->setRange(new_y2_lower,new_y2_upper);
	}
}

void testPlot::setupRealtimeData(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif
	
	/*demoName = "Real Time Data Demo";
	customPlot->yAxis->setOffset(20);
	customPlot->addGraph(); // blue line
	customPlot->graph(0)->setPen(QPen(Qt::blue));
	//customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
	customPlot->graph(0)->setAntialiasedFill(false);

	customPlot->addGraph(); // red line
	customPlot->graph(1)->setPen(QPen(Qt::red));
	//customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
	customPlot->graph(1)->setAntialiasedFill(false);
	customPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
	customPlot->xAxis->setAutoTickStep(false);
	customPlot->xAxis->setTickStep(1);
	//customPlot->axisRect()->setupFullAxesBox();

	// make left and bottom axes transfer their ranges to right and top axes:
	connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

	connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
	dataTimer.start(10); // Interval 0 means to refresh as fast as possible
	ui.plot_wid->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);*/


	// configure axis rect:
	customPlot->plotLayout()->clear(); // clear default axis rect so we can start from scratch

	wideAxisRect = new QCPAxisRect(customPlot);
	wideAxisRect->setupFullAxesBox(true);
	//wideAxisRect->axis(QCPAxis::atBottom)->setAutoTickStep(false);
	//wideAxisRect->axis(QCPAxis::atBottom)->setTickStep(1);
	wideAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::blue);
	wideAxisRect->axis(QCPAxis::atLeft)->setLabel("Nx");
	wideAxisRect->addAxis(QCPAxis::atLeft)->setTickLabelColor(Qt::red); // add an extra axis on the left and color its numbers
	
	customPlot->plotLayout()->addElement(0, 0, wideAxisRect); // insert axis rect in first row
	wideAxisRect->axis(QCPAxis::atLeft, 1)->setRangeLower(0);
	wideAxisRect->axis(QCPAxis::atLeft,1)->setLabel("Nu");

	QCPGraph *mainGraph1 = customPlot->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
	mainGraph1->setPen(QPen(Qt::blue, 2));
	mainGraph1->setBrush(QColor(255, 161, 0, 50));

	QCPGraph *mainGraph2 = customPlot->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft, 1));
	mainGraph2->setPen(QPen(Qt::red, 2));
	mainGraph2->setBrush(QColor(110, 170, 110, 30));
	
    customPlot->graph(0)->setAntialiasedFill(false);
	customPlot->graph(1)->setAntialiasedFill(false);
	prev = wideAxisRect->axis(QCPAxis::atLeft)->range();
	connect(wideAxisRect->axis(QCPAxis::atLeft), SIGNAL(rangeChanged(QCPRange)), this, SLOT(setRangeOver(QCPRange)));
	
}


void testPlot::realtimeDataSlot()
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	double key = 0;
#else
	double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
	static double lastPointKey = 0;
	if (key-lastPointKey > 0.01) // at most add point every 10 ms
	{
		ui.plot_wid->graph(0)->addData(x_value, Nx_code);
		ui.plot_wid->graph(0)->rescaleValueAxis();
		ui.plot_wid->graph(1)->addData(x_value, Nu_code);
		ui.plot_wid->graph(1)->rescaleValueAxis(true);
		lastPointKey = x_value;
		x_value += 0.01;
	}
	
	wideAxisRect->axis(QCPAxis::atBottom,0)->setRange(0,x_value);
	ui.plot_wid->replot();
	
	static double lastFpsKey;
	static int frameCount;
	++frameCount;
	if (key-lastFpsKey > 2) 
	{
		statusBar()->showMessage(
			QString("PVn: ")+PVstring+QString("  OPn: ")+OPstring
		//	.arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
		//	.arg(ui.plot_wid->graph(0)->data()->count()+ui.plot_wid->graph(1)->data()->count())
			, 0);
		lastFpsKey = key;
		frameCount = 0;
	}
}

void testPlot::timeout_one_second()
{
	QMessageBox msg;
	try {
		Nu_code = readFromDevice();
		writeToDevice(Nx_code);
		Nx_string = QString::number(Nx_code);
		Nu_string = QString::number(Nu_code);

		OPn = 100.0 * Nu_code / 1023.0;
		PVn = transfer_function(time_step, OPn);

		Nx_code = (PVn / 100.0) * 1023; 

		PVstring = QString::number(PVn);
		OPstring = QString::number(OPn);
	}
	catch (std::runtime_error& e) {
		Nx_string = Nu_string = PVstring = OPstring = "IO error";
		statusBar()->showMessage(QTextCodec::codecForLocale()->toUnicode( "Ошибка устройства ввода/вывода.Проверьте вставлено ли устройство в порт." ));
		
	}

	static int filter = 0;
	if (filter == 0) {
    	ui.progressBar->setValue(Nu_code);
		ui.lbNx->setText(Nx_string);
		ui.lbNu->setText(Nu_string);
	}
	filter = (filter + 1) % 10;
	//msg.exec();
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
//	adc_data *= 0.25;
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

/* - замена осей для масштабирования
void testPlot::on_rbNx_changedGraph()
{
	if(b_Stop )
	{
		ui.plot_wid->graph(0)->setValueAxis(wideAxisRect->axis(QCPAxis::atLeft));
		ui.plot_wid->graph(1)->setValueAxis(wideAxisRect->axis(QCPAxis::atLeft,1));
		ui.plot_wid->graph(0)->rescaleValueAxis();
		ui.plot_wid->graph(1)->rescaleValueAxis(true);
		wideAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::blue);
		wideAxisRect->axis(QCPAxis::atLeft,1)->setTickLabelColor(Qt::red);
	}
	ui.plot_wid->replot();
}
void testPlot::on_rbNu_changedGraph()
{
	if(b_Stop )
	{
		ui.plot_wid->graph(1)->setValueAxis(wideAxisRect->axis(QCPAxis::atLeft));
		ui.plot_wid->graph(0)->setValueAxis(wideAxisRect->axis(QCPAxis::atLeft,1));
		ui.plot_wid->graph(0)->rescaleValueAxis();
		ui.plot_wid->graph(1)->rescaleValueAxis(true);
		wideAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::red);
		wideAxisRect->axis(QCPAxis::atLeft,1)->setTickLabelColor(Qt::blue);
	}
	ui.plot_wid->replot();
}
*/