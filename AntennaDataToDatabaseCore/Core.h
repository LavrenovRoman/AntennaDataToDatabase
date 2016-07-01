#include <memory>
#include <vector>
#include <QStringList>
#include "Core_API.h"

struct Antenna;
class FrbrdDatabase;
class ParseFekoFile;

class CORE_API Core
{
public:
	Core();
	~Core();

	int ConnectDatabase();
	int OpenDirectory(QString strdir, int &cntOutFiles, int &cntPreFiles);
	int ReadFiles();
	int WriteData();

private:

	std::shared_ptr<FrbrdDatabase> pFBDataBase;
	QStringList outs, pres;
	QStringList out_names, pre_names;
	std::vector<Antenna> antennas;
};