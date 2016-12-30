// AntennaDataToDatabaseCmd.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <locale>
#include <codecvt>

//#include <QString>

using namespace std;

#include "Core.h"

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	int cntOutFiles = 0;
	int cntPreFiles = 0;

	//QString strdir = QString::fromWCharArray(argv[1]);
	std::wstring wdir(argv[1]);
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::string sdir = converter.to_bytes(wdir);

	Core core;
	if (core.ConnectDatabase()==0) {
		if (core.OpenDirectory(sdir/*strdir.toStdString()*/, cntOutFiles, cntPreFiles) == 0) {
			if (core.ReadFiles()==0) {
				if (core.PrepareExperimentBeforeWrite() == 0 && core.WriteData() == 0)
				{
					cout << "Success!" << endl;
					system("pause");
					return 0;
				}}}}
	cout << "Exit with Error!" << endl;
	system("pause");
	return 1;
}

