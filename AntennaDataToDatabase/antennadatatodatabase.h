#ifndef ANTENNADATATODATABASE_H
#define ANTENNADATATODATABASE_H

#include <QtWidgets/QMainWindow>
#include "ui_antennadatatodatabase.h"
#include "Core.h"

class AntennaDataToDatabase : public QMainWindow
{
	Q_OBJECT

public:
	AntennaDataToDatabase(QWidget *parent = 0);
	~AntennaDataToDatabase();

private:
	Ui::AntennaDataToDatabaseClass ui;
	Core core;

private slots:
	void ClickedOpenDir();
	void ClickedOpenOutFiles();
};

#endif // ANTENNADATATODATABASE_H
