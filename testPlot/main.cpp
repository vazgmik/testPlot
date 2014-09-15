#include "testplot.h"
#include <QtWidgets/QApplication>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	testPlot w;
	w.show();
	return a.exec();
}
