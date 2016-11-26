#include "antennadataviewer.h"
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <math.h>
#include "correlat.h"
#include "Antenna.h"


using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

AntennaDataViewer::AntennaDataViewer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	pSelAll = nullptr;
	pSelEx = nullptr;
	pSelAnt = nullptr;
	pSelExAnt = nullptr;

	ui.listParInput->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listParOutput->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listDBSelect->setSelectionMode(QListWidget::MultiSelection);
	ui.butCorr->setEnabled(true);

	connect(ui.butCorr, SIGNAL(clicked()), this, SLOT(ClickedCalcCorr()));
	connect(ui.cbLine, SIGNAL(clicked()), this, SLOT(CreateGraph()));
	connect(ui.cbPar, SIGNAL(clicked()), this, SLOT(CreateGraph()));
	connect(ui.listDBSelect, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(DBRowChanged(QListWidgetItem*)));
	connect(ui.PlotWidget, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(PlotMouseMove(QMouseEvent *)));
	connect(ui.PlotWidget, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(PlotMousePress(QMouseEvent *)));
	connect(ui.PlotWidget, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(PlotMouseRelease(QMouseEvent *))); 

	selectPars << QString::fromLocal8Bit("Поиск по всей базе данных");
	selectPars << QString::fromLocal8Bit("Выбрать конкретный эксперимент");
	selectPars << QString::fromLocal8Bit("Выбрать конкретную антенну");
	selectPars << QString::fromLocal8Bit("Анализ выбранных данных");
	
	if (core.ConnectDatabase()==0) 
	{
		pSelAll = new SelectAll(&core);

		for (int i=0; i<selectPars.size(); ++i)
		{
			ui.listDBSelect->insertItem(i, selectPars[i]);
		}
		
		pSelEx = new SelectExperiment(pSelAll, this);
		connect(pSelEx, SIGNAL(ExperimentOk()), this, SLOT(ExperimentOk()));
		connect(pSelEx, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));
		pSelEx->setVisible(false);

		pSelAnt = new SelectAntenna(pSelAll, this);
		connect(pSelAnt, SIGNAL(AntennaOk()), this, SLOT(AntennaOk()));
		connect(pSelAnt, SIGNAL(Cancel()), this, SLOT(ExperimentCancel()));
		pSelAnt->setVisible(false);

		pSelExAnt = new SelectedAntennas(pSelAll, this);
		connect(pSelExAnt, SIGNAL(SelectExpAntOk()), this, SLOT(SelectExpAntOk()));
		//connect(pSelAnt, SIGNAL(Cancel()), this, SLOT(SelectExpAntCancel()));
		pSelExAnt->setVisible(false);

		currentInput  = -1;
		currentOutput = -1;

		currentMode = -1;
		DBRowChanged(ui.listDBSelect->item(0));
	}
	else
	{
		QMessageBox::information(NULL,QString::fromLocal8Bit("Ошибка"),QString::fromLocal8Bit("Не могу подключится к БД, проверьте файл Options.ini"));
	}
	res.clear();
	IdExperiment = -1;
	IdAntenna = -1;
	selectedPoints.clear();
	selectedY = -1.0;
}

void AntennaDataViewer::CreateLists()
{
	ui.listParInput->clear();
	ui.listParOutput->clear();
	inputPars.clear();
	outputPars.clear();
	if (!parInsideAntenna)
	{
		inputPars << QString::fromLocal8Bit("Масштаб по Х");
		inputPars << QString::fromLocal8Bit("Масштаб по Y");

		outputPars << QString::fromLocal8Bit("Значение S11 первого минимума (dB)");
		outputPars << QString::fromLocal8Bit("Частота первого минимума (МГц)");
		outputPars << QString::fromLocal8Bit("Значение S11 глобального минимума (dB)");
		outputPars << QString::fromLocal8Bit("Частота глобального минимума (МГц)");
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
	if (pSelAll   != nullptr) delete pSelAll;
	if (pSelEx    != nullptr) delete pSelEx;
	if (pSelAnt   != nullptr) delete pSelAnt;
	if (pSelExAnt != nullptr) delete pSelExAnt;
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
		for (int i = 0; i < pSelAll->GetExpsID()->size(); i++)
		{
			if (ui.listDBSelect->item(1)->isSelected() && IdExperiment != -1 && pSelAll->GetExpsID()->at(i) != IdExperiment) continue;
			for (int j = 0; j < pSelAll->GetAnts()->at(i).size(); ++j)
			{
				Antenna _ant = pSelAll->GetAnts()->at(i).at(j);
				if (ui.listDBSelect->item(3)->isSelected() && !pSelExAnt->HaveAntenna(pSelAll->GetAntsID()->at(i).at(j))) continue;
				ViewDataExp vde(pSelAll->GetExpsID()->at(i), pSelAll->GetAntsID()->at(i).at(j));
				viewDataExp.push_back(vde);

				double minS11, minS11W;
				if (currentOutput == 2 || currentOutput == 3)
				{
					minS11 = 1000000;
					for (int k = 0; k < _ant.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++k)
					{
						double temp = _ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._SCATTERING_PARAMETERS.S11;
						if (temp < minS11) {
							minS11 = temp;
							minS11W = _ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[0].Frequency;
						}
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
				default:
					break;
				}
				switch (currentOutput)
				{
				case 0:
					res[1].push_back(_ant.outputPar.fst_s11);
					break;
				case 1:
					res[1].push_back(_ant.outputPar.fst_w);
					break;
				case 2:
					res[1].push_back(minS11);
					break;
				case 3:
					res[1].push_back(minS11W);
					break;
				default:
					break;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < pSelAll->GetExpsID()->size(); i++)
		{
			if (IdExperiment != -1 && pSelAll->GetExpsID()->at(i) != IdExperiment) continue;
			for (int j = 0; j < pSelAll->GetAnts()->at(i).size(); ++j)
			{
				if (IdAntenna != -1 && pSelAll->GetAntsID()->at(i).at(j) != IdAntenna) continue;
				Antenna _ant = pSelAll->GetAnts()->at(i).at(j);
				for (int k = 0; k < _ant.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++k)
				{
					switch (currentInput)
					{
					case 0:
						res[0].push_back(_ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[0].Frequency);
						break;
					default:
						break;
					}
					switch (currentOutput)
					{
					case 0:
						res[1].push_back(_ant.outputPar._VEC_DATA_FOR_ONE_FREQ[k]._SCATTERING_PARAMETERS.S11);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	SortResult(0, res[0].size() - 1);
	SortResult();

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

	/*
	std::string request;

	if (!parInsideAntenna)
	{
		request = "select ";

		switch (selectInputPar)
		{
		case 0:
			request += "scalex";
			break;
		case 1:
			request += "scaley";
			break;
		default:
			break;
		}
		request += ", ";

		switch (selectOutputPar)
		{
		case 0:
			request += "fst_s11";
			break;
		case 1:
			request += "fst_w";
			break;
		default:
			break;
		}
		request += " ";

		request += "from output_params, input_params, antenna";

		if (ui.listDBSelect->item(1)->isSelected() && IdExperiment != -1)
		{
			request += ", EXPERIMENT";
		}

		request += " where input_params.id_antenna = antenna.id and output_params.id_antenna = antenna.id";

		if (ui.listDBSelect->item(1)->isSelected() && IdExperiment != -1)
		{
			request += " and antenna.ID_EXPERIMENT = EXPERIMENT.ID and EXPERIMENT.ID = ";
			request += std::to_string(IdExperiment);
		}

		request += " order by ";
		switch (selectInputPar)
		{
		case 0:
			request += "scalex";
			break;
		case 1:
			request += "scaley";
			break;
		default:
			break;
		}
	}
	else
	{
		request = "select ";

		switch (selectInputPar)
		{
		case 0:
			request += "frequency";
			break;
		default:
			break;
		}
		request += ", ";

		switch (selectOutputPar)
		{
		case 0:
			request += "s11";
			break;
		default:
			break;
		}
		request += " ";

		request += "from SCATTERING_PARAMETERS, OUTPUT_PARAMS_FOR_ONE_FREQ, OUTPUT_PARAMS, ANTENNA where SCATTERING_PARAMETERS.ID_OUTPUT = OUTPUT_PARAMS_FOR_ONE_FREQ.id and OUTPUT_PARAMS_FOR_ONE_FREQ.ID_OUTPUT_PARAM = OUTPUT_PARAMS.id";

		if (ui.listDBSelect->item(2)->isSelected() && IdAntenna != -1)
		{
			request += " and OUTPUT_PARAMS.ID_ANTENNA = ANTENNA.ID and ANTENNA.ID = ";
			request += std::to_string(IdAntenna);
		}
	}

	core.Request(request, 2, res);
	*/

	xGraphTitle = ui.listParInput->item(currentInput)->text();
	yGraphTitle = ui.listParOutput->item(currentOutput)->text();

	selectedPoints.clear();

	CreateGraph();
}

void AntennaDataViewer::SortResult()
{
	double equalRes = std::numeric_limits<double>::min();
	double cnt = 1.0;
	for (int j = res[0].size()-1; j > 0; j--)
	{
		if (res[0][j] == res[0][j - 1])
		{
			if (equalRes == res[0][j - 1]) cnt++;
			else cnt = 1.0;
			res[0][j] += std::numeric_limits<double>::epsilon()*cnt;//(j + 1);
			equalRes = res[0][j - 1];
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

void AntennaDataViewer::DBRowChanged(QListWidgetItem* pSelectRow)
{
	ui.listDBSelect->clearFocus();

	int selectRow = -1;
	for (size_t i=0; i<selectPars.size(); ++i)
	{
		if (ui.listDBSelect->item(i) == pSelectRow)
		{
			selectRow = i;
			break;
		}
	}

	switch (selectRow)
	{
	case 0:
		currentMode = 0;
		ui.listDBSelect->item(0)->setSelected(true);
		ui.listDBSelect->item(1)->setSelected(false);
		ui.listDBSelect->item(2)->setSelected(false);
		ui.listDBSelect->item(3)->setSelected(false);
		pSelEx->setVisible(false);
		pSelAnt->setVisible(false);
		pSelExAnt->setVisible(false);
		pSelExAnt->Clear();
		pSelEx->Reset();
		parInsideAntenna = false;
		IdExperiment = -1;
		IdAntenna = -1;
		CreateLists();
		break;
	case 1:
		ui.listDBSelect->item(0)->setSelected(false);
		ui.listDBSelect->item(1)->setSelected(true);
		ui.listDBSelect->item(2)->setSelected(false);
		ui.listDBSelect->item(3)->setSelected(false);
		pSelEx->setVisible(true);
		pSelAnt->setVisible(false);
		pSelExAnt->setVisible(false);
		pSelExAnt->Clear();
		break;
	case 2:
		ui.listDBSelect->item(0)->setSelected(false);
		ui.listDBSelect->item(1)->setSelected(false);
		ui.listDBSelect->item(2)->setSelected(true);
		ui.listDBSelect->item(3)->setSelected(false);
		pSelEx->setVisible(false);
		pSelAnt->setVisible(true);
		pSelExAnt->setVisible(false);
		pSelExAnt->Clear();
		break;
	case 3:
		if (pSelExAnt->GetDataSelectedExpAnt()->size() == 0)
		{
			QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("Данный режим выбирается автоматически при выборе нескольких антенн на графике"));
			ui.listDBSelect->item(3)->setSelected(false);
		}
		if (pSelExAnt->GetDataSelectedExpAnt()->size() > 0)
		{
			ui.listDBSelect->item(0)->setSelected(false);
			ui.listDBSelect->item(1)->setSelected(false);
			ui.listDBSelect->item(2)->setSelected(false);
			ui.listDBSelect->item(3)->setSelected(true);
		}
		break;
	default:
		break;
	}
}

void AntennaDataViewer::ExperimentOk()
{
	currentMode = 1;
	ui.listDBSelect->item(0)->setSelected(false);
	ui.listDBSelect->item(1)->setSelected(true);
	ui.listDBSelect->item(2)->setSelected(false);
	ui.listDBSelect->item(3)->setSelected(false);
	parInsideAntenna = false;
	IdExperiment = pSelEx->IdExperiment;
	IdAntenna = -1;
	CreateLists();
}

void AntennaDataViewer::AntennaOk()
{
	currentMode = 2;
	ui.listDBSelect->item(0)->setSelected(false);
	ui.listDBSelect->item(1)->setSelected(false);
	ui.listDBSelect->item(2)->setSelected(true);
	ui.listDBSelect->item(3)->setSelected(false);
	parInsideAntenna = true;
	IdExperiment = pSelAnt->IdExperiment;
	IdAntenna = pSelAnt->IdAntenna;
	CreateLists();
}

void AntennaDataViewer::ExperimentCancel()
{
	ui.listDBSelect->item(0)->setSelected(false);
	ui.listDBSelect->item(1)->setSelected(false);
	ui.listDBSelect->item(2)->setSelected(false);
	ui.listDBSelect->item(3)->setSelected(false);
	ui.listDBSelect->item(currentMode)->setSelected(true);
	pSelEx->setVisible(false);
	pSelAnt->setVisible(false);
	pSelExAnt->setVisible(false);
}

void AntennaDataViewer::SelectExpAntOk()
{
	if (pSelExAnt->GetDataSelectedExpAnt()->size() == 1)
	{
		ui.listDBSelect->item(0)->setSelected(false);
		ui.listDBSelect->item(1)->setSelected(false);
		ui.listDBSelect->item(2)->setSelected(true);
		ui.listDBSelect->item(3)->setSelected(false);
		parInsideAntenna = true;
		IdExperiment = pSelExAnt->GetDataSelectedExpAnt()->at(0).IdExperiment;
		IdAntenna = pSelExAnt->GetDataSelectedExpAnt()->at(0).IdAntenna;
		CreateLists();
	}
	if (pSelExAnt->GetDataSelectedExpAnt()->size() > 2)
	{
		DBRowChanged(ui.listDBSelect->item(3));
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
				for (int i = 0; i < selectedPoints.size(); ++i)
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
				pSelExAnt->setVisible(true);
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
}

void AntennaDataViewer::PlotMouseMove(QMouseEvent *event)
{
	if (res.size() == 2)
	{
		QVector<QPointF> linePixelData;
		QVector<QCPData> scatterData;
		ui.PlotWidget->graph(0)->getLinePlotData(&linePixelData, &scatterData);

		if (event->buttons() == Qt::LeftButton)
		{
			if (!parInsideAntenna)
			{
				selectedArea.Corner2X = event->x();
				selectedArea.Corner2Y = event->y();

				selectedPoints.clear();

				for (int i = 0; i < linePixelData.size(); ++i)
				{
					if (selectedArea.IsInside(linePixelData[i].x(), linePixelData[i].y()))
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
				for (int i = 0; i < linePixelData.size(); ++i)
				{
					distx = event->x() - linePixelData[i].x();
					disty = event->y() - linePixelData[i].y();
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
	if (!parInsideAntenna)
	{
		switch (currentOutput)
		{
		case 0:
			for (int k = 0; k < y.size(); k++)	{y[k] = 10 * log10(y[k]);}
			break;
		case 1:
			for (int k = 0; k < y.size(); k++)	{y[k] /= 1000000;}
			break;
		case 2:
			for (int k = 0; k < y.size(); k++)	{ y[k] = 10 * log10(y[k]); }
			break;
		case 3:
			for (int k = 0; k < y.size(); k++)	{ y[k] /= 1000000; }
			break;
		default:
			break;
		}
	}
	else
	{
		switch (currentInput)
		{
		case 0:
			for (int k = 0; k < x.size(); k++) {x[k] /= 1000000;}
			break;
		default:
			break;
		}
		switch (currentOutput)
		{
		case 0:
			for (int k = 0; k < y.size(); k++) {y[k] = 10 * log10(y[k]); }
			break;
		default:
			break;
		}
	}
 
	vector<double> stdx = x.toStdVector();
	vector<double> stdy = y.toStdVector();

	ui.PlotWidget->clearGraphs();//Если нужно, но очищаем все графики
    //Добавляем один график в widget
    ui.PlotWidget->addGraph();
    //Говорим, что отрисовать нужно график по нашим двум массивам x и y
    ui.PlotWidget->graph(0)->setData(x, y);
 
    ui.PlotWidget->graph(0)->setPen(QColor(50, 50, 50, 255));//задаем цвет точки
    ui.PlotWidget->graph(0)->setLineStyle(QCPGraph::lsNone);//убираем линии
    //формируем вид точек
    ui.PlotWidget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

	ui.PlotWidget->clearItems();

	if (!selectedPoints.empty())
	{
		for (int i = 0; i < selectedPoints.size(); ++i)
		{
			ui.PlotWidget->addGraph();
			QVector<double> x1, y1; //Массивы координат точек
			x1.push_back(stdx[selectedPoints[i]]);
			y1.push_back(stdy[selectedPoints[i]]);
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setData(x1, y1);
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setPen(QColor(50, 50, 50, 255));//задаем цвет точки
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setLineStyle(QCPGraph::lsNone);//убираем линии
			ui.PlotWidget->graph(ui.PlotWidget->graphCount() - 1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 8));
		}


		QVector<QPointF> linePixelData;
		QVector<QCPData> scatterData;
		ui.PlotWidget->graph(0)->getLinePlotData(&linePixelData, &scatterData);

		for (int i = 0; i < selectedPoints.size(); ++i)
		{
			int posx = linePixelData[selectedPoints[i]].x();
			int posy = linePixelData[selectedPoints[i]].y();
			int centerx = ui.PlotWidget->size().width() / 2;  //350
			int centery = ui.PlotWidget->size().height() / 2; //300;
			double dx = posx - centerx;
			double dy = posy - centery;
			double dlt = sqrt((dx*dx) + (dy*dy));
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
		QVector<QPointF> linePixelData;
		QVector<QCPData> scatterData;
		ui.PlotWidget->graph(0)->getLinePlotData(&linePixelData, &scatterData);

		int minY = 1000;
		for (int i = 0; i < linePixelData.size(); ++i)
		{
			if (linePixelData[i].y() < minY)
				minY = linePixelData[i].y();
		}

		if (selectedY > minY)
		{
			int c = 0;
			while (c < linePixelData.size())
			{
				int p1=-1, p2=-1, p3=-1, p4 = -1;
				for (int i = c; i < linePixelData.size(); ++i, ++c)
				{
					if (linePixelData[i].y() < selectedY) p1 = i;
					else
					{
						p2 = i;
						p3 = i;
						break;
					}
				}
				for (int i = c; i < linePixelData.size(); ++i, ++c)
				{
					if (linePixelData[i].y() > selectedY) p3 = i;
					else
					{
						p4 = i;
						break;
					}
				}
				if (p3 != -1 && p2 != -1 && p1 != -1)
				{
					double x1, x2;
					x1 = (linePixelData[p1].x()*linePixelData[p2].y() - linePixelData[p2].x()*linePixelData[p1].y() + selectedY*(linePixelData[p2].x() - linePixelData[p1].x())) / (linePixelData[p2].y() - linePixelData[p1].y());
					if (p4 != -1)
					{
						x2 = (linePixelData[p4].x()*linePixelData[p3].y() - linePixelData[p3].x()*linePixelData[p4].y() + selectedY*(linePixelData[p3].x() - linePixelData[p4].x())) / (linePixelData[p3].y() - linePixelData[p4].y());
					}
					else
					{
						x2 = ui.PlotWidget->size().width();
					}

					double U = (stdx[p2] - stdx[p1])*(x2 - x1) / (linePixelData[p2].x() - linePixelData[p1].x());

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
 
    Correlat corr;
	double korr = corr.koefKorr(stdx, stdy);
	ui.leCorrLine->setText(QString::number(korr));

	double a, b, c, eps, A;
	
	corr.RegressLine(stdx, stdy, a, b, eps, A);
	ui.leCorrLineA->setText(QString::number(a));
	ui.leCorrLineB->setText(QString::number(b));
	ui.leCorrLineEPS->setText(QString::number(eps));
	ui.leCorrLineDlt->setText(QString::number(A));
	if (ui.cbLine->isChecked())
	{
		ui.PlotWidget->addGraph();
		ui.PlotWidget->graph(ui.PlotWidget->graphCount()-1)->setPen(QPen(Qt::blue)); // line color red for second graph
		QVector<double> xL, yL;
		for (size_t i=0; i<=100; i++)
		{
			xL.push_back(minx + (maxx-minx)*i/100);
			yL.push_back(a + xL[i]*b);
		}
		ui.PlotWidget->graph(ui.PlotWidget->graphCount()-1)->setData(xL, yL);
	}

	corr.RegressParabol(stdx, stdy, a, b, c, eps, A);
	ui.leCorrParA->setText(QString::number(a));
	ui.leCorrParB->setText(QString::number(b));
	ui.leCorrParC->setText(QString::number(c));
	ui.leCorrParEPS->setText(QString::number(eps));
	ui.leCorrParDlt->setText(QString::number(A));

	if (ui.cbPar->isChecked())
	{
		ui.PlotWidget->addGraph();
		ui.PlotWidget->graphCount();
		ui.PlotWidget->graph(ui.PlotWidget->graphCount()-1)->setPen(QPen(Qt::green)); // line color red for second graph
		QVector<double> xL, yL;
		for (size_t i=0; i<=100; i++)
		{
			xL.push_back(minx + (maxx-minx)*i/100);
			yL.push_back(a + xL[i]*b + xL[i]*xL[i]*c);
		}
		ui.PlotWidget->graph(ui.PlotWidget->graphCount()-1)->setData(xL, yL);
	}

	//И перерисуем график на нашем widget
    ui.PlotWidget->replot();
}