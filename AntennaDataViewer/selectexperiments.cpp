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
	connect(ui.dateEditFrom, SIGNAL(dateChanged()), this, SIGNAL(Filter()));
	connect(ui.dateEditBefore, SIGNAL(dateChanged(QDate)), this, SIGNAL(Filter()));
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
	for (int i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		ui.listWidget->insertItem(i, QString::number(pkCoreData->GetExpsID()->at(i)));
		IdsExperiment.push_back(pkCoreData->GetExpsID()->at(i));
	}
	ui.leDate->clear();
	ui.leCicles->clear();
	ui.leComm->clear();
	repaint();
	Filter();
}

void SelectExperiments::Filter()
{
	QDate dataBegin = ui.dateEditFrom->date();
	QDate dataEnd = ui.dateEditBefore->date();
	QString comm = ui.leCommFilter->text();

	ui.listWidget->clear();
	IdsExperiment.clear();

	for (int i = 0; i<pkCoreData->GetExpsID()->size(); ++i)
	{
		QString date = QString::number(pkCoreData->GetExps()->at(i).date.tm_mday) + "." + QString::number(pkCoreData->GetExps()->at(i).date.tm_mon) + "." + QString::number(pkCoreData->GetExps()->at(i).date.tm_year);
		QDate dataEx(pkCoreData->GetExps()->at(i).date.tm_year, pkCoreData->GetExps()->at(i).date.tm_mon, pkCoreData->GetExps()->at(i).date.tm_mday);
		if (dataEx >= dataBegin && dataEx <= dataEnd)
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
	//ui.okButton->setEnabled(false);
	if (expChange >= 0)
	{
		QString date = QString::number(pkCoreData->GetExps()->at(expChange).date.tm_mday) + "." + QString::number(pkCoreData->GetExps()->at(expChange).date.tm_mon) + "." + QString::number(pkCoreData->GetExps()->at(expChange).date.tm_year);
		ui.leDate->setText(date);

		QString cicles;
		for (size_t i = 0; i<pkCoreData->GetExps()->at(expChange).cycles.size(); ++i)
		{
			cicles += QString::fromStdString(pkCoreData->GetExps()->at(expChange).cycles[i].name) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pBegin) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pEnd) + " " + QString::number(pkCoreData->GetExps()->at(expChange).cycles[i].pStep);
			cicles += "   ";
		}
		ui.leCicles->setText(cicles);

		ui.leComm->setText(QString::fromLocal8Bit(pkCoreData->GetExps()->at(expChange).comment.data()));

		//IdExperiment = pkCoreData->GetExpsID()->at(expChange);
		//ui.okButton->setEnabled(true);
	}
}