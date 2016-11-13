#ifndef ANTENNADATATODATABASE_H
#define ANTENNADATATODATABASE_H

#include <QtWidgets/QMainWindow>
#include "ui_antennadatatodatabase.h"

#include "Core.h"

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

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
