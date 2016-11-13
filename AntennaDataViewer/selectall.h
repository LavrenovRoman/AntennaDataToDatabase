#ifndef SELECTALL_H
#define SELECTALL_H

#include "Core.h"
#include <QStringList>

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
	Core * pkCore;
};

#endif // SELECTALL_H