#include "selectall.h"
#include "Antenna.h"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectAll::SelectAll(Core* pCore)
{
	pkCore = pCore;
	ResetSelectAll();
}

void SelectAll::ResetSelectAll()
{
	pkCore->GetExperiments(ids, exps);
	for (int i = 0; i < ids.size(); ++i)
	{
		std::vector<int> tempids;
		antsids.push_back(tempids);
		std::vector<Antenna> tempants;
		ants.push_back(tempants);
		pkCore->GetAntennasByExperiment(ants[i], antsids[i], ids[i]);
	}
}

std::vector<int> * SelectAll::GetExpsID() { return &ids; }
std::vector<std::vector<int>> * SelectAll::GetAntsID() { return &antsids; }
std::vector<Experiment> * SelectAll::GetExps() { return &exps; }
std::vector<std::vector<Antenna>> * SelectAll::GetAnts() { return &ants; }