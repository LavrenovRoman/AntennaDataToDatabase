#ifndef CORE_H
#define CORE_H

#include <memory>
#include <vector>
#include "Core_API.h"
#include <QStringList>

struct Antenna;
struct Experiment;
class FrbrdDatabase;
class ParseFekoFile;

class QString;


class CORE_API Core
{
public:
	Core();
	~Core();

	int SetDonorDB();
	int ConnectDatabase();
	int OpenDirectory(QString strdir, int &cntOutFiles, int &cntPreFiles);
	int ReadFiles();
	int WriteData();
	int PrepareExperimentBeforeWrite();
	int Request(std::string requestStr, int countSelect, std::vector<std::vector<double>>& result);
	int GetExperiments(std::vector<int>& ids, std::vector<Experiment>& exps, bool fullComment = false);
	int GetAntennasByExperiment(std::vector<Antenna>& antennas, std::vector<int>& antennasID, int idExperiment);
	int SetData(Experiment &exp, std::vector<Antenna>& _antennas);

private:

	std::shared_ptr<FrbrdDatabase> pFBDataBase;
	QStringList outs, pres;
	QStringList out_names, pre_names;
	std::vector<Antenna> antennas;
	std::shared_ptr<Experiment> pExperiment;
	bool donorDB;
};

#endif // CORE_H