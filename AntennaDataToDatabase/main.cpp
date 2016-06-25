#include "antennadatatodatabase.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AntennaDataToDatabase w;
	w.show();
	return a.exec();
}
