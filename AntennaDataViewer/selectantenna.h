#ifndef SELECTANTENNA_H
#define SELECTANTENNA_H

#include <QtWidgets/QMainWindow>
#include "ui_selectantenna.h"
#include "selectall.h"

class Experiment;
class Antenna;

class SelectAntenna : public QDialog
{
	Q_OBJECT

public:
	SelectAntenna(SelectAll* pCoreData, QWidget *parent = 0);
	~SelectAntenna();
	void Reset();
	int IdExperiment; 
	int IdAntenna;

signals:
	void AntennaOk();
	void Cancel();

private:
	SelectAll * pkCoreData;
	Ui::DialogAntenna ui;
	//std::vector<int> ids;
	//std::vector<int> antsids;
	//std::vector<Experiment> exps;
	//std::vector<Antenna> ants; 

	private slots:
	void ExpChanged(int expChange);
	void AntChanged(int antChange);

};

#endif // SELECTANTENNA_H