#ifndef SELECTEXPERIMENT_H
#define SELECTEXPERIMENT_H

#include <QtWidgets/QMainWindow>
#include "ui_selectexperiment.h"
#include "selectall.h"

class Experiment;

class SelectExperiment : public QDialog
{
	Q_OBJECT

public:
	SelectExperiment(SelectAll* pCoreData, QWidget *parent = 0);
	~SelectExperiment();
	void Reset();
	int IdExperiment;

signals:
  void ExperimentOk();
  void Cancel();

private:
	SelectAll * pkCoreData = nullptr;
	Ui::DialogExperiment ui;

private slots:
	void ExpChanged(int expChange);

};

#endif // SELECTEXPERIMENT_H