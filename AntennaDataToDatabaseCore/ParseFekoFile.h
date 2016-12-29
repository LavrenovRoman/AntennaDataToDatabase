#include <string>

struct Antenna;
struct Experiment;
enum TYPE;

class ParseFekoFile
{
public:
	void ParseFileComment(std::string _file, Experiment& _experiment);
	void ParseFileOut(std::string _file, Antenna& _antenna);
	void ParseFilePre(std::string _file, Antenna& _antenna);

private:
	TYPE _type;
};