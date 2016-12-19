#include "selectall.h"
#include "Antenna.h"
#include "thread"
#include <QApplication>

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectAll::SelectAll(Core* pCore, QWidget *parent) : QObject(parent)
{
	pkCore = pCore;

	progress = 0;
	pprd = new QProgressDialog(QString::fromLocal8Bit("Загрузка данных из БД"), QString::fromLocal8Bit("Отмена"), 0, 100, parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	pprd->setWindowTitle(QString::fromLocal8Bit("Подождите"));
	pprd->setWindowModality(Qt::WindowModal);
	pprd->setModal(true);
	pprd->setMinimumDuration(1);
	pprd->setValue(progress.load());
	QApplication::processEvents(QEventLoop::AllEvents);
	QObject::connect(pprd, SIGNAL(canceled()), this, SLOT(Cancel()));
	pprd->show();

	std::thread reset_func(&SelectAll::ResetSelectAll, this);

	while (progress.load() < 100)
	{
		if (pprd->wasCanceled())
		{
			Cancel();
		}
		QApplication::processEvents(QEventLoop::AllEvents);
		if (pprd->value() == progress.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}
		//qDebug() << progress.load();
		pprd->setValue(progress.load());
		pprd->update();
	}
	pprd->setValue(progress.load());
	reset_func.join();

	delete pprd;
	pprd = nullptr;
}

void SelectAll::Cancel()
{
	exit(0);
}

SelectAll::~SelectAll()
{
	if (pprd != nullptr)
	{
		delete pprd;
		pprd = nullptr;
	}
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