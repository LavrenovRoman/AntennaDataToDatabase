#ifndef ANTENNADATATODATABASE_H
#define ANTENNADATATODATABASE_H

#include <QtWidgets/QMainWindow>
#include "ui_antennadatatodatabase.h"
#include "ParseFekoFile.h"
#include "FrbrdDatabase.h"

class AntennaDataToDatabase : public QMainWindow
{
	Q_OBJECT

public:
	AntennaDataToDatabase(QWidget *parent = 0);
	~AntennaDataToDatabase();

private:
	Ui::AntennaDataToDatabaseClass ui;
	QStringList outs, pres;
	QStringList out_names, pre_names;
	std::vector<Antenna> antennas;
	FrbrdDatabase FBDataBase;

private slots:
	void ClickedOpenDir();
	void ClickedOpenOutFiles();
};

#endif // ANTENNADATATODATABASE_H
