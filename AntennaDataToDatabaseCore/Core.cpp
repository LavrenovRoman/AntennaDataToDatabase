#include <iostream>
#include <stdio.h> 
#include "Windows.h"
#include "Antenna.h"
#include "INIReader.h"
#include "ParseFekoFile.h"
#include "FrbrdDatabase.h"
#include "Core.h"

using namespace std;

#include <ctime> // time_t

Core::Core()
{
	sOuts.clear(); 
	sPres.clear(); 
	sOuts_paths.clear();
	sPres_paths.clear();
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
	INIReader reader(spath);
	pathRecipient = reader.Get("", "path", "");
	pathDonor = reader.Get("", "pathdonordb", "");
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
	login = "SYSDBA";
	password = "masterkey";
	server = "127.0.0.1";
	if (pathDB == nullptr)
	{
		std::string full_path = GetCurrentDir();
		std::string fOptions("Options.ini");
		full_path = full_path + fOptions;
		
		cout << "Settings from file - " << full_path << endl;
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
		filesDirectory = strdir;

		std::string sout = strdir + "\\*.out";
		std::string spre = strdir + "\\*.pre";
		sOuts.clear();
		sPres.clear();
		WIN32_FIND_DATA fileData;
		HANDLE hFind;
		if (!((hFind = FindFirstFile(sout.c_str(), &fileData)) == INVALID_HANDLE_VALUE)) {
			do sOuts.push_back(fileData.cFileName);
			while (FindNextFile(hFind, &fileData));
		}
		FindClose(hFind);
		if (!((hFind = FindFirstFile(spre.c_str(), &fileData)) == INVALID_HANDLE_VALUE)) {
			do sPres.push_back(fileData.cFileName);
			while (FindNextFile(hFind, &fileData));
		}
		FindClose(hFind);
		for (size_t i = 0; i < min(sOuts.size(), sPres.size()); ++i)
		{
			if (sOuts[i].substr(0, sOuts[i].size() - 4) != sPres[i].substr(0, sPres[i].size() - 4))
			{
				if (sOuts.size() > sPres.size())
				{
					sOuts.erase(sOuts.begin() + i);
					continue;
				}
				if (sOuts.size() < sPres.size())
				{
					sPres.erase(sPres.begin() + i);
					continue;
				}
			}
		}
		sOuts_paths.resize(sOuts.size());
		if (sOuts.size() > 0)
		{
			for (size_t i = 0; i < sOuts.size(); ++i)
			{
				sOuts_paths[i] = strdir + "\\" + sOuts[i];
			}
		}
		sPres_paths.resize(sPres.size());
		if (sPres.size() > 0)
		{
			for (size_t i = 0; i < sPres.size(); ++i)
			{
				sPres_paths[i] = strdir + "\\" + sPres[i];
			}
		}
		if (sOuts.size() > 0 && sPres.size() > 0)
		{
			cntOutFiles = sOuts.size();
			cntPreFiles = sPres.size();
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
	if (sOuts.size() > 0 && sOuts.size() == sPres.size())
	{
		ParseFekoFile parseFeko;
		antennas.clear();
		
		std::string experiment_name = filesDirectory + "\\comment.txt";
		parseFeko.ParseFileComment(experiment_name, *(pExperiment.get()));

		antennas.resize(sOuts.size());
		for (size_t i = 0; i<sOuts.size(); ++i)
		{
			antennas[i].aborted = false;

			cout << sPres[i] << endl;
			parseFeko.ParseFilePre(sPres_paths[i], antennas[i]);

			if (antennas[i].aborted)
			{
				cout << "File " << sPres[i] << " is aborted!" << endl;
				continue;
			}

			cout << sOuts[i] << endl;
			parseFeko.ParseFileOut(sOuts_paths[i], antennas[i]);

			if (antennas[i].aborted) 
			{
				cout << "File " << sOuts[i] << " is aborted!" << endl;
				continue;
			}
		}

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
			//clock_t tStart = clock();
			if (!antennas[i].aborted)
			{
				if (sOuts.size() > 0)
				{
					cout << sOuts[i].substr(0, sOuts[i].length()-4) << endl;
				}
				pFBDataBase->WriteAntennaData(antennas[i], resId);
			}
			//cout << (double)(clock() - tStart) / CLOCKS_PER_SEC << endl;
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