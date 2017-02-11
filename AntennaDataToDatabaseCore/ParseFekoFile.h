#include <string>
#include <vector>

struct Antenna;
struct Experiment;
enum TYPE;

class ParseFekoFile
{
public:
	ParseFekoFile();
	void ParseFileComment(const std::string &_file, Experiment& _experiment);
	void ParseFileOut(const std::string &_file, Antenna& _antenna);
	void ParseFilePre(const std::string &_file, Antenna& _antenna);

private:
	std::vector<std::string> vsOut, vsPre;
};