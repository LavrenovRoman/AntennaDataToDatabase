#include <QString>

struct Antenna;
enum TYPE;

class ParseFekoFile
{
public:
	void ParseFileOut(QString _file, Antenna& _antenna);
	void ParseFilePre(QString _file, Antenna& _antenna);

private:
	TYPE _type;
};