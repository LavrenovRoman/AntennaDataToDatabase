// AntennaDataToDatabaseCmd.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <iostream>

using namespace std;

#include "Core.h"

#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	int cntOutFiles = 0;
	int cntPreFiles = 0;

	QString strdir = QString::fromWCharArray(argv[1]);

	Core core;
	if (core.ConnectDatabase()==0) {
		if (core.OpenDirectory(strdir, cntOutFiles, cntPreFiles)==0) {
			if (core.ReadFiles()==0) {
				if (core.WriteData()==0)
				{
					cout << "Success!" << endl;
					return 0;
				}}}}
	cout << "Exit with Error!" << endl;
	return 0;
}

