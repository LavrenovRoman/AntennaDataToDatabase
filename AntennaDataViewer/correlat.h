#include <vector>

class Correlat {
public:
	double koefKorr(const std::vector<double> &x, const std::vector<double> &y);
	void RegressLine(const std::vector<double> &x, const std::vector<double> &y, double& a, double& b, double& eps, double& A);
	void RegressParabol(const std::vector<double> &x, const std::vector<double> &y, double& a0, double& a1, double& a2, double& eps, double& A);
	void RegressLn(const std::vector<double> &x, const std::vector<double> &y, double& a, double& b);
};