#ifndef DELETEEXPERIMENT_H
#define DELETEEXPERIMENT_H

#include <QtWidgets/QMainWindow>
#include "ui_deleteexperiment.h"
#include "selectall.h"

class DeleteExperiment : public QDialog
{
	Q_OBJECT

public:
	DeleteExperiment(SelectAll* pCoreData, QWidget *parent = 0);
	~DeleteExperiment();
	void Reset();
	int IdExperiment;

signals:
  void DeleteExperimentOk();
  void Cancel();

private:
	SelectAll * pkCoreData = nullptr;
	Ui::DialogDelExperiment ui;

private slots:
	void DeleteExp();
	void ExpChanged(int expChange);

};

#endif // DELETEEXPERIMENT_H