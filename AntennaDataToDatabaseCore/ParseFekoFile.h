#include <string>
#include <vector>

struct Antenna;
struct Experiment;
enum TYPE;

class ParseFekoFile
{
public:
	ParseFekoFile();
	void ParseFileComment(std::string _file, Experiment& _experiment);
	void ParseFileOut(std::string _file, Antenna& _antenna);
	void ParseFilePre(std::string _file, Antenna& _antenna);

private:
	TYPE _type;
	std::vector<std::string> vsOut, vsPre;
};