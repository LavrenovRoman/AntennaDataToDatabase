#include <QSettings>
#include "ParseFekoFile.h"
#include "FrbrdDatabase.h"

class Core

{
public:
	Core();
	~Core();

	int ConnectDatabase();
	int OpenDirectory(QString strdir, int &cntOutFiles, int &cntPreFiles);
	int ReadFiles();
	int WriteData();

private:

	FrbrdDatabase FBDataBase;
	QStringList outs, pres;
	QStringList out_names, pre_names;
	std::vector<Antenna> antennas;
};