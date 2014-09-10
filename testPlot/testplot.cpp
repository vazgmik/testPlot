#include "testplot.h"


testPlot::testPlot(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.pbStart,SIGNAL(clicked()),SLOT(on_bStart_clicked()));
}

testPlot::~testPlot()
{

}

void testPlot::on_bStart_clicked()
{
	QVector<double> x(101), y(101); // initialize with entries 0..100
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
}