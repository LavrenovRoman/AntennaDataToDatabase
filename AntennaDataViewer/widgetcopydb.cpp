#include "widgetcopydb.h"
#include "Antenna.h"
#include <QFileDialog>
#include <QMessageBox>

using namespace std;

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

WidgetCopyDB::WidgetCopyDB(SelectAll* pCoreData, Core* pCore, QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);

	ui.okButton->setEnabled(false);
	connect(ui.butDB1, SIGNAL(clicked()), this, SLOT(SelectDB1()));
	connect(ui.butDB2, SIGNAL(clicked()), this, SLOT(SelectDB2()));
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(CopyDB()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SIGNAL(Cancel()));
	pkCoreData = pCoreData;
	Reset();
	pkCore = pCore;
	dirDB = pkCore->GetCurrentDir();

	if (!pkCore->GetPathDonor().empty())
	{
		pathDB1 = QString::fromLocal8Bit(pkCore->GetPathDonor().c_str());
		ui.leDB1->setText(QString::fromLocal8Bit(pkCore->GetPathDonor().c_str()));
		if (!pkCore->GetPathRecipient().empty())
		{
			pathDB2 = QString::fromLocal8Bit(pkCore->GetPathRecipient().c_str());
			ui.leDB2->setText(QString::fromLocal8Bit(pkCore->GetPathRecipient().c_str()));
			ui.okButton->setEnabled(true);
		}
	}

	ui.progressBar->setValue(0);
}

WidgetCopyDB::~WidgetCopyDB()
{

}

void WidgetCopyDB::SelectDB1()
{
	pathDB1 = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Выберите базу данных - донор"), QString::fromLocal8Bit(dirDB.c_str()), tr("Database Files (*.fdb)"));
	if (pathDB1.isEmpty()) return;
	QFileInfo fi(pathDB1);
	QDir tmp_dir(fi.dir());
	dirDB = tmp_dir.absolutePath().toStdString();
	ui.leDB1->setText(pathDB1);
	if (!ui.leDB1->text().isEmpty() && !ui.leDB2->text().isEmpty()) ui.okButton->setEnabled(true);
	ui.progressBar->setValue(0);
}

void WidgetCopyDB::SelectDB2()
{
	pathDB2 = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Выберите базу данных - реципиент"), QString::fromLocal8Bit(dirDB.c_str()), tr("Database Files (*.fdb)"));
	if (pathDB2.isEmpty()) return;
	QFileInfo fi(pathDB2);
	QDir tmp_dir(fi.dir());
	dirDB = tmp_dir.absolutePath().toStdString();
	ui.leDB2->setText(pathDB2);
	if (!ui.leDB1->text().isEmpty() && !ui.leDB2->text().isEmpty()) ui.okButton->setEnabled(true);
	ui.progressBar->setValue(0);
}

void WidgetCopyDB::CopyDB()
{
	ui.okButton->setEnabled(false);
	ui.cancelButton->setEnabled(false);
	update();
	repaint();
	Core coreRecepient;
	Core coreDonor;
	std::vector<int> donorExpsId, recepientExpsId;
	std::vector<Experiment> donorExps, recepientExps;
	if (coreDonor.ConnectDatabase(pathDB1.toStdString().c_str()) == 0) {
		coreDonor.GetExperiments(donorExpsId, donorExps, true);
	}
	else
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("Не могу подключится к БД-донору, выберите другую базу данных"));
		return;
	}
	if (coreRecepient.ConnectDatabase(pathDB2.toStdString().c_str()) == 0) {
		coreRecepient.GetExperiments(recepientExpsId, recepientExps, true);
	}
	else
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("Ошибка"), QString::fromLocal8Bit("Не могу подключится к БД-реципиенту, выберите другую базу данных"));
		return;
	}
	std::vector<int> donorExpIndexes;
	for (size_t i = 0; i < donorExps.size(); ++i)
	{
		size_t j;
		for (j = 0; j < recepientExps.size(); ++j)
		{
			if (donorExps[i].comment == recepientExps[j].comment)
			{
				break;
			}
		}
		if (j == recepientExps.size())
		{
			donorExpIndexes.push_back(i);
		}
	}
	for (size_t i = 0; i < donorExpIndexes.size(); ++i)
	{
		std::vector<Antenna> antennas;
		std::vector<int> antennasId;
		if (coreDonor.GetAntennasByExperiment(antennas, antennasId, donorExpsId[donorExpIndexes[i]]) == 0) {
			if (coreRecepient.SetData(donorExps[i], antennas) == 0) {
				coreRecepient.WriteData();
			}
		}
		ui.lblResult->setText(QString::fromLocal8Bit("Скопировано экспериментов: ") + QString::number(i + 1));
		ui.progressBar->setValue(100 * (i + 1) / donorExpIndexes.size());
	}
	QString res = QString::fromLocal8Bit("Копирование успешно завершено! Скопировано экспериментов: ") + QString::number(donorExpIndexes.size());
	ui.lblResult->setText(res);
	ui.progressBar->setValue(100);

	ui.okButton->setEnabled(true);
	ui.cancelButton->setEnabled(true);
	update();
	repaint();

	if (donorExpIndexes.size()>0 && pkCore->GetPathCurrentDB() == coreRecepient.GetPathCurrentDB())
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("Внимание"), QString::fromLocal8Bit("В текущую БД добавлены новые данные, требуется перезапуск программы!"));
	}
}

void WidgetCopyDB::Reset()
{
	ui.okButton->setEnabled(false);
	ui.leDB1->clear();
	ui.leDB1->clear();
	repaint();
}