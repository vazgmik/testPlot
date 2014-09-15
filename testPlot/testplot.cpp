#include "testplot.h"


testPlot::testPlot(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.pbStart,SIGNAL(clicked()),SLOT(on_bStart_clicked()));
	setupRealtimeDataDemo(ui.plot_wid1);
	setWindowTitle("QCustomPlot: "+demoName);
	statusBar()->clearMessage();
	ui.plot_wid1->replot();
}

testPlot::~testPlot()
{

}

void testPlot::on_bStart_clicked()
{
	/*QVector<double> x(101), y(101); // initialize with entries 0..100
	for (int i=0; i<101; ++i)
	{
		x[i] = i/50.0 - 1; // x goes from -1 to 1
		y[i] = x[i]*x[i]; // let's plot a quadratic function
	}
	// create graph and assign data to it:
	ui.plot_wid1->addGraph();
	ui.plot_wid1->graph(0)->setData(x, y);
	// give the axes some labels:
	ui.plot_wid1->xAxis->setLabel("x");
	ui.plot_wid1->yAxis->setLabel("y");
	ui.plot_wid2->xAxis->setLabel("x");
	ui.plot_wid2->yAxis->setLabel("y");
	// set axes ranges, so we see all data:
	ui.plot_wid1->xAxis->setRange(-1, 1);
	ui.plot_wid1->yAxis->setRange(0, 1);
	ui.plot_wid1->replot();
	ui.plot_wid2->replot();
	
	setupRealtimeDataDemo(ui.plot_wid1);
	setWindowTitle("QCustomPlot: "+demoName);
	statusBar()->clearMessage();

	ui.plot_wid1->replot();*/
}


void testPlot::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif
  demoName = "Real Time Data Demo";
  
  // include this section to fully disable antialiasing for higher performance:
/*  
  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->legend->setFont(font);
  */
  customPlot->addGraph(); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
  customPlot->graph(0)->setAntialiasedFill(false);
  customPlot->addGraph(); // red line
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));
  
  customPlot->addGraph(); // blue dot
  customPlot->graph(2)->setPen(QPen(Qt::blue));
  customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot->addGraph(); // red dot
  customPlot->graph(3)->setPen(QPen(Qt::red));
  customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);
  
  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(2);
  customPlot->axisRect()->setupFullAxesBox();
  
  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
  
  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
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
  if (key-lastPointKey > 0.001) // at most add point every 10 ms
  {
    double value0 = qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
    double value1 = qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
    // add data to lines:
    ui.plot_wid1->graph(0)->addData(key, value0);
    ui.plot_wid1->graph(1)->addData(key, value1);
    // set data of dots:
    ui.plot_wid1->graph(2)->clearData();
    ui.plot_wid1->graph(2)->addData(key, value0);
    ui.plot_wid1->graph(3)->clearData();
    ui.plot_wid1->graph(3)->addData(key, value1);
    // remove data of lines that's outside visible range:
    ui.plot_wid1->graph(0)->removeDataBefore(key-8);
    ui.plot_wid1->graph(1)->removeDataBefore(key-8);
    // rescale value (vertical) axis to fit the current data:
    ui.plot_wid1->graph(0)->rescaleValueAxis();
    ui.plot_wid1->graph(1)->rescaleValueAxis(true);
    lastPointKey = key;
  }
  // make key axis range scroll with the data (at a constant range size of 8):
  ui.plot_wid1->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
  ui.plot_wid1->replot();
  
  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    statusBar()->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui.plot_wid1->graph(0)->data()->count()+ui.plot_wid1->graph(1)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}

