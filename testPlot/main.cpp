#include "testplot.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	testPlot w;
	w.show();
	return a.exec();
}
