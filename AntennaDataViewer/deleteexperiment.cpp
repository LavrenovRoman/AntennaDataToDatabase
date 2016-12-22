#include "deleteexperiment.h"
#include "Antenna.h"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

DeleteExperiment::DeleteExperiment(SelectAll* pCoreData, QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);

	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ExpChanged(int)));
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(DeleteExp()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SIGNAL(Cancel()));
	pkCoreData = pCoreData;
	Reset();
}

DeleteExperiment::~DeleteExperiment()
{

}

void DeleteExperiment::Reset()
{
	ui.okButton->setEnabled(false);
	ui.listWidget->clear();
	for (int i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		ui.listWidget->insertItem(i, QString::number(pkCoreData->GetExpsID()->at(i)));
	}
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	IdExperiment = 0;
	repaint();
}

void DeleteExperiment::ExpChanged(int expChange)
{
	ui.okButton->setEnabled(false);
	if (expChange >= 0)
	{
		QString date = QString::number(pkCoreData->GetExps()->at(expChange).date.tm_mday) + "." + QString::number(pkCoreData->GetExps()->at(expChange).date.tm_mon) + "." + QString::number(pkCoreData->GetExps()->at(expChange).date.tm_year);
		ui.leDate->setText(date);

		QString cicles;
		for (size_t i = 0; i<pkCoreData->GetExps()->at(expChange).cycles.size(); ++i)
		{
			cicles += QString::fromStdString(pkCoreData->GetExps()->at(expChange).cycles[i].name) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pBegin) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pEnd) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pStep);
			if (i != pkCoreData->GetExps()->at(expChange).cycles.size()-1)	cicles += "   ";
		}
		ui.leCicles->setText(cicles);

		ui.leComm->setText(QString::fromLocal8Bit(pkCoreData->GetExps()->at(expChange).comment.data()));

		IdExperiment = pkCoreData->GetExpsID()->at(expChange);
		ui.okButton->setEnabled(true);
	}
}

void DeleteExperiment::DeleteExp()
{
	pkCoreData->DeleteExp(IdExperiment);
	Reset();
	emit DeleteExperimentOk();
}