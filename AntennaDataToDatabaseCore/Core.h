#ifndef CORE_H
#define CORE_H

#pragma warning(disable:4251)

#include <memory>
#include <vector>
#include "Core_API.h"

struct Antenna;
struct Experiment;
class FrbrdDatabase;
class ParseFekoFile;

class CORE_API Core
{
public:
	Core();
	~Core();

	int DeleteExperiment(int idExp);
	int SetDonorDB();
	int ConnectDatabase(const char* pathDB = nullptr);
	int OpenDirectory(std::string strdir, int &cntOutFiles, int &cntPreFiles);
	int ReadFiles();
	int WriteData();
	int PrepareExperimentBeforeWrite();
	int Request(std::string requestStr, int countSelect, std::vector<std::vector<double>>& result);
	int GetExperiments(std::vector<int>& ids, std::vector<Experiment>& exps, bool fullComment = false);
	int GetAntennasByExperiment(std::vector<Antenna>& antennas, std::vector<int>& antennasID, int idExperiment);
	int SetData(Experiment &exp, std::vector<Antenna>& _antennas);
	std::string GetCurrentDir();
	std::string GetPathRecipient();
	std::string GetPathDonor();
	std::string GetPathCurrentDB();

private:
	std::shared_ptr<FrbrdDatabase> pFBDataBase;
	std::vector<std::string> sOuts, sPres, sOuts_paths, sPres_paths;
	std::vector<Antenna> antennas;
	std::shared_ptr<Experiment> pExperiment;
	bool donorDB;
	std::string pathDonor, pathRecipient, pathCurrentDB, filesDirectory;

	void SetPaths(std::string spath);
};

#endif // CORE_H