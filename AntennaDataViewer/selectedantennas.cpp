#include "selectedantennas.h"
#include "Antenna.h"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectedAntennas::SelectedAntennas(SelectAll* pCoreData, QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);

	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ExpAntChanged(int)));
	connect(ui.okButton, SIGNAL(clicked()), this, SIGNAL(SelectExpAntOk()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(Cancel()));
	pkCoreData = pCoreData;
}

SelectedAntennas::~SelectedAntennas()
{

}

void SelectedAntennas::Clear()
{
	selectedDataExpAnt.clear();
}

void SelectedAntennas::Cancel()
{
	Clear();
}

bool SelectedAntennas::HaveAntenna(int ant)
{
	for (int i = 0; i < selectedDataExpAnt.size(); ++i)
	{
		if (selectedDataExpAnt[i].IdAntenna == ant) return true;
	}
	return false;
}

std::vector<ViewDataExp> * SelectedAntennas::GetDataSelectedExpAnt()
{
	return &selectedDataExpAnt;
}

void SelectedAntennas::Reset(std::vector<ViewDataExp> _selectedDataExpAnt)
{
	selectedDataExpAnt = _selectedDataExpAnt;

	SortResult(0, selectedDataExpAnt.size() - 1);
	ui.okButton->setEnabled(true);
	ui.listWidget->clear();

	selectedExpId.clear();
	selectedAntId.clear();

	int s = 0;
	for (int i = 0; i < pkCoreData->GetExpsID()->size(); ++i)
	{
		while (s < selectedDataExpAnt.size() && pkCoreData->GetExpsID()->at(i) == selectedDataExpAnt[s].IdExperiment)
		{
			selectedExpId.push_back(i);
			for (int j = 0; j < pkCoreData->GetAntsID()->at(i).size(); ++j)
			{
				if (pkCoreData->GetAntsID()->at(i).at(j) == selectedDataExpAnt[s].IdAntenna)
				{
					selectedAntId.push_back(j);
				}
			}
			s++;
		}
	}

	for (int i = 0; i<selectedDataExpAnt.size(); ++i)
	{
		QString txt = QString::fromLocal8Bit("Эксперимент: ") + QString::number(selectedDataExpAnt[i].IdExperiment) + QString::fromLocal8Bit(" Антенна: ") + QString::number(selectedDataExpAnt[i].IdAntenna);
		ui.listWidget->insertItem(i, txt);
	}
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	ui.leCicles_2->clear();
}

void SelectedAntennas::SortResult(int l, int r)
{
	int x = selectedDataExpAnt[l + (r - l) / 2].IdAntenna;
	int i = l;
	int j = r;
	while (i <= j)
	{
		while (selectedDataExpAnt[i].IdAntenna < x) i++;
		while (selectedDataExpAnt[j].IdAntenna > x) j--;
		if (i <= j)
		{
			swap(selectedDataExpAnt[i], selectedDataExpAnt[j]);
			i++;
			j--;
		}
	}
	if (i<r)
		SortResult(i, r);

	if (l<j)
		SortResult(l, j);
}

void SelectedAntennas::ExpAntChanged(int expantChange)
{
	if (expantChange >= 0)
	{		
		QString date = QString::number(pkCoreData->GetExps()->at(selectedExpId[expantChange]).date.tm_mday) + "." + QString::number(pkCoreData->GetExps()->at(selectedExpId[expantChange]).date.tm_mon) + "." + QString::number(pkCoreData->GetExps()->at(selectedExpId[expantChange]).date.tm_year);
		ui.leDate->setText(date);

		QString cicles;
		for (size_t i = 0; i<pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles.size(); ++i)
		{
			cicles += QString::fromStdString(pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles[i].name) + " " + QString::number(pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles[i].pBegin) + " " + QString::number(pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles[i].pEnd) + " " + QString::number(pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles[i].pStep);
			cicles += "   ";
		}
		ui.leCicles->setText(cicles);

		ui.leComm->setText(QString::fromLocal8Bit(pkCoreData->GetExps()->at(selectedExpId[expantChange]).comment.data()));

		QString ciclesA;
		for (size_t i = 0; i<pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles.size(); ++i)
		{
			if (pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles[i].name == "L[0]")
				ciclesA += QString::fromStdString(pkCoreData->GetExps()->at(selectedExpId[expantChange]).cycles[i].name) + " " + QString::number(pkCoreData->GetAnts()->at(selectedExpId[expantChange]).at(selectedAntId[expantChange]).inputPar.Radiator.ScaleX);
			ciclesA += "   ";
		}
		ui.leCicles_2->setText(ciclesA);
	}
}