#ifndef SELECTALL_H
#define SELECTALL_H

#include "Core.h"
#include <QProgressDialog>
#include <QStringList>
#include <QObject>
#include <atomic>

class Antenna;
class Experiment;

struct ViewDataExp
{
	ViewDataExp(int _IdExperiment, int _IdAntenna)
	{
		IdExperiment = _IdExperiment;
		IdAntenna = _IdAntenna;
	}
	int IdExperiment;
	int IdAntenna;
};

class SelectAll : public QObject
{
	Q_OBJECT

public:
	SelectAll(Core* pCore, QWidget *parent = 0);
	~SelectAll();
	void ResetSelectAll();

	std::vector<int> * GetExpsID();
	std::vector<std::vector<int>> * GetAntsID();
	std::vector<Experiment> * GetExps();
	std::vector<std::vector<Antenna>> * GetAnts();

protected:
	std::vector<int> ids;
	std::vector<std::vector<int>> antsids;
	std::vector<Experiment> exps;
	std::vector<std::vector<Antenna>> ants;

private:
	Core * pkCore = nullptr;
	QProgressDialog* pprd = nullptr;
	std::atomic<int> progress;
	void Cancel();
};

#endif // SELECTALL_H