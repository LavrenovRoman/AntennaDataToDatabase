#include "SelectExperiments.h"
#include "Antenna.h"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectExperiments::SelectExperiments(SelectAll* pCoreData, QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);

	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ExpChanged(int)));
	connect(ui.okButton, SIGNAL(clicked()), this, SIGNAL(ExperimentsOk()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SIGNAL(Cancel()));
	connect(ui.dateEditFrom, SIGNAL(dateChanged(QDate)), this, SLOT(Filter()));
	connect(ui.dateEditBefore, SIGNAL(dateChanged(QDate)), this, SLOT(Filter()));
	connect(ui.leCommFilter, SIGNAL(textEdited(QString)), this, SLOT(Filter()));
	pkCoreData = pCoreData;
	ui.okButton->setEnabled(true);
	Reset();
}

SelectExperiments::~SelectExperiments()
{

}

void SelectExperiments::Reset()
{
	ui.listWidget->clear();
	IdsExperiment.clear();
	for (size_t i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		ui.listWidget->insertItem(i, QString::number(pkCoreData->GetExpsID()->at(i)));
		IdsExperiment.push_back(pkCoreData->GetExpsID()->at(i));
	}
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	repaint();
}

void SelectExperiments::Filter()
{
	QDate dataBegin = ui.dateEditFrom->date();
	QDate dataEnd = ui.dateEditBefore->date();
	QString comm = ui.leCommFilter->text();
	std::string s_comm = comm.toLocal8Bit().constData();

	ui.listWidget->clear();
	IdsExperiment.clear();

	for (size_t i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		std::string s_commEx = pkCoreData->GetExps()->at(i).comment.data();

		size_t pos = std::string::npos;
		if (!s_comm.empty())
		{
			pos = s_commEx.find(s_comm);
		}
		
		QDate dataEx(pkCoreData->GetExps()->at(i).date.tm_year, pkCoreData->GetExps()->at(i).date.tm_mon, pkCoreData->GetExps()->at(i).date.tm_mday);
		if (dataEx >= dataBegin && dataEx <= dataEnd && (s_comm.empty() || pos != std::string::npos))
		{
			ui.listWidget->insertItem(i, QString::number(pkCoreData->GetExpsID()->at(i)));
			IdsExperiment.push_back(pkCoreData->GetExpsID()->at(i));
		}
	}
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	repaint();
}

void SelectExperiments::ExpChanged(int expChange)
{
	if (expChange >= 0)
	{
		Experiment & exp = pkCoreData->GetExps()->at(expChange);

		QString date = QString::number(exp.date.tm_mday) + "." + QString::number(exp.date.tm_mon) + "." + QString::number(exp.date.tm_year);
		ui.leDate->setText(date);

		QString cicles;
		for (size_t i = 0; i<exp.cycles.size(); ++i)
		{
			Experiment_Param &expp = exp.cycles[i];
			cicles += QString::fromStdString(expp.name) + " " + QString::number(expp.pBegin) + " " + QString::number(expp.pEnd) + " " + QString::number(expp.pStep);
			if (i != exp.cycles.size() - 1)	cicles += "   ";
		}
		ui.leCicles->setText(cicles);

		ui.leComm->setText(QString::fromLocal8Bit(exp.comment.data()));
	}
}