#include "correlat.h"
#include "math.h"

double Correlat::koefKorr(const std::vector<double> &x, const std::vector<double> &y)
{
    if (x.size() != y.size()) return 0.0;
    double x_ = 0.0;
    double y_ = 0.0;
    int n = x.size();
	for (size_t i = 0; i < n; i++)
    {
        x_ += x[i];
        y_ += y[i];
    }
    x_ /= n;
    y_ /= n;

    double xy = 0;
    double x2 = 0;
    double y2 = 0;
	for (size_t i = 0; i < n; i++)
    {
        double x1 = x[i] - x_;
        double y1 = y[i] - y_;
        xy += x1 * y1;
        x2 += x1 * x1;
        y2 += y1 * y1;
    }
    return xy / sqrt(x2 * y2);
}

// y = a + b * x
void Correlat::RegressLine(const std::vector<double> &x, const std::vector<double> &y, double& a, double& b, double& eps, double& A)
{
    double x_ = 0.0;
    double y_ = 0.0;
    double xy_ = 0.0;
    double x2_ = 0.0;
    int n = x.size();
	for (size_t i = 0; i < n; i++)
    {
        x_ += x[i];
        y_ += y[i];
        xy_ += x[i] * y[i];
        x2_ += x[i] * x[i];
    }
    x_ /= n;
    y_ /= n;
    xy_ /= n;
    x2_ /= n;

    b = (xy_ - x_ * y_) / (x2_ - x_ * x_);
    a = y_ - b * x_;

    eps = 0.0;
    A = 0.0;
	for (size_t i = 0; i < n; i++)
    {
        double pogr = a + b * x[i] - y[i];
        eps += pogr * pogr;
        if (y[i]!=0.0) A += abs(pogr / y[i]);
        else A += abs(pogr / y_);
    }
    eps = sqrt(eps / n);
    A = A / n * 100;
}

// y = a0 + a1 * x + a2 * x * x
void Correlat::RegressParabol(const std::vector<double> &x, const std::vector<double> &y, double& a0, double& a1, double& a2, double& eps, double& A)
{
    double Sx = 0.0;
    double Sx2 = 0.0;
    double Sx3 = 0.0;
    double Sx4 = 0.0;
    double Sy = 0.0;
    double Sxy = 0.0;
    double Sx2y = 0.0;
	size_t n = x.size();
	for (size_t i = 0; i < n; i++)
    {
        double xx = x[i];
        Sx += xx;
        xx *= xx;
        Sx2 += xx;
        xx *= x[i];
        Sx3 += xx;
        xx *= x[i];
        Sx4 += xx;
        xx = y[i];
        Sy += xx;
        xx *= x[i];
        Sxy += xx;
        xx *= x[i];
        Sx2y += xx;
    }

    a0 = Sx3 * Sx3 * Sy - Sx2 * (Sx3 * Sxy + Sx4 * Sy) + Sx2 * Sx2 * Sx2y + Sx * (Sx4 * Sxy - Sx3 * Sx2y);
    a1 = Sx2 * Sx2 * Sxy + Sx4 * (-n * Sxy + Sx * Sy) + n * Sx3 * Sx2y - Sx2 * (Sx3 * Sy + Sx * Sx2y);
    a2 = n * Sx3 * Sxy - Sx * (Sx2 * Sxy + Sx3 * Sy) + Sx * Sx * Sx2y + Sx2 * (Sx2 * Sy - n * Sx2y);
    double d = Sx2 * Sx2 * Sx2 + n * Sx3 * Sx3 + Sx * Sx * Sx4 - Sx2 * (2.0 * Sx * Sx3 + n * Sx4);
    a0 /= d;
    a1 /= d;
    a2 /= d;

    Sy /= n;
    eps = 0.0;
    A = 0.0;
	for (size_t i = 0; i < n; i++)
    {
        double pogr = a0 + a1 * x[i] + a2 * x[i] * x[i] - y[i];
        eps += pogr * pogr;
        if (y[i] != 0.0) A += abs(pogr / y[i]);
        else A += abs(pogr / Sy);
    }
    eps = sqrt(eps / n);
    A = A / n * 100;

}

// y = a + b * ln x
void Correlat::RegressLn(const std::vector<double> &x, const std::vector<double> &y, double& a, double& b)
{
    double lnx_ = 0.0;
    double y_ = 0.0;
    double lnxy_ = 0.0;
    double lnx2_ = 0.0;
	size_t n = x.size();
	for (size_t i = 0; i < n; i++)
    {
        double lnx = log(x[i]);
        lnx_ += lnx;
        y_ += y[i];
        lnxy_ += lnx * y[i];
        lnx2_ += lnx * lnx;
    }
    //lnx_ /= n;
    //y_ /= n;
    //lnxy_ /= n;
    //lnx2_ /= n;

    b = (n * lnxy_ - lnx_ * y_) / (n * lnx2_ - lnx_ * lnx_);
    a = (y_ - b * lnx_) / n;
}