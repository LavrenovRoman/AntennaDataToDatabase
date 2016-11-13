#ifndef SELECTEDANTENNAS_H
#define SELECTEDANTENNAS_H

#include <QtWidgets/QMainWindow>
#include "ui_selectedantennas.h"
#include "selectall.h"

class Experiment;
class Antenna;

class SelectedAntennas : public QDialog
{
	Q_OBJECT

public:
	SelectedAntennas(SelectAll* pCoreData, QWidget *parent = 0);
	~SelectedAntennas();
	void Clear();
	void Reset(std::vector<ViewDataExp> _selectedDataExpAnt);
	std::vector<ViewDataExp> * GetDataSelectedExpAnt();
	bool HaveAntenna(int ant);

signals:
	void SelectExpAntOk();

private:
	SelectAll * pkCoreData;
	Ui::DialogSelAntennas ui;
	std::vector<ViewDataExp> selectedDataExpAnt;
	std::vector<int> selectedExpId, selectedAntId;

	void SortResult(int l, int r);

	private slots:
	void ExpAntChanged(int expantChange);
	void Cancel();

};

#endif // SELECTEDANTENNAS_H