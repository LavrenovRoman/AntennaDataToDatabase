#include <string>
#include <vector>

struct Antenna;
struct Experiment;
enum TYPE;

class ParseFekoFile
{
public:
	ParseFekoFile(); 
	std::string dtos(double &d);
	std::string itos(int &i);
	void ParseFileComment(const std::string &_file, Experiment& _experiment);
	void ParseFileOut(const std::string &_file, Antenna& _antenna);
	void ParseFilePre(const std::string &_file, Antenna& _antenna);
	void CreateFilePre(Antenna& _antenna, std::string _dir);

private:
	std::vector<std::string> vsOut, vsPre;
};