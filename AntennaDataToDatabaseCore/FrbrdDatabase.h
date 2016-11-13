
namespace IBPP
{
	template <class T> class Ptr;
	class IDatabase;
	typedef Ptr<IDatabase> Database;
}

struct Antenna;
struct Experiment;

class FrbrdDatabase
{
public:
	FrbrdDatabase();
	~FrbrdDatabase();

	/// @brief ������������� �������
	int Initialization(std::string server, std::string path, std::string login, std::string password);

	/// @brief C������� ���������� � ����� ������
	int CreateConnection(const std::string& server_, const std::string& path_, const std::string& login_, const std::string& password_);

	/// @brief ������ ������� � ���� ������
	int WriteAntennaData(Antenna &_antenna, int idExperiment);

	/// @brief ������ ������������ � ���� ������
	int WriteExperiment(Experiment *pExperiment=nullptr);

	/// @brief ���������� select
	int Request(std::string requestStr, int countSelect, std::vector<std::vector<double>>& result);

	/// @brief ��������� ���� ������������� �� ��
	int GetExperiments(std::vector<int>& ids, std::vector<Experiment>& exps, bool fullComment);

	/// @brief ��������� ���� ������ �� ID ������������ �� ��
	int GetAntennas(std::vector<Antenna>& antennas, std::vector<int>& antennasID, int idExperiment);
	
private:
	IBPP::Database* dataBase_;
};