#ifndef SELECTEXPERIMENTS_H
#define SELECTEXPERIMENTS_H

#include <QtWidgets/QMainWindow>
#include "ui_selectexperiments.h"
#include "selectall.h"

class Experiment;

class SelectExperiments : public QDialog
{
	Q_OBJECT

public:
	SelectExperiments(SelectAll* pCoreData, QWidget *parent = 0);
	~SelectExperiments();
	void Reset();
	std::vector<int> IdsExperiment;

signals:
  void ExperimentsOk();
  void Cancel();

private:
	SelectAll * pkCoreData;
	Ui::DialogExperiments ui;

private slots:
	void ExpChanged(int expChange);
	void Filter();

};

#endif // SELECTEXPERIMENTS_H