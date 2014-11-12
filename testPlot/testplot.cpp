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
	,TimeConst2("<html><head/><body><p align='center'><span style=' font-size:14pt;font-weight:400;'>T</span><span style=' font-size:14pt; vertical-align:sub;'>2</span><span style=' font-size:14pt;'>:</span></p></body></html>")
	,EpsConst("<html><head/><body><p align='center'><span style=' font-size:14pt; font-weight:400;'>x:</span></p></body></html>")
	,zoom(false)
	,io_delay(0)
	
{
	ui.setupUi(this);
	demoName = "Model";
	setWindowTitle("QCustomPlot: "+demoName);
	ui.lb_TimeEps->setVisible(false);
	ui.sbTimeEps->setVisible(false);
	ui.sbGain->setValue(transfer_function.gain);
	ui.sbDelay->setValue(transfer_function.delay);
	ui.sbTime1->setValue(transfer_function.time_constant1);
	ui.sbTimeEps->isVisible() && ui.sbTimeEps->text() == TimeConst2 ? ui.sbTimeEps->setValue(transfer_function.time_constant2) : 0;
	ui.graph_range->setValue(20);
	gr1_wideAxisRect = new QCPAxisRect(ui.plot_wid);
	gr2_wideAxisRect = new QCPAxisRect(ui.plot_wid_2);
	setupRealtimeData(ui.plot_wid,"Nx","Nu",gr1_wideAxisRect,gr1_textError);
	setupRealtimeData(ui.plot_wid_2,"OPn","PVn",gr2_wideAxisRect,gr2_textError);
	gr1_wideAxisRect->axis(QCPAxis::atLeft)->setProperty("name","Graph1");
	gr2_wideAxisRect->axis(QCPAxis::atLeft)->setProperty("name","Graph2");
	timer = new QTimer(this);


	connect(gr1_wideAxisRect->axis(QCPAxis::atLeft), SIGNAL(rangeChanged(QCPRange,QCPRange)), this, SLOT(setRangeOver(QCPRange,QCPRange)));
	connect(gr2_wideAxisRect->axis(QCPAxis::atLeft), SIGNAL(rangeChanged(QCPRange,QCPRange)), this, SLOT(setRangeOver2(QCPRange,QCPRange)));
	connect(ui.cbModelType,SIGNAL(currentIndexChanged (int)),SLOT(on_cbModel_changed(int)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)) , this, SLOT(on_sbNx_changed(int)));
	connect(ui.save_ini, SIGNAL(triggered()), this, SLOT(SaveSettings()));
	connect(ui.load_ini, SIGNAL(triggered()), this, SLOT(LoadSettings()));
	connect(ui.cbZoom, SIGNAL(clicked()), this, SLOT(OnZoom()));
	connect(ui.action_CSV, SIGNAL(triggered()), this, SLOT(SaveCSV()));
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout_one_second()));
	onStart();
	
}

testPlot::~testPlot()
{

}
void testPlot::onStart()
{
	b_Stop = false;
	connect(timer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
	timer->start(100);
	statusBar()->clearMessage();
	ui.plot_wid->replot();
}

void testPlot::OnZoom()
{
	if(ui.cbZoom->isChecked())
	{
		zoom = true;
		ui.plot_wid->setInteraction(QCP::iRangeDrag,true);
		ui.plot_wid->setInteraction(QCP::iRangeZoom,true);
		ui.plot_wid_2->setInteraction(QCP::iRangeDrag,true);
		ui.plot_wid_2->setInteraction(QCP::iRangeZoom,true);
	}
	else
	{
		zoom = false;
		ui.plot_wid->setInteraction(QCP::iRangeDrag,false);
		ui.plot_wid->setInteraction(QCP::iRangeZoom,false);
		ui.plot_wid_2->setInteraction(QCP::iRangeDrag,false);
		ui.plot_wid_2->setInteraction(QCP::iRangeZoom,false);
	}
	ui.plot_wid->replot();
	ui.plot_wid_2->replot();
}

void testPlot::SaveSettings()
{
	QFileDialog dialog(this);
	dialog.setWindowModality(Qt::WindowModal);
	QSettings settings(dialog.getSaveFileName(0, "Save file", QDir::currentPath(),tr("Ini (*.ini)")), QSettings::IniFormat);
	settings.setValue("Gain/val", ui.sbGain->value());
	settings.setValue("Delay/val", ui.sbDelay->value());
	settings.setValue("Time1/val", ui.sbTime1->value());
	ui.sbTimeEps->isVisible() && ui.sbTimeEps->text() == EpsConst ? settings.setValue("Eps/val", ui.sbTimeEps->value()) : settings.setValue("Time2/val", ui.sbTimeEps->value());
	settings.sync();
	this->raise();
	this->activateWindow();
}

void testPlot::SaveCSV()
{
	QFileDialog dialog(this);
	dialog.setWindowModality(Qt::WindowModal);
	QFile file(dialog.getSaveFileName(0, "Save file", QDir::currentPath(),tr("CSV (*.csv)")));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << "t;" << "Nx;" << "Nu;" << "OPn;" << "PVn" <<"\n";
	QCPDataMap * gr1= ui.plot_wid->graph(0)->data();
	QCPDataMap * gr2= ui.plot_wid->graph(1)->data();
	QCPDataMap * gr3= ui.plot_wid_2->graph(0)->data();
	QCPDataMap * gr4= ui.plot_wid_2->graph(1)->data();
	int size = ui.plot_wid->graph(0)->data()->size();
	for(auto i=0;i<size;++i)
	{
		double t = gr1->keys().at(i);
		double Nx = gr1->values().at(i).value;
		double Nu = gr2->values().at(i).value;
		double OP = gr3->values().at(i).value;
		double PV = gr4->values().at(i).value;
		out <<"\""<<QString::number(t).replace('.',',') <<"\""<< ";" <<"\""<< QString::number(Nx).replace('.',',')  <<"\""<< ";"<<"\""<<QString::number(Nu).replace('.',',') <<"\""<< ";"<<"\""<<QString::number(OP).replace('.',',') <<"\""<< ";"<<"\""<<QString::number(PV).replace('.',',') <<"\""<<"\n";
	}
	out.flush();
	file.close();
	this->raise();
	this->activateWindow();
}

void testPlot::LoadSettings()
{
	QFileDialog dialog(this);
	dialog.setWindowModality(Qt::WindowModal);
	dialog.setNameFilter(tr("Ini (*.ini)"));
	QStringList fileNames;
	if (dialog.exec())
	{
		fileNames = dialog.selectedFiles();
		QSettings settings(fileNames[0], QSettings::IniFormat);
		ui.sbGain->setValue(settings.value("Gain/val").toDouble());
		ui.sbDelay->setValue(settings.value("Delay/val").toDouble());
		ui.sbTime1->setValue(settings.value("Time1/val").toDouble());
		ui.sbTimeEps->isVisible() && ui.sbTimeEps->text() == EpsConst ? ui.sbTimeEps->setValue(settings.value("Eps/val").toDouble()) : ui.sbTimeEps->setValue(settings.value("Time2/val").toDouble()) ;
		settings.sync();
		this->raise();
		this->activateWindow();
	}

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
		ui.lb_TimeEps->setText(TimeConst2);
		ui.lb_TimeEps->setVisible(true);
		ui.sbTimeEps->setVisible(true);
	}
	else if(index == 2)
	{
		ui.lb_TimeEps->setText(EpsConst);
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
		gr1_textError->setVisible(true);
		gr2_textError->setVisible(true);
		QMessageBox::critical(this,QTextCodec::codecForLocale()->toUnicode( "ОШИБКА СВЯЗИ"), QTextCodec::codecForLocale()->toUnicode( "Ошибка устройства ввода/вывода" ));
	}
}

void testPlot::setRangeOver(QCPRange newRange, QCPRange oldRange)
{
	if(zoom)
	{
		QCPAxis *r = dynamic_cast<QCPAxis *>(sender());
		if(!r)
			return;
		
		QString s = r->property("name").toString();
		
		QCPAxisRect * rect=0;
		if(s == "Graph1")
		{
			rect = gr1_wideAxisRect;
		}
		else if(s =="Graph2" )
		{
			rect = gr2_wideAxisRect;
		}

		if(rect == NULL)
			return; 

		QCPRange y2 =  rect->axis(QCPAxis::atLeft,1)->range();
		double ypos = ui.plot_wid->mapFromGlobal(QCursor::pos()).y();
		double y =   rect->axis(QCPAxis::atLeft)->pixelToCoord(ypos);
		double delta1 = newRange.upper -  newRange.lower;
		double delta2 = y2.upper -  y2.lower;
		double delta3 = oldRange.upper - oldRange.lower;
		double delta = delta1*delta2/delta3;
		double dy = (delta2 - delta);
		double new_y2_upper = 0;
		double new_y2_lower = 0;
		if(dy < 0.0000001)
		{
			double delta3_1 = oldRange.upper-  newRange.upper;
			double delta4_1 = oldRange.lower-  newRange.lower;
			new_y2_upper =  -delta2*delta3_1/delta1 +y2.upper;
			new_y2_lower =  -delta2*delta4_1/delta1 +y2.lower;
		}
		else
		{
			double k = qAbs(y-newRange.upper) / delta1 ;
			new_y2_upper = y2.upper - k*dy;
			new_y2_lower = y2.lower + (1-k)*dy;
		}

		 rect->axis(QCPAxis::atLeft,1)->setRange(new_y2_upper,new_y2_lower);
	}
}



void testPlot::setupRealtimeData(QCustomPlot *customPlot,QString lax_lab1,QString lax_lab2,QCPAxisRect * Rect,QCPItemText * textLabel )
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
	QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif

	// configure axis rect:
	customPlot->plotLayout()->clear(); // clear default axis rect so we can start from scratch

	Rect->setupFullAxesBox(true);																																																																																																																								
	Rect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::blue);
	Rect->axis(QCPAxis::atLeft)->setLabel(lax_lab1);
	Rect->addAxis(QCPAxis::atLeft)->setTickLabelColor(Qt::red); // add an extra axis on the left and color its numbers

	customPlot->plotLayout()->addElement(0, 0,Rect); // insert axis rect in first row
	Rect->axis(QCPAxis::atLeft, 1)->setRangeLower(0);
	Rect->axis(QCPAxis::atLeft,1)->setLabel(lax_lab2);

	QCPGraph *mainGraph1 = customPlot->addGraph(Rect->axis(QCPAxis::atBottom), Rect->axis(QCPAxis::atLeft));
	mainGraph1->setPen(QPen(Qt::blue, 2));
	mainGraph1->setBrush(QColor(255, 161, 0, 50));

	QCPGraph *mainGraph2 = customPlot->addGraph(Rect->axis(QCPAxis::atBottom), Rect->axis(QCPAxis::atLeft, 1));
	mainGraph2->setPen(QPen(Qt::red, 2));
	mainGraph2->setBrush(QColor(110, 170, 110, 30));

	customPlot->graph(0)->setAntialiasedFill(false);
	customPlot->graph(1)->setAntialiasedFill(false);

	connect(Rect->axis(QCPAxis::atLeft), SIGNAL(rangeChanged(QCPRange,QCPRange)), this, SLOT(setRangeOver(QCPRange,QCPRange)));

	// add the text label at the top:
	textLabel = new QCPItemText(customPlot);
	customPlot->addItem(textLabel);
	textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
	textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
	textLabel->position->setCoords(0.5, 0.1); // place position at center/top of axis rect
	textLabel->setText(QTextCodec::codecForLocale()->toUnicode( "Ошибка устройства ввода/вывода.Проверьте вставлено ли устройство в порт." ));
	textLabel->setFont(QFont(font().family(), 12)); // make font a bit larger
	textLabel->setPen(QPen(Qt::black)); // show black border around text
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
		ui.plot_wid->graph(1)->addData(x_value, Nu_code);
		ui.plot_wid_2->graph(0)->addData(x_value, PVn);
		ui.plot_wid_2->graph(1)->addData(x_value, OPn);

		if(!zoom)
		{
			ui.plot_wid->graph(0)->rescaleValueAxis();
			ui.plot_wid->graph(1)->rescaleValueAxis();
			ui.plot_wid_2->graph(0)->rescaleValueAxis();
			ui.plot_wid_2->graph(1)->rescaleValueAxis();
		}

		lastPointKey = x_value;
		x_value += 0.1;
	}

	if(!zoom)
	{
		if(x_value<ui.graph_range->value())
		{
			gr1_wideAxisRect->axis(QCPAxis::atBottom,0)->setRange(0,x_value);
			gr2_wideAxisRect->axis(QCPAxis::atBottom,0)->setRange(0,x_value);
		}
		else
		{
			double val = x_value - ui.graph_range->value();
			gr1_wideAxisRect->axis(QCPAxis::atBottom,0)->setRange(val,x_value);
			gr2_wideAxisRect->axis(QCPAxis::atBottom,0)->setRange(val,x_value);
		}
		ui.plot_wid->replot();
		ui.plot_wid_2->replot();
	}
}

void testPlot::timeout_one_second()
{
	try {
		Nu_code = readFromDevice();
		writeToDevice(Nx_code);
		statusBar()->clearMessage();
		gr1_textError->setVisible(false);
		gr2_textError->setVisible(false);
		Nx_string = QString::number(Nx_code);
		Nu_string = QString::number(Nu_code);

		OPn = 100.0 * Nu_code / 1023.0;

		double t1 = ui.sbTime1->value();
		double t2 = 0;
		double eps =0;
		ui.sbTimeEps->text() == TimeConst2 && ui.sbTimeEps->isVisible() ?t2 = ui.sbTimeEps->value(),eps=0 : t2 = 0,eps = ui.sbTimeEps->value();
		double g = ui.sbGain->value();
		double d = ui.sbDelay->value();
		transfer_function.setData(t1,t2,g,d);

		PVn = transfer_function(time_step, OPn);
		Nx_code = (PVn / 100.0) * 1023; 

		PVstring = QString::number(PVn);
		OPstring = QString::number(OPn);
	}
	catch (std::runtime_error& e) {
		if (io_delay > 5) {
			Nx_string = Nu_string = PVstring = OPstring = "IO error";
			statusBar()->showMessage(QTextCodec::codecForLocale()->toUnicode( "Ошибка устройства ввода/вывода.Проверьте вставлено ли устройство в порт." ));
			gr1_textError->setVisible(true);
			gr2_textError->setVisible(true);
		}
		++ io_delay;
	}
	static int filter = 0;
	if (filter == 0) {
		ui.progressBar->setValue(Nu_code);
		ui.lbNx->setText(Nx_string);
		ui.lbNu->setText(Nu_string);
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
	io_delay = 0;
	char buffer[255];
	int nBytes = usb_control_msg(handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
		USB_DATA_OUT, 0, 0, (char *)buffer, sizeof(buffer), 5000);

	unsigned raw_data = *((unsigned *) buffer);
	double adc_data = 0x0000FFFF & raw_data; // АЦП в Tiny 10-разрядный
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

