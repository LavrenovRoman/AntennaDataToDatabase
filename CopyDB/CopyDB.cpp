// CoreDB.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <iostream>

#include <QStringList>

using namespace std;

#include "Core.h"
#include "Antenna.h"

#if _DEBUG
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32d.lib")
#else
#pragma comment(lib, "../lib/AntennaDataToDatabaseCore32.lib")
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	QString strdir = QString::fromWCharArray(argv[1]);

	Core coreRecepient;
	Core coreDonor;
	coreDonor.SetDonorDB();
	std::vector<int> donorExpsId, recepientExpsId;
	std::vector<Experiment> donorExps, recepientExps;
	if (coreDonor.ConnectDatabase() == 0) {
		coreDonor.GetExperiments(donorExpsId, donorExps, true);
	}
	else
	{
		cout << "Exit with Error in connect to donor DB!" << endl;
		system("pause");
		return 1;
	}

	if (coreRecepient.ConnectDatabase() == 0) {
		coreRecepient.GetExperiments(recepientExpsId, recepientExps, true);
	}
	else
	{
		cout << "Exit with Error in connect to recepient DB!" << endl;
		system("pause");
		return 1;
	}

	int cnt = 0;
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
			std::vector<Antenna> antennas; 
			std::vector<int> antennasId;
			if (coreDonor.GetAntennasByExperiment(antennas, antennasId, donorExpsId[i]) == 0) {
				if (coreRecepient.SetData(donorExps[i], antennas) == 0) {
					cout << "Write experiment to DB" << endl;
					if (coreRecepient.WriteData() == 0) {
						cnt++;
					}
				}
			}
		}
	}
	cout << "Success!" << endl;
	cout << "Copied " << cnt << " experiments" << endl;
	system("pause");
	return 0;
}

