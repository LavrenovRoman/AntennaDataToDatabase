#ifndef ANTENNADATAVIEWER_H
#define ANTENNADATAVIEWER_H

#include <QtWidgets/QMainWindow>
#include "ui_antennadataviewer.h"
#include <QStringList>
#include "selectexperiment.h"
#include "selectantenna.h"
#include "selectedantennas.h"

struct SelectedArea
{
	SelectedArea()
	{
		Reset();
	}
	void Reset()
	{
		Corner1X = -1;
		Corner1Y = -1;
		Corner2X = -1;
		Corner2Y = -1;
	}
	bool IsInside(int x, int y)
	{
		if (Corner1X == Corner2X || Corner1Y == Corner2Y) return false;
		if (Corner1X < Corner2X && (x<Corner1X || x>Corner2X)) return false;
		if (Corner1X > Corner2X && (x>Corner1X || x<Corner2X)) return false;
		if (Corner1Y < Corner2Y && (y<Corner1Y || y>Corner2Y)) return false;
		if (Corner1Y > Corner2Y && (y>Corner1Y || y<Corner2Y)) return false;
		return true;
	}
	bool Empty()
	{
		if (Corner1X == -1 || Corner1Y == -1 ) return true;
		return false;
	}
	int Corner1X;
	int Corner1Y;
	int Corner2X;
	int Corner2Y;
};

class AntennaDataViewer : public QMainWindow
{
	Q_OBJECT

public:
	AntennaDataViewer(QWidget *parent = 0);
	~AntennaDataViewer();

private:
	Ui::AntennaDataViewerClass ui;
	Core core;
	SelectExperiment * pSelEx;
	SelectAntenna * pSelAnt;
	SelectedAntennas * pSelExAnt;
	SelectAll *pSelAll;
	QStringList inputPars, outputPars, selectPars;
	std::vector<std::vector<double>> res;
	QString xGraphTitle, yGraphTitle; 
	int IdExperiment;
	int IdAntenna;
	bool parInsideAntenna;
	std::vector<int> selectedPoints;
	std::vector<int> selectedPars;
	std::vector<ViewDataExp> viewDataExp;
	SelectedArea selectedArea;

private slots:
	void ClickedCalcCorr();
	void CreateGraph();
	void DBRowChanged(QListWidgetItem* pSelectRow);
	void AntennaOk();
	void ExperimentOk();
	void ExperimentCancel();
	void SelectExpAntOk();
	void CreateLists();
	void PlotMouseMove(QMouseEvent *event);
	void PlotMousePress(QMouseEvent *event);
	void PlotMouseRelease(QMouseEvent *event);
	void SortResult(int l, int r);
	void SortResult();
};

#endif // ANTENNADATAVIEWER_H
