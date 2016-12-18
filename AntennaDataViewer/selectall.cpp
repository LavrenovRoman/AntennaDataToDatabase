#include "selectall.h"
#include "Antenna.h"
#include "thread"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectAll::SelectAll(Core* pCore)
{
	pkCore = pCore;
	//pWD = new Wait_Download();
	//pWD->moveToThread(QApplication::instance()->thread());
	//pWD->show();
	//pWD->setVisible(true);
	//pWD->update();
	//pWD->repaint();
	progress = 0;
	std::thread reset_func(&SelectAll::ResetSelectAll, this);
	reset_func.join();

	//QtConcurrent::run(&text1, &TestClass::run);
	//std::thread view_func(&SelectAll::ViewProgress, this);
	//view_func.detach();

	//while (progress!=100)
	//{
	//	pWD->setVisible(true);
	//	pWD->WaitChanged(progress);
	//	pWD->update();
	//	pWD->repaint();
	//}
}

void SelectAll::ViewProgress()
{
	Wait_Download WD = new Wait_Download();
	WD.moveToThread(QApplication::instance()->thread());
	WD.setVisible(true);
	WD.update();
	while (progress != 100)
	{
		WD.setVisible(true);
		WD.WaitChanged(progress);
		WD.update();
	}
}

SelectAll::~SelectAll()
{
	if (pWD != nullptr)
		delete pWD;
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
		progress = 100*(i+1)/ids.size();
	}
}

std::vector<int> * SelectAll::GetExpsID() { return &ids; }
std::vector<std::vector<int>> * SelectAll::GetAntsID() { return &antsids; }
std::vector<Experiment> * SelectAll::GetExps() { return &exps; }
std::vector<std::vector<Antenna>> * SelectAll::GetAnts() { return &ants; }