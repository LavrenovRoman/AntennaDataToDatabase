#ifndef SELECTANTENNA_H
#define SELECTANTENNA_H

#include <QtWidgets/QMainWindow>
#include "ui_selectantenna.h"
#include "selectall.h"

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
	SelectAll * pkCoreData = nullptr;
	Ui::DialogAntenna ui;

	private slots:
	void ExpChanged(int expChange);
	void AntChanged(int antChange);

};

#endif // SELECTANTENNA_H