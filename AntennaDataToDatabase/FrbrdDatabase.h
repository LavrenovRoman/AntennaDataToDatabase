
namespace IBPP
{
	template <class T> class Ptr;
	class IDatabase;
	typedef Ptr<IDatabase> Database;
}

class Antenna;

class FrbrdDatabase
{
public:
	FrbrdDatabase();
	~FrbrdDatabase();

	/// @brief Инициализация сервера
	int Initialization(std::string server, std::string path, std::string login, std::string password);

	/// @brief Cоздание соединения с базой данных
	int  CreateConnection(const std::string& server_, const std::string& path_, const std::string& login_, const std::string& password_);

	/// @brief Запись антенны в базу данных
	int WriteAntennaData(Antenna &_antenna);
	
private:
	IBPP::Database* dataBase_;
};