#ifndef SELECTALL_H
#define SELECTALL_H

#include "Core.h"
#include <QStringList>
#include "wait_download.h"
#include "atomic"

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

class SelectAll
{

public:
	SelectAll(Core* pCore);
	~SelectAll();
	void ResetSelectAll();
	void ViewProgress();

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
	Wait_Download * pWD = nullptr;
	std::atomic<int> progress;
};

#endif // SELECTALL_H