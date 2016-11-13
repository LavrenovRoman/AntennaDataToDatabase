#include <vector>

class Correlat {
public:
	double koefKorr(std::vector<double> x, std::vector<double> y);
	void RegressLine(std::vector<double> x, std::vector<double> y, double& a, double& b, double& eps, double& A);
	void RegressParabol(std::vector<double> x, std::vector<double> y, double& a0, double& a1, double& a2, double& eps, double& A);
	void RegressLn(std::vector<double> x, std::vector<double> y, double& a, double& b);
};