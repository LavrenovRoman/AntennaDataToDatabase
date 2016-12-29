#include "selectantenna.h"
#include "Antenna.h"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectAntenna::SelectAntenna(SelectAll* pCoreData, QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);

	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ExpChanged(int))); 
	connect(ui.listWidget_2, SIGNAL(currentRowChanged(int)), this, SLOT(AntChanged(int)));
	connect(ui.okButton, SIGNAL(clicked()), this, SIGNAL(AntennaOk()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SIGNAL(Cancel()));
	pkCoreData = pCoreData;
	Reset();
}

SelectAntenna::~SelectAntenna()
{

}

void SelectAntenna::Reset()
{
	ui.okButton->setEnabled(false);
	ui.listWidget->clear();
	for (int i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		ui.listWidget->insertItem(i, QString::number(pkCoreData->GetExpsID()->at(i)));
	}
	ui.listWidget_2->clear();
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	IdExperiment = 0;
	repaint();
}

void SelectAntenna::ExpChanged(int expChange)
{
	if (expChange >= 0)
	{
		ui.okButton->setEnabled(false);

		QString date = QString::number(pkCoreData->GetExps()->at(expChange).date.tm_mday) + "." + QString::number(pkCoreData->GetExps()->at(expChange).date.tm_mon) + "." + QString::number(pkCoreData->GetExps()->at(expChange).date.tm_year);
		ui.leDate->setText(date);

		QString cicles;
		for (size_t i = 0; i<pkCoreData->GetExps()->at(expChange).cycles.size(); ++i)
		{
			cicles += QString::fromStdString(pkCoreData->GetExps()->at(expChange).cycles[i].name) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pBegin) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pEnd) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pStep);
			if (i != pkCoreData->GetExps()->at(expChange).cycles.size() - 1)	cicles += "   ";
		}
		ui.leCicles->setText(cicles);

		ui.leComm->setText(QString::fromLocal8Bit(pkCoreData->GetExps()->at(expChange).comment.data()));

		IdExperiment = pkCoreData->GetExpsID()->at(expChange);

		ui.listWidget_2->clear();
		for (int i = 0; i<pkCoreData->GetAntsID()->at(expChange).size(); ++i)
		{
			ui.listWidget_2->insertItem(i, QString::number(pkCoreData->GetAntsID()->at(expChange).at(i)));
		}
	}
}

void SelectAntenna::AntChanged(int antChange)
{
	if (antChange >= 0)
	{
		ui.okButton->setEnabled(true);

		int expChange = std::find(pkCoreData->GetExpsID()->begin(), pkCoreData->GetExpsID()->end(), IdExperiment) - pkCoreData->GetExpsID()->begin();

		QString cicles;
		for (size_t i = 0; i<pkCoreData->GetExps()->at(expChange).cycles.size(); ++i)
		{
			if (pkCoreData->GetExps()->at(expChange).cycles[i].name == "L[0]")
				cicles += QString::fromStdString(pkCoreData->GetExps()->at(expChange).cycles[i].name) + " " + QString::number(pkCoreData->GetAnts()->at(expChange).at(antChange).inputPar.Radiator.ScaleX);
			if (i != pkCoreData->GetExps()->at(expChange).cycles.size() - 1)	cicles += "   ";
		}
		ui.leCicles_2->setText(cicles);

		IdAntenna = pkCoreData->GetAntsID()->at(expChange).at(antChange);
	}
}