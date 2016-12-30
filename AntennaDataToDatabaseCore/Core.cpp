#include <iostream>
#include <QStringList>
#include <QDir>
#include <QSettings>
#include "Core.h"
#include "FrbrdDatabase.h"
#include "Antenna.h"
#include "ParseFekoFile.h"
#include <stdio.h> 
#include <direct.h>
#include "Windows.h"

using namespace std;

//#include <ctime> // time_t

Core::Core()
{
	outs.clear();
	pres.clear();
	out_names.clear();
	pre_names.clear();
	pFBDataBase = std::make_shared<FrbrdDatabase>();
	pExperiment = std::make_shared<Experiment>();
	donorDB = false;
}

Core::~Core()
{
}

int Core::SetData(Experiment &exp, std::vector<Antenna>& _antennas)
{
	pExperiment = std::make_shared<Experiment>(exp);
	antennas = _antennas;
	return 0;
}

int Core::SetDonorDB()
{
	donorDB = true;
	return 0;
}

std::string Core::GetCurrentDir()
{
	char pth[FILENAME_MAX] = { 0 };
	GetModuleFileName(NULL, pth, FILENAME_MAX);
	char* p = strrchr(pth, '\\');
	if (p) *(p + 1) = 0;
	else pth[0] = 0;

	char current_work_dir_[FILENAME_MAX];
	int j = 0;
	for (int i = 0; i < sizeof(pth); ++i)
	{
		if (pth[i] == '\\')
		{
			current_work_dir_[j] = '/';
			j++;
			continue;
		}
		current_work_dir_[j] = pth[i];
		j++;
	}
	std::string path(current_work_dir_);
	return path;
}

void Core::SetPaths(std::string spath)
{
	QString qpath = QString::fromStdString(spath);
	QSettings sett(qpath, QSettings::IniFormat);
	QString v1 = QString::fromLocal8Bit("Path");
	pathRecipient = sett.value(v1).toString().toStdString();
	QString v2 = QString::fromLocal8Bit("PathDonorDB");
	pathDonor = sett.value(v2).toString().toStdString();
}

std::string Core::GetPathRecipient()
{
	return pathRecipient;
}

std::string Core::GetPathDonor()
{
	return pathDonor;
}

std::string Core::GetPathCurrentDB()
{
	return pathCurrentDB;
}

int Core::ConnectDatabase(const char* pathDB)
{
	cout << "Try to connect database...  " << endl;
	string server, path, login, password;
	login = "SYSDBA";//sett.value("Login").toString().toStdString();
	password = "masterkey";//sett.value("Password").toString().toStdString();
	if (pathDB == nullptr)
	{
		std::string full_path = GetCurrentDir();
		std::string fOptions("Options.ini");
		//path_ = path_ + "/" + path__;
		full_path = full_path + fOptions;
		
		cout << "Settings from file - " << full_path << endl;
		QSettings sett(QString::fromStdString(full_path), QSettings::IniFormat);
		server = sett.value("Server").toString().toStdString();
		SetPaths(full_path);
		if (!donorDB)
		{
			path = pathRecipient;
		}
		else
		{
			path = pathDonor;
		}
		if (path == "")
		{
			cout << "Error! Can not find file Options.ini" << endl;
			return -1;
		}
	}
	else
	{
		server = "127.0.0.1";
		path = pathDB;
	}
	
	if (pFBDataBase->Initialization(server, path, login, password) != 0)
	{
		cout << "Error! Can not connect to database. Check parameters" << endl;
		cout << "Server = " << server << endl;
		cout << "Path = " << path << endl;
		return -2;
	}
	
	cout << "Database connected!" << endl;
	pathCurrentDB = path;
	return 0;
}

int Core::OpenDirectory(std::string strdir, int &cntOutFiles, int &cntPreFiles)
{
	if (!strdir.empty())
	{
		QString qstrdir = QString::fromLocal8Bit(strdir.c_str());
		QDir dir(qstrdir);
		cout << "Directory =" << strdir.c_str() << endl;
		outs = dir.entryList(QStringList("*.out"));
		pres = dir.entryList(QStringList("*.pre"));
		cout << "Find " << outs.size() << " out files" << endl;
		cout << "Find " << pres.size() << " pre files" << endl;
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
				outs[i] = qstrdir + "/" + outs[i];
			}
		}
		if (pres.size() > 0)
		{
			pre_names = pres;
			for (int i=0; i<pres.size(); ++i)
			{
				pres[i] = qstrdir + "/" + pres[i];
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
	QString name, file;

	if (outs.size() > 0 && outs.size() == pres.size())
	{
		ParseFekoFile parseFeko;
		antennas.clear();
		
		QFileInfo filetemp(outs[0]);
		QDir dir(filetemp.absoluteDir());
		QString experiment_name = dir.absolutePath() + QString("/comment.txt");
		parseFeko.ParseFileComment(experiment_name.toStdString(), *(pExperiment.get()));

		//time_t t1, t2;
		//double tpre = 0;
		//double tout = 0;
		for (int i=0; i<outs.size(); ++i)
		{
			Antenna newAntenna;
			antennas.push_back(newAntenna);

			antennas[i].aborted = false;

			name = pre_names.at(i).toLocal8Bit();
			cout << name.toStdString() << endl;
			file = pres[i];
			//time(&t1);
			parseFeko.ParseFilePre(file.toStdString(), antennas[i]);
			//time(&t2);
			//tpre += difftime(t2, t1);

			if (antennas[i].aborted) continue;

			name = out_names.at(i).toLocal8Bit();
			cout << name.toStdString() << endl;
			file = outs[i];
			//time (&t1);
			parseFeko.ParseFileOut(file.toStdString(), antennas[i]);
			//time (&t2);
			//tout += difftime(t2, t1);

			if (antennas[i].aborted) 
			{
				cout << "File " << name.toStdString() << " is aborted!"<< endl;
				continue;
			}
		}

		//cout << tpre << endl;
		//cout << tout << endl;

		cout << "Reading files is finished" << endl;
		return 0;
	}
	cout << "Error! There are no some out or pre files" << endl;
	return -1;
}

int Core::PrepareExperimentBeforeWrite()
{
	tm* wrnow;
	time_t now;
	time(&now);
	wrnow = localtime(&now);
	pExperiment->comment += " (";
	pExperiment->comment += std::to_string(wrnow->tm_year + 1900);
	pExperiment->comment += std::to_string(wrnow->tm_mon + 1);
	pExperiment->comment += std::to_string(wrnow->tm_mday);
	pExperiment->comment += std::to_string(wrnow->tm_hour);
	pExperiment->comment += std::to_string(wrnow->tm_min);
	pExperiment->comment += std::to_string(wrnow->tm_sec);
	pExperiment->comment += ")";
	return 0;
}

int Core::WriteData()
{
	if (antennas.size() > 0)
	{

		int resId = -1;
		for (size_t i = 0; i < antennas.size(); ++i)
		{
			if (!antennas[i].aborted)
			{
				resId = pFBDataBase->WriteExperiment(pExperiment.get());
				break;
			}
		}

		for (size_t i=0; i<antennas.size(); ++i)
		{
			if (!antennas[i].aborted)
			{
				if (out_names.size() > 0)
				{
					QString name = out_names.at(i).toLocal8Bit();
					cout << name.toStdString() << endl;
				}
				pFBDataBase->WriteAntennaData(antennas[i], resId);
			}
		}

		cout << "Writing files to database is finished" << endl;
		return 0;
	}
	cout << "Error! There are not valid antennas" << endl;
	return -1;
}

int Core::Request(std::string requestStr, int countSelect, std::vector<std::vector<double>>& result)
{
	return pFBDataBase->Request(requestStr, countSelect, result);
}

int Core::GetExperiments(std::vector<int>& ids, std::vector<Experiment>& exps, bool fullComment)
{
	return pFBDataBase->GetExperiments(ids, exps, fullComment);
}

int Core::GetAntennasByExperiment(std::vector<Antenna>& antennas, std::vector<int>& antennasID, int idExperiment)
{
	return pFBDataBase->GetAntennas(antennas, antennasID, idExperiment);
}

int Core::DeleteExperiment(int idExp)
{
	return pFBDataBase->DeleteExperiment(idExp);
}