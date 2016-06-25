#include "antennadatatodatabase.h"
#include <QFileDialog>
#include <QSettings>
#include <QVariant>

AntennaDataToDatabase::AntennaDataToDatabase(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//::setCodecForTr(QTextCodec::codecForName("Windows-1251"));
	connect(ui.but_OpenDir, SIGNAL(clicked()), this, SLOT(ClickedOpenDir()));
	connect(ui.but_LoadOutToDB, SIGNAL(clicked()), this, SLOT(ClickedOpenOutFiles()));

	outs.clear();
	pres.clear();
	out_names.clear();
	pre_names.clear();
	ui.lbl_FindFiles->setText("");
	ui.but_LoadOutToDB->setEnabled(false);

	QSettings sett("Options.ini", QSettings::IniFormat);
	std::string server  = sett.value("Server").toString().toStdString();
	std::string path = sett.value("Path").toString().toStdString();
	std::string login = sett.value("Login").toString().toStdString();
	std::string password = sett.value("Password").toString().toStdString();
	if (path == "" || login == "" || password == "")
	{
		ui.lbl_FindFiles->setText(QString::fromLocal8Bit("Ошибка! Не найден файл Options.ini!"));
		ui.but_OpenDir->setEnabled(false);
	}
	else
	{
		if (FBDataBase.Initialization(server, path, login, password) != 0)
		{
			ui.lbl_FindFiles->setText(QString::fromLocal8Bit("Ошибка! Не могу подключиться к БД!"));
			ui.but_OpenDir->setEnabled(false);
		}
	}
}

AntennaDataToDatabase::~AntennaDataToDatabase()
{

}

void AntennaDataToDatabase::ClickedOpenDir()
{
	QString strdir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Открыть папку"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (strdir != "")
	{
		QDir dir(strdir);
		outs = dir.entryList(QStringList("*.out"));
		pres = dir.entryList(QStringList("*.pre"));
		ui.lbl_FindFiles->setText(QString::fromLocal8Bit("Найдено out файлов: ") + QString::number(outs.size()) + QString::fromLocal8Bit("\n") + QString::fromLocal8Bit("Найдено pre файлов: ") + QString::number(pres.size()));
		ui.lbl_Result->setText(QString::fromLocal8Bit(""));
		for (size_t i=0; i<std::min(outs.size(), pres.size()); ++i)
		{
			if (outs[i].left(outs[i].indexOf('.')) != pres[i].left(pres[i].indexOf('.')))
			{
				if (outs.size() > pres.size())
				{
					outs.erase(outs.begin() + i);
					continue;
				}
				if (outs.size() < pres.size())
				{
					pres.erase(pres.begin() + i);
					continue;
				}
			}
		}
		if (outs.size() > 0)
		{
			out_names = outs;
			for (size_t i=0; i<outs.size(); ++i)
			{
				outs[i] = strdir + "/" + outs[i];
			}
		}
		if (pres.size() > 0)
		{
			pre_names = pres;
			for (size_t i=0; i<pres.size(); ++i)
			{
				pres[i] = strdir + "/" + pres[i];
			}
		}
		if (outs.size() > 0 && pres.size() > 0)
		{
			ui.but_LoadOutToDB->setEnabled(true);
		}
	}
}

void AntennaDataToDatabase::ClickedOpenOutFiles()
{
	antennas.clear();
	if (outs.size() > 0)
	{
		ParseFekoFile parseFeko;
		antennas.clear();
		for (size_t i=0; i<outs.size(); ++i)
		{
			QString name = out_names.at(i).toLocal8Bit();
			ui.lbl_Result->setText(name);
			ui.lbl_Result->repaint();
			Antenna newAntenna;
			QString file = outs[i];
			parseFeko.ParseFileOut(file, newAntenna);
			antennas.push_back(newAntenna);
		}

		for (size_t i=0; i<outs.size(); ++i)
		{
			if (!antennas[i].aborted)
			{
				for (size_t j=0; j<pres.size(); ++j)
				{
					if (outs[i].left(outs[i].indexOf('.')) == pres[j].left(pres[j].indexOf('.')))
					{
						QString name = pre_names.at(i).toLocal8Bit();
						ui.lbl_Result->setText(name);
						ui.lbl_Result->repaint();
						QString file = pres[j];
						parseFeko.ParseFilePre(file, antennas[i]);
						break;
					}
				}
			}
		}

		ui.lbl_Result->setText(QString::fromLocal8Bit("Файлы считаны"));
		ui.lbl_Result->repaint();

		if (antennas.size() > 0)
		{
			for (size_t i=0; i<antennas.size(); ++i)
			{
				if (!antennas[i].aborted)
				{
					QString name = out_names.at(i).toLocal8Bit();
					ui.lbl_Result->setText(name);
					ui.lbl_Result->repaint();
					FBDataBase.WriteAntennaData(antennas[i]);
				}
			}

			ui.lbl_Result->setText(QString::fromLocal8Bit("Записано в БД"));
		}
	}
}