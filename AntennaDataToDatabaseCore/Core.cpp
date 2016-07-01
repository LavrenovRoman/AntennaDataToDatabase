#include <iostream>
#include <QFileDialog>
#include <QSettings>
#include <QVariant>
#include "Core.h"
#include "FrbrdDatabase.h"
#include "Antenna.h"
#include "ParseFekoFile.h"

using namespace std;

Core::Core()
{
	outs.clear();
	pres.clear();
	out_names.clear();
	pre_names.clear();
	pFBDataBase = std::make_shared<FrbrdDatabase>();
}

Core::~Core()
{
}

int Core::ConnectDatabase()
{
	QSettings sett("Options.ini", QSettings::IniFormat);
	string server  = sett.value("Server").toString().toStdString();
	string path = sett.value("Path").toString().toStdString();
	string login = sett.value("Login").toString().toStdString();
	string password = sett.value("Password").toString().toStdString();
	if (path == "" || login == "" || password == "")
	{
		cout << "Error! Can not find file Options.ini" << endl;
		return -1;
	}
	else
	{
		if (pFBDataBase->Initialization(server, path, login, password) != 0)
		{
			cout << "Error! Can not connect to database. Check parameters" << endl;
			return -2;
		}
	}
	return 0;
}

int Core::OpenDirectory(QString strdir, int &cntOutFiles, int &cntPreFiles)
{
	if (strdir != "")
	{
		QDir dir(strdir);
		outs = dir.entryList(QStringList("*.out"));
		pres = dir.entryList(QStringList("*.pre"));
		cout << "Find " << outs.size() << "out files" << endl;
		cout << "Find " << pres.size() << "pre files" << endl;
		for (int i=0; i<min(outs.size(), pres.size()); ++i)
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
			for (int i=0; i<outs.size(); ++i)
			{
				outs[i] = strdir + "/" + outs[i];
			}
		}
		if (pres.size() > 0)
		{
			pre_names = pres;
			for (int i=0; i<pres.size(); ++i)
			{
				pres[i] = strdir + "/" + pres[i];
			}
		}
		if (outs.size() > 0 && pres.size() > 0)
		{
			cntOutFiles = outs.size();
			cntPreFiles = pres.size();
			cout << "Directory with .out and .pre files is opened" << endl;
			return 0;
		}
		else
		{
			std::cout << "Error! Check existence of .out and .pre files" << endl;
			return -1;
		}
	}
	std::cout << "Error! Name of directory does not correct" << endl;
	return -2;
}

int Core::ReadFiles()
{
	if (outs.size() > 0)
	{
		ParseFekoFile parseFeko;
		antennas.clear();
		for (int i=0; i<outs.size(); ++i)
		{
			QString name = out_names.at(i).toLocal8Bit();
			cout << name.toStdString() << endl;
			Antenna newAntenna;
			QString file = outs[i];
			parseFeko.ParseFileOut(file, newAntenna);
			antennas.push_back(newAntenna);
		}

		for (int i=0; i<outs.size(); ++i)
		{
			if (!antennas[i].aborted)
			{
				for (int j=0; j<pres.size(); ++j)
				{
					if (outs[i].left(outs[i].indexOf('.')) == pres[j].left(pres[j].indexOf('.')))
					{
						QString name = pre_names.at(i).toLocal8Bit();
						cout << name.toStdString() << endl;
						QString file = pres[j];
						parseFeko.ParseFilePre(file, antennas[i]);
						break;
					}
				}
			}
		}

		cout << "Reading files is finished" << endl;
		return 0;
	}
	cout << "Error! There are no out files" << endl;
	return -1;
}

int Core::WriteData()
{
	if (antennas.size() > 0)
	{
		for (size_t i=0; i<antennas.size(); ++i)
		{
			if (!antennas[i].aborted)
			{
				QString name = out_names.at(i).toLocal8Bit();
				cout << name.toStdString() << endl;
				pFBDataBase->WriteAntennaData(antennas[i]);
			}
		}

		cout << "Writing files to database is finished" << endl;
		return 0;
	}
	cout << "Error! There are not valid antennas" << endl;
	return -1;
}