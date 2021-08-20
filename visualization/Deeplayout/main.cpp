#include "deeplayout.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Deeplayout w;
	w.showMaximized();
	return a.exec();
}
