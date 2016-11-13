
class QString;

struct Antenna;
struct Experiment;
enum TYPE;

class ParseFekoFile
{
public:
	void ParseFileComment(QString _file, Experiment& _experiment);
	void ParseFileOut(QString _file, Antenna& _antenna);
	void ParseFilePre(QString _file, Antenna& _antenna);

private:
	TYPE _type;
};