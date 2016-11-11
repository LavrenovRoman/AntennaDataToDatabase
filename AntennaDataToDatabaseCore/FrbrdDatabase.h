
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

	/// @brief Инициализация сервера
	int Initialization(std::string server, std::string path, std::string login, std::string password);

	/// @brief Cоздание соединения с базой данных
	int CreateConnection(const std::string& server_, const std::string& path_, const std::string& login_, const std::string& password_);

	/// @brief Запись антенны в базу данных
	int WriteAntennaData(Antenna &_antenna, int idExperiment);

	/// @brief Запись эксперимента в базу данных
	int WriteExperiment(Experiment *pExperiment=nullptr);

	/// @brief Выполнения select
	int Request(std::string requestStr, int countSelect, std::vector<std::vector<double>>& result);

	/// @brief Получение всех экспериментов из БД
	int GetExperiments(std::vector<int>& ids, std::vector<Experiment>& exps, bool fullComment);

	/// @brief Получение всех антенн по ID эксперимента из БД
	int GetAntennas(std::vector<Antenna>& antennas, std::vector<int>& antennasID, int idExperiment);
	
private:
	IBPP::Database* dataBase_;
};