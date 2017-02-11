#include "selectexperiment.h"
#include "Antenna.h"

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

SelectExperiment::SelectExperiment(SelectAll* pCoreData, QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);

	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(ExpChanged(int)));
	connect(ui.okButton, SIGNAL(clicked()), this, SIGNAL(ExperimentOk()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SIGNAL(Cancel()));
	pkCoreData = pCoreData;
	Reset();
}

SelectExperiment::~SelectExperiment()
{

}

void SelectExperiment::Reset()
{
	ui.okButton->setEnabled(false);
	ui.listWidget->clear();
	for (size_t i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		ui.listWidget->insertItem(i, QString::number(pkCoreData->GetExpsID()->at(i)));
	}
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	IdExperiment = 0;
	repaint();
}

void SelectExperiment::ExpChanged(int expChange)
{
	ui.okButton->setEnabled(false);
	if (expChange >= 0)
	{
		Experiment & expr = pkCoreData->GetExps()->at(expChange);

		QString date = QString::number(expr.date.tm_mday) + "." + QString::number(expr.date.tm_mon) + "." + QString::number(expr.date.tm_year);
		ui.leDate->setText(date);

		QString cicles;
		for (size_t i = 0; i<expr.cycles.size(); ++i)
		{
			Experiment_Param & expp = expr.cycles[i];
			cicles += QString::fromStdString(expp.name) + " " + QString::number(expp.pBegin) + " " + QString::number(expp.pEnd) + " " + QString::number(expp.pStep);
			if (i != expr.cycles.size() - 1)	cicles += "   ";
		}
		ui.leCicles->setText(cicles);

		ui.leComm->setText(QString::fromLocal8Bit(expr.comment.data()));

		IdExperiment = pkCoreData->GetExpsID()->at(expChange);
		ui.okButton->setEnabled(true);
	}
}