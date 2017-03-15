#include "antennadataviewer.h"
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <math.h>
#include "Antenna.h"
#include <fstream>

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

enum DB_Select_Param
{
	DB_SelectAll,
	DB_SelectExp,
	Concrete_Exp,
	Concrete_Ant,
	Analisys_Sel
};

AntennaDataViewer::AntennaDataViewer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	pSelAll = nullptr;
	pSelEx = nullptr;
	pSelExs = nullptr;
	pSelAnt = nullptr;
	pSelExAnt = nullptr;

	ui.listParInput->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listParOutput->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listDBSelect->setSelectionMode(QListWidget::MultiSelection);
	ui.butCorr->setEnabled(true);

	contextMenu = new QMenu(this);
	contextMenu->addAction(QString::fromLocal8Bit("Создать файл"));

	connect(ui.butCorr, SIGNAL(clicked()), this, SLOT(ClickedCalcCorr()));
	connect(ui.cbLine, SIGNAL(clicked()), this, SLOT(CreateGraph()));
	connect(ui.cbPar, SIGNAL(clicked()), this, SLOT(CreateGraph()));
	connect(ui.listDBSelect, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(DBRowChanged(QListWidgetItem*)));
	connect(ui.PlotWidget, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(PlotMouseMove(QMouseEvent *)));
	connect(ui.PlotWidget, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(PlotMousePress(QMouseEvent *)));
	connect(ui.PlotWidget, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(PlotMouseRelease(QMouseEvent *))); 
	connect(ui.actChangeDB, SIGNAL(triggered()), this, SLOT(ChangeDB()));
	connect(ui.actCopyDB, SIGNAL(triggered()), this, SLOT(CopyDB()));
	connect(ui.actDelDataDB, SIGNAL(triggered()), this, SLOT(DelFromDB()));

	connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(CreateTextFile())); // SLOT(slotActivated(QAction*)));

	QStringList selectPars;
	selectPars << QString::fromLocal8Bit("Поиск по всей базе данных");
	selectPars << QString::fromLocal8Bit("Выбрать некоторые эксперименты");
	selectPars << QString::fromLocal8Bit("Выбрать конкретный эксперимент");
	selectPars << QString::fromLocal8Bit("Выбрать конкретную антенну");
	selectPars << QString::fromLocal8Bit("Анализ выбранных данных");
	
	if (core.ConnectDatabase()==0) 
	{
		for (int i=0; i<selectPars.size(); ++i)
		{
			ui.listDBSelect->insertItem(i, selectPars[i]);
		}

		pSelAll = new SelectAll(&core);

		pSelExs = new SelectExperiments(pSelAll, this);
		connect(pSelExs, SIGNAL(ExperimentsOk()), this, SLOT(ExperimentsOk()));
		connect(pSelExs, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));
		
		pSelEx = new SelectExperiment(pSelAll, this);
		connect(pSelEx, SIGNAL(ExperimentOk()), this, SLOT(ExperimentOk()));
		connect(pSelEx, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));

		pSelAnt = new SelectAntenna(pSelAll, this);
		connect(pSelAnt, SIGNAL(AntennaOk()), this, SLOT(AntennaOk()));
		connect(pSelAnt, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));

		pSelExAnt = new SelectedAntennas(pSelAll, this);
		connect(pSelExAnt, SIGNAL(SelectExpAntOk()), this, SLOT(SelectExpAntOk()));
		//connect(pSelAnt, SIGNAL(Cancel()), this, SLOT(SelectExpAntCancel()));
		
		Init();
	}
	else
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("Ошибка"),QString::fromLocal8Bit("Не могу подключится к БД, проверьте файл Options.ini"));
	}
	dirDB = core.GetCurrentDir();
}

void AntennaDataViewer::Init()
{
	VisibleWidget();
	currentInput = -1;
	currentOutput = -1;
	currentMode = -1;
	DBRowChanged(ui.listDBSelect->item(DB_SelectAll));
	res.clear();
	IdsExperiment.clear();
	IdExperiment = -1;
	IdAntenna = -1;
	selectedPoints.clear();
	selectedY = -1.0;
	ui.PlotWidget->clearGraphs();//Если нужно, но очищаем все графики
	ui.PlotWidget->clearItems();
	ui.PlotWidget->replot();
}

void AntennaDataViewer::ResetWidgets()
{
	pSelExs->Reset();
	pSelEx->Reset();
	pSelAnt->Reset();
	pSelExAnt->Clear();
	Init();
}

void AntennaDataViewer::ChangeDB()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Выберите новую базу данных"), QString::fromLocal8Bit(dirDB.c_str()), tr("Database Files (*.fdb)"));
	
	if (fileName.isEmpty()) return;

	QFileInfo fi(fileName);
	QDir tmp_dir(fi.dir());
	dirDB = tmp_dir.absolutePath().toStdString();
	if (core.ConnectDatabase(fileName.toStdString().c_str()) != 0)
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("Не могу подключится к БД, выберите другую базу данных"));
		if (core.ConnectDatabase() != 0)
		{
			QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("Не могу подключится к БД, проверьте файл Options.ini"));
			exit(0);
		}
	}
	else
	{
		setVisible(false);
		pSelAll->Reset();
		ResetWidgets();
		setVisible(true);
	}
}

void AntennaDataViewer::CopyDB()
{
	if (pCopyDB != nullptr)
	{
		delete pCopyDB;
		pCopyDB = nullptr;
	}
	pCopyDB = new WidgetCopyDB(pSelAll, &core, this);
	pCopyDB->setVisible(true);
	connect(pCopyDB, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));
}

void AntennaDataViewer::DelFromDB()
{
	if (pDelEx != nullptr)
	{
		delete pDelEx;
		pDelEx = nullptr;
	}
	pDelEx = new DeleteExperiment(pSelAll, this);
	pDelEx->setVisible(true);
	connect(pDelEx, SIGNAL(DeleteExperimentOk()), this, SLOT(ResetWidgets()));
	connect(pDelEx, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));
}

void AntennaDataViewer::CreateLists()
{
	ui.listParInput->clear();
	ui.listParOutput->clear();
	QStringList inputPars, outputPars;
	if (!parInsideAntenna)
	{
		inputPars << QString::fromLocal8Bit("Масштаб по Х");
		inputPars << QString::fromLocal8Bit("Масштаб по Y");
		inputPars << QString::fromLocal8Bit("Отношение масштаба антенн к первой");
		inputPars << QString::fromLocal8Bit("Габарит по Х (м.)");
		inputPars << QString::fromLocal8Bit("Габарит по Y (м.)");
		inputPars << QString::fromLocal8Bit("Значение S11 первого минимума (dB)");
		inputPars << QString::fromLocal8Bit("Частота первого минимума (МГц)");
		inputPars << QString::fromLocal8Bit("Значение S11 глобального минимума (dB)");
		inputPars << QString::fromLocal8Bit("Частота глобального минимума (МГц)");


		outputPars << QString::fromLocal8Bit("Значение S11 первого минимума (dB)");
		outputPars << QString::fromLocal8Bit("Частота первого минимума (МГц)");
		outputPars << QString::fromLocal8Bit("Значение S11 глобального минимума (dB)");
		outputPars << QString::fromLocal8Bit("Частота глобального минимума (МГц)");
		outputPars << QString::fromLocal8Bit("Отношение первых S11 к первой антенне");
		outputPars << QString::fromLocal8Bit("Отношение частот первого минимума к первой антенне");
		outputPars << QString::fromLocal8Bit("Отношения глобальных S11 к первой антенне");
		outputPars << QString::fromLocal8Bit("Отношение частот глобального минимума к первой антенне");
	}
	else
	{
		inputPars << QString::fromLocal8Bit("Частота антенны (МГц)");

		outputPars << QString::fromLocal8Bit("Значение S11 (dB)");
	}

	for (int i = 0; i<inputPars.size(); ++i)
	{
		ui.listParInput->insertItem(i, inputPars[i]);
	}
	for (int i = 0; i<outputPars.size(); ++i)
	{
		ui.listParOutput->insertItem(i, outputPars[i]);
	}

	currentInput = -1;
	currentOutput = -1;
	selectedPoints.clear();
}

AntennaDataViewer::~AntennaDataViewer()
{
	if (pSelAll     != nullptr) delete pSelAll;
	if (pSelEx      != nullptr) delete pSelEx;   
	if (pSelExs     != nullptr) delete pSelExs;  
	if (pSelAnt     != nullptr) delete pSelAnt;  
	if (pSelExAnt   != nullptr) delete pSelExAnt;
	if (pDelEx      != nullptr) delete pDelEx;   
	if (pCopyDB     != nullptr) delete pCopyDB;
	if (contextMenu != nullptr) delete contextMenu;
}

void AntennaDataViewer::ClickedCalcCorr()
{
	QList<QListWidgetItem *> inputList = ui.listParInput->selectedItems();
	if (inputList.size() == 0) 
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("Ошибка"),QString::fromLocal8Bit("Выберите хотя бы один входной параметр"));
		return;
	}
	QList<QListWidgetItem *> outputList = ui.listParOutput->selectedItems();
	if (outputList.size() == 0) 
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("Ошибка"),QString::fromLocal8Bit("Выберите хотя бы один выходной параметр"));
		return;
	}
	QList<QListWidgetItem *> dbList = ui.listDBSelect->selectedItems();
	currentMode = ui.listDBSelect->row(dbList.at(0));
	currentInput = ui.listParInput->row(inputList.at(0));
	currentOutput = ui.listParOutput->row(outputList.at(0));

	res.clear();
	vector<double> temp;
	res.push_back(temp);
	res.push_back(temp);

	if (!parInsideAntenna)
	{
		viewDataExp.clear();
		for (size_t i = 0; i < pSelAll->GetExpsID()->size(); i++)
		{
			int indexFirstSelect = -1;
			double minS11_1, minS11W_1;

			int id_ex = pSelAll->GetExpsID()->at(i);
			if (ui.listDBSelect->item(DB_SelectExp)->isSelected() && std::find(std::begin(IdsExperiment), std::end(IdsExperiment), id_ex) == std::end(IdsExperiment)) continue;
			if (ui.listDBSelect->item(Concrete_Exp)->isSelected() && IdExperiment != -1 && id_ex != IdExperiment) continue;
			for (size_t j = 0; j < pSelAll->GetAnts()->at(i).size(); ++j)
			{
				Antenna _ant = pSelAll->GetAnts()->at(i).at(j);
				if (ui.listDBSelect->item(Analisys_Sel)->isSelected() && !pSelExAnt->HaveAntenna(pSelAll->GetAntsID()->at(i).at(j))) continue;
				if (indexFirstSelect == -1) indexFirstSelect = j;
				ViewDataExp vde(id_ex, pSelAll->GetAntsID()->at(i).at(j));
				viewDataExp.push_back(vde);

				double sizeX, sizeY;
				if (currentInput == 3)
				{
					double tx;
					double maxx = -1*std::numeric_limits<double>::max();
					double minx = std::numeric_limits<double>::max();
					for (size_t i = 0; i < _ant.inputPar.Radiator.fr_predmX.size(); ++i)
					{
						tx = _ant.inputPar.Radiator.fr_predmX.at(i);
						if (tx < minx) minx = tx;
						if (tx > maxx) maxx = tx;
					}
					sizeX = maxx - minx;
					sizeX *= _ant.inputPar.Radiator.ScaleX;
				}
				if (currentInput == 4)
				{
					double ty;
					double maxy = -1 * std::numeric_limits<double>::max();
					double miny = std::numeric_limits<double>::max();
					for (size_t i = 0; i < _ant.inputPar.Radiator.fr_predmY.size(); ++i)
					{
						ty = _ant.inputPar.Radiator.fr_predmY.at(i);
						if (ty < miny) miny = ty;
						if (ty > maxy) maxy = ty;
					}
					sizeY = maxy - miny;
					sizeY *= _ant.inputPar.Radiator.ScaleY;
				}


				double minS11, minS11W;
				if (currentOutput == 2 || currentOutput == 3 || currentOutput == 6 || currentOutput == 7
				 || currentInput == 7 || currentInput == 8)
				{
					minS11 = 1000000;
					for (size_t k = 0; k < _ant.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++k)
					{
						double temp = _ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._SCATTERING_PARAMETERS.S11;
						if (temp < minS11) {
							minS11 = temp;
							minS11W = _ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._EXCITATION_BY_VOLTAGE_SOURCE.Frequency;
						}
					}
					if (j == 0)
					{
						minS11_1 = minS11;
						minS11W_1 = minS11W;
					}
				}

				switch (currentInput)
				{
				case 0:
					res[0].push_back(_ant.inputPar.Radiator.ScaleX);
					break;
				case 1:
					res[0].push_back(_ant.inputPar.Radiator.ScaleY);
					break;
				case 2:
					res[0].push_back(_ant.inputPar.Radiator.ScaleX / pSelAll->GetAnts()->at(i).at(indexFirstSelect).inputPar.Radiator.ScaleX);
					break;
				case 3:
					res[0].push_back(sizeX);
					break;
				case 4:
					res[0].push_back(sizeY);
					break;
				case 5:
					res[0].push_back(20 * log10(_ant.outputPar.fst_s11));
					break;
				case 6:
					res[0].push_back(_ant.outputPar.fst_w / 1000000);
					break;
				case 7:
					res[0].push_back(20 * log10(minS11));
					break;
				case 8:
					res[0].push_back(minS11W / 1000000);
					break;
				default:
					break;
				}
				switch (currentOutput)
				{
				case 0:
					res[1].push_back(20 * log10(_ant.outputPar.fst_s11));
					break;
				case 1:
					res[1].push_back(_ant.outputPar.fst_w / 1000000);
					break;
				case 2:
					res[1].push_back(20 * log10(minS11));
					break;
				case 3:
					res[1].push_back(minS11W / 1000000);
					break;
				case 4:
					res[1].push_back(log10(_ant.outputPar.fst_s11) / log10(pSelAll->GetAnts()->at(i).at(indexFirstSelect).outputPar.fst_s11));
					break;
				case 5:
					res[1].push_back(_ant.outputPar.fst_w / pSelAll->GetAnts()->at(i).at(indexFirstSelect).outputPar.fst_w);
					break;
				case 6:
					res[1].push_back(log10(minS11) / log10(minS11_1));
					break;
				case 7:
					res[1].push_back(minS11W / minS11W_1);
					break;
				default:
					break;
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < pSelAll->GetExpsID()->size(); i++)
		{
			if (IdExperiment != -1 && pSelAll->GetExpsID()->at(i) != IdExperiment) continue;
			for (size_t j = 0; j < pSelAll->GetAnts()->at(i).size(); ++j)
			{
				if (IdAntenna != -1 && pSelAll->GetAntsID()->at(i).at(j) != IdAntenna) continue;
				Antenna _ant = pSelAll->GetAnts()->at(i).at(j);
				for (size_t k = 0; k < _ant.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++k)
				{
					switch (currentInput)
					{
					case 0:
						res[0].push_back(_ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._EXCITATION_BY_VOLTAGE_SOURCE.Frequency / 1000000);
						break;
					default:
						break;
					}
					switch (currentOutput)
					{
					case 0:
						res[1].push_back(20 * log10(_ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._SCATTERING_PARAMETERS.S11));
						break;
					default:
						break;
					}
				}
			}
		}
	}
	
	if (res[0].size() < 2 || res[1].size() < 2)
	{
		if (res[0].size() == 0 || res[1].size() == 0)
		{
			QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("По вашему запросу данных не обнаружено, построить график невозможно"));
		}
		if (res[0].size() == 1 || res[1].size() == 1)
		{
			QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("По вашему запросу получено меньше двух значений, построить график невозможно"));
		}
		return;
	}
	
	SortResult(0, res[0].size() - 1);
	
	corr.RegressLine(res[0], res[1], RL_a, RL_b, RL_eps, RL_A);
	corr.RegressParabol(res[0], res[1], RP_a, RP_b, RP_c, RP_eps, RP_A);

	double korr = corr.koefKorr(res[0], res[1]);
	ui.leCorrLine->setText(QString::number(korr));

	double korrPar = corr.koefKorrPar(res[0], res[1], RP_a, RP_b, RP_c);
	ui.leCorrPar->setText(QString::number(korrPar));

	SortResult();

	xGraphTitle = ui.listParInput->item(currentInput)->text();
	yGraphTitle = ui.listParOutput->item(currentOutput)->text();

	selectedPoints.clear();

	CreateGraph();

	ui.leCorrLineA->setText(QString::number(RL_a));
	ui.leCorrLineB->setText(QString::number(RL_b));
	ui.leCorrLineEPS->setText(QString::number(RL_eps));
	ui.leCorrLineDlt->setText(QString::number(RL_A));
	ui.leCorrParA->setText(QString::number(RP_a));
	ui.leCorrParB->setText(QString::number(RP_b));
	ui.leCorrParC->setText(QString::number(RP_c));
	ui.leCorrParEPS->setText(QString::number(RP_eps));
	ui.leCorrParDlt->setText(QString::number(RP_A));
}

void AntennaDataViewer::SortResult()
{
	double equalRes = std::numeric_limits<double>::min();
	int cnt = 0; 
	double step;
	for (int j = 1; j < res[0].size(); j++)
	{
		if (equalRes == std::numeric_limits<double>::min())
		{
			if (res[0][j] == res[0][j - 1])
			{
				equalRes = res[0][j - 1];
				cnt++;
				step = res[0][j] / 1000000000000;
				res[0][j] += step*(double)cnt;
			}
		}
		else
		{
			if (res[0][j] == equalRes)
			{
				cnt++;
				res[0][j] += step*(double)cnt;
			}
			else
			{
				equalRes = std::numeric_limits<double>::min();
				cnt = 0;
			}
		}
	}
}

void AntennaDataViewer::SortResult(int l, int r)
{
	double x = res[0][l + (r - l) / 2];
	int i = l;
	int j = r;
	while (i <= j)
	{
		while (res[0][i] < x) i++;
		while (res[0][j] > x) j--;
		if (i <= j)
		{
			swap(res[0][i], res[0][j]);
			swap(res[1][i], res[1][j]);
			if (!parInsideAntenna) swap(viewDataExp[i], viewDataExp[j]);
			i++;
			j--;
		}
	}
	if (i<r)
		SortResult(i, r);

	if (l<j)
		SortResult(l, j);
}

void AntennaDataViewer::ViewDBSelect(int par)
{
	ui.listDBSelect->item(DB_SelectAll)->setSelected(false);
	ui.listDBSelect->item(DB_SelectExp)->setSelected(false);
	ui.listDBSelect->item(Concrete_Exp)->setSelected(false);
	ui.listDBSelect->item(Concrete_Ant)->setSelected(false);
	ui.listDBSelect->item(Analisys_Sel)->setSelected(false);
	ui.listDBSelect->item(par)->setSelected(true);
}

void AntennaDataViewer::VisibleWidget(QDialog * widget)
{
	pSelEx->setVisible(false);
	pSelExs->setVisible(false);
	pSelAnt->setVisible(false);
	pSelExAnt->setVisible(false);
	if (widget != nullptr)
	{
		widget->setVisible(true);
	}
}

void AntennaDataViewer::DBRowChanged(QListWidgetItem* pSelectRow)
{
	ui.listDBSelect->clearFocus();
	
	int selectRow = -1;
	for (int i = 0; i<ui.listDBSelect->count(); ++i)
	{
		if (ui.listDBSelect->item(i) == pSelectRow)
		{
			selectRow = i;
			break;
		}
	}

	ui.actOpenAntKrona->setEnabled(false);

	switch (selectRow)
	{
	case DB_SelectAll:
		ViewDBSelect(DB_SelectAll);
		currentMode = DB_SelectAll;
		VisibleWidget();
		pSelExAnt->Clear();
		pSelEx->Reset();
		parInsideAntenna = false;
		IdsExperiment.clear();
		IdExperiment = -1;
		IdAntenna = -1;
		CreateLists();
		break;
	case DB_SelectExp:
		ViewDBSelect(DB_SelectExp);
		VisibleWidget(pSelExs);
		break;
	case Concrete_Exp:
		ViewDBSelect(Concrete_Exp);
		VisibleWidget(pSelEx);
		break;
	case Concrete_Ant:
		ui.actOpenAntKrona->setEnabled(true);
		ViewDBSelect(Concrete_Ant);
		VisibleWidget(pSelAnt);
		break;
	case Analisys_Sel:
		if (pSelExAnt->GetDataSelectedExpAnt()->size() == 0)
		{
			QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("Данный режим выбирается автоматически при выборе нескольких антенн на графике"));
			ui.listDBSelect->item(Analisys_Sel)->setSelected(false);
		}
		if (pSelExAnt->GetDataSelectedExpAnt()->size() > 0)
		{
			ViewDBSelect(Analisys_Sel);
		}
		break;
	default:
		break;
	}
}


void AntennaDataViewer::ExperimentsOk()
{
	currentMode = DB_SelectExp;
	ViewDBSelect(DB_SelectExp);
	parInsideAntenna = false;
	IdsExperiment = pSelExs->IdsExperiment;
	IdExperiment = -1;
	IdAntenna = -1;
	CreateLists();
}

void AntennaDataViewer::ExperimentOk()
{
	currentMode = Concrete_Exp;
	ViewDBSelect(Concrete_Exp);
	parInsideAntenna = false;
	IdsExperiment.clear();
	IdExperiment = pSelEx->IdExperiment;
	IdAntenna = -1;
	CreateLists();
}

void AntennaDataViewer::AntennaOk()
{
	currentMode = Concrete_Ant;
	ViewDBSelect(Concrete_Ant);
	parInsideAntenna = true;
	IdsExperiment.clear();
	IdExperiment = pSelAnt->IdExperiment;
	IdAntenna = pSelAnt->IdAntenna;
	CreateLists();
}

void AntennaDataViewer::ExperimentCancel()
{
	ViewDBSelect(currentMode);
	VisibleWidget();
}

void AntennaDataViewer::SelectExpAntOk()
{
	if (pSelExAnt->GetDataSelectedExpAnt()->size() == 1)
	{
		ViewDBSelect(Concrete_Ant);
		parInsideAntenna = true;
		IdsExperiment.clear();
		IdExperiment = pSelExAnt->GetDataSelectedExpAnt()->at(0).IdExperiment;
		IdAntenna = pSelExAnt->GetDataSelectedExpAnt()->at(0).IdAntenna;
		CreateLists();
	}
	if (pSelExAnt->GetDataSelectedExpAnt()->size() > 1)
	{
		currentMode = Analisys_Sel;
		DBRowChanged(ui.listDBSelect->item(Analisys_Sel));
	}
}

void AntennaDataViewer::PlotMousePress(QMouseEvent *event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		if (!parInsideAntenna)
		{
			if (selectedArea.Corner1X == -1 || selectedArea.Corner1Y == -1)
			{
				selectedArea.Corner1X = event->x();
				selectedArea.Corner1Y = event->y();
				selectedArea.Corner2X = event->x();
				selectedArea.Corner2Y = event->y();

				selectedPoints.clear();
				CreateGraph();
			}
		}
		if (!parInsideAntenna)
		{
			selectedY = -1.0;
		}
	}
}

void AntennaDataViewer::PlotMouseRelease(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (!parInsideAntenna)
		{
			selectedArea.Corner2X = event->x();
			selectedArea.Corner2Y = event->y();

			if (selectedPoints.size() > 0)
			{
				std::vector<ViewDataExp> selectedDataExpAnt;
				for (size_t i = 0; i < selectedPoints.size(); ++i)
				{
					if (viewDataExp.size() > selectedPoints[i])
					{
						ViewDataExp newVDE(viewDataExp[selectedPoints[i]].IdExperiment, viewDataExp[selectedPoints[i]].IdAntenna);
						selectedDataExpAnt.push_back(newVDE);
					}
					else
					{
						selectedArea.Reset();
						CreateGraph();
						return;
					}
				}
				pSelExAnt->Reset(selectedDataExpAnt);
				VisibleWidget(pSelExAnt);
				selectedPoints.clear();
			}
			selectedArea.Reset();
			CreateGraph();
		}
		else
		{
			switch (currentInput)
			{
			case 0:
				selectedY = event->y();
				CreateGraph();
				break;
			default:
				break;
			}
		}
	}
	if (event->button() == Qt::RightButton && !res.empty() && !res[0].empty() && !res[1].empty())
	{
		contextMenu->exec(event->globalPos());
	}
}

void AntennaDataViewer::PlotMouseMove(QMouseEvent *event)
{
	if (res.size() == 2)
	{
		if (event->buttons() == Qt::LeftButton)
		{
			if (!parInsideAntenna)
			{
				selectedArea.Corner2X = event->x();
				selectedArea.Corner2Y = event->y();

				selectedPoints.clear();
				selectedPoints.reserve(lineAllPixelData.size());

				for (int i = 0; i < lineAllPixelData.size(); ++i)
				{
					if (selectedArea.IsInside(lineAllPixelData[i].x(), lineAllPixelData[i].y()))
					{
						selectedPoints.push_back(i);
					}
				}
				CreateGraph();
			}
		}
		else
		{
			if (!parInsideAntenna)
			{
				if (currentInput == -1 || currentOutput == -1) return;

				int changedpoint = -1;
				double length, minL, distx, disty;
				minL = 100000000;
				for (int i = 0; i < lineAllPixelData.size(); ++i)
				{
					distx = event->x() - lineAllPixelData[i].x();
					disty = event->y() - lineAllPixelData[i].y();
					distx *= distx;
					disty *= disty;
					length = sqrt(distx + disty);
					if (length < minL)
					{
						minL = length;
						changedpoint = i;
					}
				}
				if (!selectedPoints.empty() && selectedPoints[0] == changedpoint) return;
				else
				{
					if (!selectedPoints.empty()) selectedPoints.clear();
					selectedPoints.push_back(changedpoint);
					CreateGraph();
				}
			}
		}
	}
}

void AntennaDataViewer::CreateGraph()
{
	if (res.empty()) return;
	QVector<double> x, y; //Массивы координат точек
	
	x = QVector<double>::fromStdVector(res[0]);
	y = QVector<double>::fromStdVector(res[1]);
 
	const vector<double> &stdx = res[0];
	const vector<double> &stdy = res[1];

	ui.PlotWidget->clearGraphs();//Если нужно, но очищаем все графики
    //Добавляем один график в widget
    ui.PlotWidget->addGraph();
    //Говорим, что отрисовать нужно график по нашим двум массивам x и y
	QCPGraph * graph0 = ui.PlotWidget->graph(0);
	graph0->setData(x, y);
 
	graph0->setPen(QColor(50, 50, 50, 255));//задаем цвет точки
	graph0->setLineStyle(QCPGraph::lsNone);//убираем линии
    //формируем вид точек
	graph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

	ui.PlotWidget->clearItems();

	if (!selectedPoints.empty())
	{
		QVector<double> x1, y1; //Массивы координат точек
		x1.resize(selectedPoints.size());
		y1.resize(selectedPoints.size());
		for (size_t i = 0; i < selectedPoints.size(); ++i)
		{
			x1[i] = stdx[selectedPoints[i]];
			y1[i] = stdy[selectedPoints[i]];
		}
		ui.PlotWidget->addGraph();
		QCPGraph * graphlast = ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1);
		graphlast->setData(x1, y1);
		graphlast->setPen(QColor(50, 50, 50, 255));//задаем цвет точки
		graphlast->setLineStyle(QCPGraph::lsNone);//убираем линии
		graphlast->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
		
		int posx, posy, centerx, centery;
		double dx, dy, dlt;
		for (size_t i = 0; i < selectedPoints.size(); ++i)
		{
			posx = lineAllPixelData[selectedPoints[i]].x();
			posy = lineAllPixelData[selectedPoints[i]].y();
			centerx = ui.PlotWidget->size().width() / 2;  //350
			centery = ui.PlotWidget->size().height() / 2; //300;
			dx = posx - centerx;
			dy = posy - centery;
			dlt = sqrt((dx*dx) + (dy*dy));
			if (dx == 0 && dy == 0)
			{
				dx = 1 / sqrt(2);
				dy = 1 / sqrt(2);
			}
			else
			{
				dx = dx / dlt;
				dy = dy / dlt;
			}

			int newposx = posx - (30 * dx);
			int newposy = posy - (30 * dy);
			QPointF posF(newposx, newposy);

			QString pointtext;
			if (!parInsideAntenna && viewDataExp.size() > selectedPoints[i])
			{
				pointtext = QString::fromLocal8Bit("Эксперимент: ") + QString::number(viewDataExp[selectedPoints[i]].IdExperiment) +
					QString::fromLocal8Bit("\n Антенна: ") + QString::number(viewDataExp[selectedPoints[i]].IdAntenna);
			}

			QCPItemText *textLabel = new QCPItemText(ui.PlotWidget);
			ui.PlotWidget->addItem(textLabel);
			textLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
			textLabel->position->setPixelPoint(posF);
			textLabel->setText(pointtext);
			textLabel->setFont(QFont(font().family(), 8)); // make font a bit larger
		}
	}

	if (!selectedArea.Empty())
	{
		QCPItemLine *line1 = new QCPItemLine(ui.PlotWidget);
		ui.PlotWidget->addItem(line1);
		QPointF pos1s(selectedArea.Corner1X, selectedArea.Corner1Y);
		line1->start->setPixelPoint(pos1s);
		QPointF pos1f(selectedArea.Corner2X, selectedArea.Corner1Y);
		line1->end->setPixelPoint(pos1f);

		QCPItemLine *line2 = new QCPItemLine(ui.PlotWidget);
		ui.PlotWidget->addItem(line2);
		QPointF pos2s(selectedArea.Corner1X, selectedArea.Corner1Y);
		line2->start->setPixelPoint(pos2s);
		QPointF pos2f(selectedArea.Corner1X, selectedArea.Corner2Y);
		line2->end->setPixelPoint(pos2f);

		QCPItemLine *line3 = new QCPItemLine(ui.PlotWidget);
		ui.PlotWidget->addItem(line3);
		QPointF pos3s(selectedArea.Corner2X, selectedArea.Corner1Y);
		line3->start->setPixelPoint(pos3s);
		QPointF pos3f(selectedArea.Corner2X, selectedArea.Corner2Y);
		line3->end->setPixelPoint(pos3f);

		QCPItemLine *line4 = new QCPItemLine(ui.PlotWidget);
		ui.PlotWidget->addItem(line4);
		QPointF pos4s(selectedArea.Corner1X, selectedArea.Corner2Y);
		line4->start->setPixelPoint(pos4s);
		QPointF pos4f(selectedArea.Corner2X, selectedArea.Corner2Y);
		line4->end->setPixelPoint(pos4f);
	}
	if (selectedY != -1.0)
	{
		int minY = 1000;
		for (int i = 0; i < lineAllPixelData.size(); ++i)
		{
			if (lineAllPixelData[i].y() < minY)
				minY = lineAllPixelData[i].y();
		}

		if (selectedY > minY)
		{
			int c = 0;
			while (c < lineAllPixelData.size())
			{
				int p1=-1, p2=-1, p3=-1, p4 = -1;
				for (size_t i = c; i < lineAllPixelData.size(); ++i, ++c)
				{
					if (lineAllPixelData[i].y() < selectedY) p1 = i;
					else
					{
						p2 = i;
						p3 = i;
						break;
					}
				}
				for (size_t i = c; i < lineAllPixelData.size(); ++i, ++c)
				{
					if (lineAllPixelData[i].y() > selectedY) p3 = i;
					else
					{
						p4 = i;
						break;
					}
				}
				if (p3 != -1 && p2 != -1 && p1 != -1)
				{
					double x1, x2;
					x1 = (lineAllPixelData[p1].x()*lineAllPixelData[p2].y() - lineAllPixelData[p2].x()*lineAllPixelData[p1].y() + selectedY*(lineAllPixelData[p2].x() - lineAllPixelData[p1].x())) / (lineAllPixelData[p2].y() - lineAllPixelData[p1].y());
					if (p4 != -1)
					{
						x2 = (lineAllPixelData[p4].x()*lineAllPixelData[p3].y() - lineAllPixelData[p3].x()*lineAllPixelData[p4].y() + selectedY*(lineAllPixelData[p3].x() - lineAllPixelData[p4].x())) / (lineAllPixelData[p3].y() - lineAllPixelData[p4].y());
					}
					else
					{
						x2 = ui.PlotWidget->size().width();
					}

					double U = (stdx[p2] - stdx[p1])*(x2 - x1) / (lineAllPixelData[p2].x() - lineAllPixelData[p1].x());

					QPointF posF((x2+x1)/2, selectedY);
					QString pointtext = QString::fromLocal8Bit("bw = ") + QString::number(U);
					QCPItemText *textLabel = new QCPItemText(ui.PlotWidget);
					ui.PlotWidget->addItem(textLabel);
					textLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
					textLabel->position->setPixelPoint(posF);
					textLabel->setText(pointtext);
					textLabel->setFont(QFont(font().family(), 8)); // make font a bit larger
					
					QPointF posf1(x1, selectedY);
					QPointF posf2(x2, selectedY);
					QCPItemLine *arrow = new QCPItemLine(ui.PlotWidget);
					ui.PlotWidget->addItem(arrow);
					arrow->start->setPixelPoint(posf1);
					arrow->end->setPixelPoint(posf2);
					arrow->setTail(QCPLineEnding::esSpikeArrow);
					arrow->setHead(QCPLineEnding::esSpikeArrow);
				}
			}
		}

		selectedY = -1.0;
	}
 
    //Подписываем оси Ox и Oy
	ui.PlotWidget->xAxis->setLabel(xGraphTitle);
    ui.PlotWidget->yAxis->setLabel(yGraphTitle);
 
    //Установим область, которая будет показываться на графике
	double minx = *(std::min_element(stdx.begin(), stdx.end()));
	double maxx = *(std::max_element(stdx.begin(), stdx.end()));
	double miny = *(std::min_element(stdy.begin(), stdy.end()));
	double maxy = *(std::max_element(stdy.begin(), stdy.end()));
	double tmpx = (maxx-minx)/10;
	if (tmpx == 0.0) tmpx = 0.1;
	minx -= tmpx;
	maxx += tmpx;
	double tmpy = (maxy - miny) / 10;
	if (tmpy == 0.0) tmpy = 0.1;
	miny -= tmpy;
	maxy += tmpy;

    ui.PlotWidget->xAxis->setRange(minx, maxx);//Для оси Ox
    ui.PlotWidget->yAxis->setRange(miny, maxy);//Для оси Oy

	QVector<double> xL;
	if (ui.cbLine->isChecked() || ui.cbPar->isChecked())
	{
		xL.resize(101);
		for (size_t i = 0; i <= 100; i++)
		{
			xL[i] = minx + (maxx - minx)*i / 100;
		}
		if (ui.cbLine->isChecked())
		{
			ui.PlotWidget->addGraph();
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setPen(QPen(Qt::blue)); // line color red for second graph
			QVector<double> yL;
			yL.resize(101);
			for (size_t i = 0; i <= 100; i++)
			{
				yL[i] = RL_a + xL[i] * RL_b;
			}
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setData(xL, yL);
		}
		if (ui.cbPar->isChecked())
		{
			ui.PlotWidget->addGraph();
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setPen(QPen(Qt::green)); // line color red for second graph
			QVector<double> yP;
			yP.resize(101);
			for (size_t i = 0; i <= 100; i++)
			{
				yP[i] = RP_a + xL[i] * RP_b + xL[i] * xL[i] * RP_c;
			}
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setData(xL, yP);
		}
	}

	//И перерисуем график на нашем widget
    ui.PlotWidget->replot();

	graph0->getDataPosition(x, y, &lineAllPixelData);
}

void AntennaDataViewer::CreateTextFile()
{
	tm* wrnow;
	time_t now;
	time(&now);
	wrnow = localtime(&now);
	std::string namefile = "graph_";
	namefile += std::to_string(wrnow->tm_year + 1900);
	namefile += "_";
	namefile += std::to_string(wrnow->tm_mon + 1);
	namefile += "_";
	namefile += std::to_string(wrnow->tm_mday);
	namefile += "_";
	namefile += std::to_string(wrnow->tm_hour);
	namefile += "-";
	namefile += std::to_string(wrnow->tm_min);
	namefile += "-";
	namefile += std::to_string(wrnow->tm_sec);
	namefile += ".txt";
	ofstream out(namefile);
	if (!out) {
		cout << "Cannot open file.\n";
		return;
	}
	for (size_t i = 0; i < res[0].size(); ++i)
	{
		out << res[0][i] << "\t" << res[1][i] << "\n";
	}
	out.close();
}