#include <iostream>
#include "ibpp/ibpp.h"
#include "FrbrdDatabase.h"
#include <time.h>
#include "Antenna.h"
#include <sstream>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FrbrdDatabase::FrbrdDatabase()
{
	dataBase_ = new IBPP::Database;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FrbrdDatabase::~FrbrdDatabase()
{
	if ((*dataBase_) != nullptr && (*dataBase_)->Connected())
	{
		(*dataBase_)->Disconnect();
	}
	if (dataBase_ != nullptr) delete dataBase_;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IBPP::Timestamp toTimestamp(int day, int month, int year, int hour, int minute, int sec, int msec)
{
	IBPP::Timestamp res;
	res.SetDate(year, month, day);
	res.SetTime(hour, minute, sec, msec * 10);
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IBPP::Timestamp currentDateTime()
{
	time_t currentTime;
	struct tm* localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time

	int Day    = localTime->tm_mday;
	int Month  = localTime->tm_mon + 1;
	int Year   = localTime->tm_year + 1900;
	int Hour   = localTime->tm_hour;
	int Min    = localTime->tm_min;
	int Sec    = localTime->tm_sec;
	return toTimestamp(Day, Month, Year, Hour, Min, Sec, 0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string fromTimestampToString(IBPP::Timestamp& timestamp)
{
	char Buffer[256];
	sprintf_s(Buffer, sizeof(Buffer), "%02u.%02u.%04u %02u:%02u:%02u", timestamp.Day(), timestamp.Month(), timestamp.Year(),
		timestamp.Hours(), timestamp.Minutes(), timestamp.Seconds());
	return std::string(Buffer);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* GetBinDataFromBlobField(IBPP::Statement& statement, int fieldNum, size_t bytesTotal)
{
	int largestChunkSize, segmentsTotal, chunkSize, bytesBlob;
	int bytesRead = 0;
	IBPP::Blob blob = IBPP::BlobFactory(statement->DatabasePtr(), statement->TransactionPtr());
	statement->Get(fieldNum, blob);
	blob->Open();
	blob->Info(&bytesBlob, &largestChunkSize, &segmentsTotal);

	if (bytesBlob <= 1)
	{
		blob->Close();
		return 0;
	}
	char* imagePtr = new char[bytesBlob];
	char* chunkPtr = imagePtr;
	do
	{
		chunkSize = (bytesBlob - bytesRead) < largestChunkSize ?
			(bytesBlob - bytesRead) : largestChunkSize;
		bytesRead += blob->Read((void*)chunkPtr, chunkSize);
		chunkPtr += chunkSize;
	}
	while (bytesBlob != bytesRead);

	return imagePtr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteBinDataToBlobField(IBPP::Statement& statement, int fieldNum, const void* byteArray, const size_t bytesTotal)
{
	IBPP::Blob blobData = IBPP::BlobFactory(
		statement->DatabasePtr(),
		statement->TransactionPtr());

	size_t chunkSize;
	unsigned long largestChunkSize = 32 * 1024 - 1;

	size_t compbytesTotal = bytesTotal;
	size_t compSize = bytesTotal;
	size_t bytesWritten = 0;

	char* chunkPtr = (char*)byteArray;
	blobData->Create();
	if (compSize > 0)
	{
		do
		{
			chunkSize = (compbytesTotal - bytesWritten) < largestChunkSize ? (compSize - bytesWritten) : largestChunkSize;
			blobData->Write((void*)chunkPtr, (int)chunkSize);
			bytesWritten += chunkSize;
			chunkPtr += chunkSize;
		}
		while (bytesWritten != compSize);
	}
	blobData->Close();
	statement->Set(fieldNum, blobData);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Считываение параметров подключения к базе данных
int FrbrdDatabase::Initialization(const std::string &server, const std::string &path, const std::string &login, const std::string &password)
{
	if (CreateConnection(server, path, login, password) != 0)
	{
		return 1;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Cоздание соединения с базой данных
int FrbrdDatabase::CreateConnection(const std::string& server_, const std::string& path_, const std::string& login_, const std::string& password_)
{
	(*dataBase_) = nullptr;
	try
	{
		(*dataBase_) = IBPP::DatabaseFactory(server_, path_, login_, password_);
		(*dataBase_)->Connect();
		return 0;
	}
	// catch (IBPP::Exception*)
	// {
	//     if ((*dataBase_) != 0)
	//     {
	//         (*dataBase_)->Disconnect();
	//     }
	//     return E_FAIL;
	// }
	catch (...)
	{
		return 1;
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Выполнение select запроса
int FrbrdDatabase::Request(const std::string &requestStr, int countSelect, std::vector<std::vector<double>>& result)
{
	double d1, d2;
	result.clear();
	for (int i=0; i<countSelect; ++i)
	{
		std::vector<double> tmp;
		result.push_back(tmp);
	}
	if ((*dataBase_)->Connected())
	{
		{
			IBPP::Transaction trRequest = IBPP::TransactionFactory(*dataBase_);
			trRequest->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trRequest);
				st->Prepare(requestStr);
				st->Execute();
				while (st->Fetch())
				{
					st->Get(1, d1);
					st->Get(2, d2);
					result[0].push_back(d1);
					result[1].push_back(d2);
				}
				trRequest->Commit();
			}
			catch (...)
			{
				return -1;
			}
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Удаление эксперимента по ID из БД
int FrbrdDatabase::DeleteExperiment(int idExperiment)
{
	if ((*dataBase_)->Connected())
	{
		IBPP::Transaction trDelExp = IBPP::TransactionFactory(*dataBase_);
		trDelExp->Start();
		try
		{
			IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trDelExp);
			st->Prepare("delete from EXPERIMENT where EXPERIMENT.ID=?");
			st->Set(1, idExperiment);
			st->Execute();
			trDelExp->Commit();
		}
		catch (...)
		{
			return -1;
		}
	}
	else
	{
		return -2;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Получение всех экспериментов из БД
int FrbrdDatabase::GetExperiments(std::vector<int>& ids, std::vector<Experiment>& exps, bool fullComment)
{
	int id, param_size, ant_cnt;
	IBPP::Date date;
	std::string comm, word;
	std::vector<std::string> vs;

	ids.clear();
	exps.clear();
	if ((*dataBase_)->Connected())
	{
		{
			IBPP::Transaction trExperiments = IBPP::TransactionFactory(*dataBase_);
			trExperiments->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trExperiments);
				st->Prepare("select id, WRITEN_DATE, COMMENT, PARAMS_SIZE, PARAMS, ANTENNAS_COUNT from EXPERIMENT order by id");
				st->Execute();
				while (st->Fetch())
				{
					st->Get(1, id);
					st->Get(2, date);
					st->Get(3, comm);
					st->Get(4, param_size);
					char * c_param = GetBinDataFromBlobField(st, 5, param_size);
					st->Get(6, ant_cnt);
					std::string s_param(c_param, param_size);

					ids.push_back(id);
					Experiment newEx;
					if (!fullComment)
					{
						std::stringstream ss(comm);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						if (vs.back()[0] == '(' && vs.back().back() == ')')
						{
							comm.clear();
							for (size_t i = 0; i < vs.size() - 1; ++i)
							{
								if (i != 0) comm += " ";
								comm += vs[i];
							}
						}
					}

					newEx.comment = comm;
					newEx.date.tm_year = date.Year();
					newEx.date.tm_mon = date.Month();
					newEx.date.tm_mday = date.Day();
					newEx.antennas_count = ant_cnt;
					exps.push_back(newEx);

					stringstream strs(s_param);
					vs.clear();
					while (strs >> word) vs.push_back(word);
					for (size_t i = 0; i < vs.size()/4; ++i)
					{
						Experiment_Param newExp;
						newExp.name   = vs[0 + 4*i];
						newExp.pBegin = stod(vs[1 + 4*i]);
						newExp.pEnd   = stod(vs[2 + 4*i]);
						newExp.pStep  = stod(vs[3 + 4*i]);
						exps.back().cycles.push_back(newExp);
					}
					delete[] c_param;
				}
				trExperiments->Commit();
			}
			catch (...)
			{
				return -1;
			}
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Получение всех антенн по ID эксперимента из БД
int FrbrdDatabase::GetAntennas(std::vector<Antenna>& antennas, std::vector<int>& antennasID, int idExperiment)
{
	antennas.clear();
	antennasID.clear();
	int type, id;
	std::string path_pre, path_out;
	if ((*dataBase_)->Connected())
	{
		{
			IBPP::Transaction trAntennas = IBPP::TransactionFactory(*dataBase_);
			trAntennas->Start();
			try
			{
				short dip;
				int afs, tps, dms;
				int fractal_n, mpoints_size, fpoints_size, grd_size;
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trAntennas);
				st->Prepare("select type, path_out, path_pre, id, is_dipole, scalex, scaley, radius, fractal_n, fractal_m, fpoints_size, mpoints_size, points, affine, fpoints, mpoints, FEED_X, FEED_Y, S_PERMITTIVITY, S_LOSSTANGENT, S_DENSITY, S_THICKNESS, S_COORLEFTUPX, S_COORLEFTUPY, S_COORRIGHTDOWNX, S_COORRIGHTDOWNY, GROUND_SIZE, GROUND, ALL_FREQ_SIZE, TP_SIZE, DM_SIZE, ALL_FREQ, FST_S11, FST_W, FST_BANDWIDTH, SCND_S11, SCND_W, SCND_BANDWIDTH, THIRD_S11, THIRD_W, THIRD_BANDWIDTH, MU_LENGTHALLSEGMS_M, MU_SURFALLTRI_MM, MU_NUMMETALLICTRI, MU_NUMDIELECTRTRI, MU_NUMAPERTURETRI, MU_NUMGOTRI, MU_NUMWINDSCREENTRI, MU_NUMFEMSURFACETRI, MU_NUMMODALPORTTRI, MU_NUMMETALLICSEGMS, MU_NUMDIEMAGNCUBS, MU_NUMTETRAHEDRA, MU_NUMEDGESPOREGION, MU_NUMWEDGESPOREGION, MU_NUMFOCKREGIONS, MU_NUMPOLYSURFACES, MU_NUMUTDCYLINDRES, MU_NUMMETALLICEDGESMOM, MU_NUMMETALLICEDGESPO, MU_NUMDIELECTRICEDGESMOM, MU_NUMDIELECTRICEDGESPO, MU_NUMAPERTUREEDGESMOM, MU_NUMEDGESFEMMOMSURF, MU_NUMNODESBETWEENSEGMS, MU_NUMCONNECTIONPOINTS, MU_NUMDIELECTRICCUBOIDS, MU_NUMMAGNETICCUBOIDS, MU_NUMBASISFUNCTMOM, MU_NUMBASISFUNCTPO from ANTENNA where ID_EXPERIMENT = ? order by id");
				st->Set(1, idExperiment);
				st->Execute();
				while (st->Fetch())
				{
					st->Get(1, type);
					st->Get(2, path_out);
					st->Get(3, path_pre);
					st->Get(4, id);
					Antenna newAntenna;
					newAntenna.aborted = false;
					newAntenna.type = (TYPE)type;
					newAntenna.pathOut = path_out;
					newAntenna.pathPre = path_pre;
					newAntenna.inputPar.findRADIATOR = true;
					newAntenna.inputPar.findFEED = true;
					newAntenna.inputPar.findGROUND = true;
					newAntenna.inputPar.findSUBSTRATE = true;
					antennas.push_back(newAntenna);
					antennasID.push_back(id);
					Antenna & antI = antennas.back();
					st->Get(5, dip);
					dip == 0 ? antI.inputPar.isDipole = false : antI.inputPar.isDipole = true;
					st->Get(6, antI.inputPar.Radiator.ScaleX);
					st->Get(7, antI.inputPar.Radiator.ScaleY);
					st->Get(8, antI.inputPar.Radiator.Radius_StripWidth_FeedLineWidth);
					st->Get(9, antI.inputPar.Radiator.fr_N);
					st->Get(10, antI.inputPar.Radiator.fr_m);
					st->Get(11, fpoints_size);
					st->Get(12, mpoints_size);
					fractal_n = antI.inputPar.Radiator.fr_N;
					char * с_points = GetBinDataFromBlobField(st, 13, (fractal_n + 1)*sizeof(double) * 3);
					double * points = (double *)с_points;
					char * c_affine = GetBinDataFromBlobField(st, 14, fractal_n*sizeof(double) * 7);
					double * affine = (double *)c_affine;
					char * c_fpoints = GetBinDataFromBlobField(st, 15, fpoints_size);
					double * fpoints = (double *)c_fpoints;
					char * c_mpoints = GetBinDataFromBlobField(st, 16, mpoints_size);
					double * mpoints = (double *)c_mpoints;
					st->Get(17, antI.inputPar.Feed.FeedX);
					st->Get(18, antI.inputPar.Feed.FeedY);
					st->Get(19, antI.inputPar.Substrate.Permittivity);
					st->Get(20, antI.inputPar.Substrate.LossTangent);
					st->Get(21, antI.inputPar.Substrate.Density);
					st->Get(22, antI.inputPar.Substrate.Thickness);
					st->Get(23, antI.inputPar.Substrate.CoorLeftUpX);
					st->Get(24, antI.inputPar.Substrate.CoorLeftUpY);
					st->Get(25, antI.inputPar.Substrate.CoorRightDownX);
					st->Get(26, antI.inputPar.Substrate.CoorRightDownY);
					st->Get(27, grd_size);
					char * c_ground = GetBinDataFromBlobField(st, 28, grd_size);
					double * ground = (double *)c_ground;
					antI.inputPar.Radiator.fr_pT.resize(fractal_n + 1);
					antI.inputPar.Radiator.fr_pX.resize(fractal_n + 1);
					antI.inputPar.Radiator.fr_pY.resize(fractal_n + 1);
					antI.inputPar.Radiator.fr_typeD.resize(fractal_n);
					antI.inputPar.Radiator.fr_D11.resize(fractal_n);
					antI.inputPar.Radiator.fr_D12.resize(fractal_n);
					antI.inputPar.Radiator.fr_D21.resize(fractal_n);
					antI.inputPar.Radiator.fr_D22.resize(fractal_n);
					antI.inputPar.Radiator.fr_lam.resize(fractal_n);
					antI.inputPar.Radiator.fr_al.resize(fractal_n);
					antI.inputPar.Radiator.fr_pred1X.resize((fpoints_size / sizeof(double)) / 2);
					antI.inputPar.Radiator.fr_pred1Y.resize((fpoints_size / sizeof(double)) / 2);
					antI.inputPar.Radiator.fr_predmX.resize((mpoints_size / sizeof(double)) / 2);
					antI.inputPar.Radiator.fr_predmY.resize((mpoints_size / sizeof(double)) / 2);
					antI.inputPar.Ground.coordX.resize((grd_size / sizeof(double)) / 2);
					antI.inputPar.Ground.coordY.resize((grd_size / sizeof(double)) / 2);
					for (int j = 0; j <= fractal_n; ++j)
					{
						antI.inputPar.Radiator.fr_pT[j] = points[0 + j * 3];
						antI.inputPar.Radiator.fr_pX[j] = points[1 + j * 3];
						antI.inputPar.Radiator.fr_pY[j] = points[2 + j * 3];
					}
					for (int j = 0; j < fractal_n; ++j)
					{
						antI.inputPar.Radiator.fr_typeD[j] = affine[0 + j * 7];
						antI.inputPar.Radiator.fr_D11[j] = affine[1 + j * 7];
						antI.inputPar.Radiator.fr_D12[j] = affine[2 + j * 7];
						antI.inputPar.Radiator.fr_D21[j] = affine[3 + j * 7];
						antI.inputPar.Radiator.fr_D22[j] = affine[4 + j * 7];
						antI.inputPar.Radiator.fr_lam[j] = affine[5 + j * 7];
						antI.inputPar.Radiator.fr_al[j] = affine[6 + j * 7];
					}
					for (size_t j = 0; j < (fpoints_size / sizeof(double)) / 2; ++j)
					{
						antI.inputPar.Radiator.fr_pred1X[j] = fpoints[0 + j * 2];
						antI.inputPar.Radiator.fr_pred1Y[j] = fpoints[1 + j * 2];
					}
					for (size_t j = 0; j < (mpoints_size / sizeof(double)) / 2; ++j)
					{
						antI.inputPar.Radiator.fr_predmX[j] = mpoints[0 + j * 2];
						antI.inputPar.Radiator.fr_predmY[j] = mpoints[1 + j * 2];
					}
					for (size_t j = 0; j < (grd_size / sizeof(double)) / 2; ++j)
					{
						antI.inputPar.Ground.coordX[j] = ground[0 + j * 2];
						antI.inputPar.Ground.coordY[j] = ground[1 + j * 2];
					}
					delete[] с_points;
					delete[] c_affine;
					delete[] c_fpoints;
					delete[] c_mpoints;
					delete[] c_ground;


					st->Get(29, afs);
					st->Get(30, tps);
					st->Get(31, dms);
					char * c_freq = GetBinDataFromBlobField(st, 32, afs);
					double * p_freq = (double *)c_freq;
					st->Get(33, antI.outputPar.fst_s11);
					st->Get(34, antI.outputPar.fst_w);
					st->Get(35, antI.outputPar.fst_bandwidth);
					st->Get(36, antI.outputPar.scnd_s11);
					st->Get(37, antI.outputPar.scnd_w);
					st->Get(38, antI.outputPar.scnd_bandwidth);
					st->Get(39, antI.outputPar.third_s11);
					st->Get(40, antI.outputPar.third_w);
					st->Get(41, antI.outputPar.third_bandwidth);
					st->Get(42, antI.outputPar._DATA_FOR_MEMORY_USAGE.LengthAllSegms_M);
					st->Get(43, antI.outputPar._DATA_FOR_MEMORY_USAGE.SurfAllTri_MM);
					st->Get(44, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicTri);
					st->Get(45, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectrTri);
					st->Get(46, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureTri);
					st->Get(47, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumGoTri);
					st->Get(48, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumWindscreenTri);
					st->Get(49, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumFemSurfaceTri);
					st->Get(50, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumModalPortTri);
					st->Get(51, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicSegms);
					st->Get(52, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumDieMagnCubs);
					st->Get(53, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumTetrahedra);
					st->Get(54, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesPORegion);
					st->Get(55, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumWedgesPORegion);
					st->Get(56, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumFockRegions);
					st->Get(57, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumPolySurfaces);
					st->Get(58, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumUTDCylindres);
					st->Get(59, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesMoM);
					st->Get(60, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesPO);
					st->Get(61, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesMoM);
					st->Get(62, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesPO);
					st->Get(63, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureEdgesMoM);
					st->Get(64, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesFEMMomSurf);
					st->Get(65, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumNodesBetweenSegms);
					st->Get(66, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumConnectionPoints);
					st->Get(67, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricCuboids);
					st->Get(68, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumMagneticCuboids);
					st->Get(69, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctMoM);
					st->Get(70, antI.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctPO);

					int count_freq = afs / sizeof(double);
					int one_frq_size = 53 + 12 * tps + 6 * dms;
					count_freq /= one_frq_size;
					antI.outputPar._VEC_DATA_FOR_ONE_FREQ.resize(count_freq);

					for (int j = 0; j < count_freq; ++j)
					{
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.resize(tps);
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.resize(dms);

						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.Frequency = p_freq[0 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.Wavelength = p_freq[1 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.OpenCircuitVoltage = p_freq[2 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.Phase = p_freq[3 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.ElectricalEdgeLength = p_freq[4 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.SourceSegmLabel = static_cast<int>(p_freq[5 + j*one_frq_size]);
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.AbsolNumSegms = static_cast<int>(p_freq[6 + j*one_frq_size]);
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitX = p_freq[7 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitY = p_freq[8 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitZ = p_freq[9 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirX = p_freq[10 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirY = p_freq[11 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirZ = p_freq[12 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SPortSinc = static_cast<int>(p_freq[13 + j*one_frq_size]);
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SPortSource = static_cast<int>(p_freq[14 + j*one_frq_size]);
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SRealPart = p_freq[15 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SImagPart = p_freq[16 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SMagnitudeLinear = p_freq[17 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SMagnitudeDB = p_freq[18 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SPhase = p_freq[19 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SumS2MagnitudeLinear = p_freq[20 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SumS2MagnitudeDB = p_freq[21 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.S11 = p_freq[22 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.VSWR = p_freq[23 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.EMAX = p_freq[24 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.MetallicElements = p_freq[25 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.MismatchFeed = p_freq[26 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.NonRadiatingNetworks = p_freq[27 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.BackPowerPassWaveguidePorts = p_freq[28 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.BackPowerPassModalPorts = p_freq[29 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.SumAllLosses = p_freq[30 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.EfficiencyTheAntenna = p_freq[31 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.BasedTotalActivePower = p_freq[32 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentRealPart = p_freq[33 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentImagPart = p_freq[34 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentMagnitude = p_freq[35 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentPhase = p_freq[36 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittRealPart = p_freq[37 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittImagPart = p_freq[38 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittMagnitude = p_freq[39 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittPhase = p_freq[40 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceRealPart = p_freq[41 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceImagPart = p_freq[42 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceMagnitude = p_freq[43 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedancePhase = p_freq[44 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.Inductance = p_freq[45 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.Capacitance = p_freq[46 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DATA_OF_THE_VOLTAGE_SOURCE.Power = p_freq[47 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.GainPower = p_freq[48 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss = p_freq[49 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.PowerLoss = p_freq[50 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.MaximumSARValue = p_freq[51 + j*one_frq_size];
						antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.AveragedSARValue = p_freq[52 + j*one_frq_size];
						int s = 53 + j*one_frq_size;
						for (int k = 0; k < tps; ++k)
						{
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].Tetta = p_freq[s + 0 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].Phi = p_freq[s + 1 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].EthetaMagn = p_freq[s + 2 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].EthetaPhase = p_freq[s + 3 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].EphiMagn = p_freq[s + 4 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].EphiPhase = p_freq[s + 5 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].DirectivityVert = p_freq[s + 6 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].DirectivityHoriz = p_freq[s + 7 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].DirectivityTotal = p_freq[s + 8 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].PolarizationAxial = p_freq[s + 9 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].PolarizationAngle = p_freq[s + 10 + k * 12];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[k].Gain = p_freq[s + 11 + k * 12];
						}
						s = 53 + 12 * tps;
						for (int k = 0; k < dms; ++k)
						{
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA[k].RelPermittivity = p_freq[s + 0 + k * 6];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA[k].RelPermeability = p_freq[s + 1 + k * 6];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA[k].Conductivity = p_freq[s + 2 + k * 6];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA[k].TanDeltaElectric = p_freq[s + 3 + k * 6];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA[k].TanDeltaMagnetic = p_freq[s + 4 + k * 6];
							antI.outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA[k].MassDensity = p_freq[s + 5 + k * 6];
						}
					}

					delete[] c_freq;
				}
				trAntennas->Commit();
			}
			catch (...)
			{
				return -1;
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Запись эксперимента в базу данных
int FrbrdDatabase::WriteExperiment(Experiment *pExperiment)
{
	int idExperiment = -1;
	if ((*dataBase_)->Connected())
	{
		{
			IBPP::Transaction trExperiment = IBPP::TransactionFactory(*dataBase_);
			trExperiment->Start();
			try
			{
				std::string s_params;
				for (size_t i = 0; i < pExperiment->cycles.size(); ++i)
				{
					s_params += pExperiment->cycles[i].name;
					s_params += " ";
					std::ostringstream strs;
					strs << pExperiment->cycles[i].pBegin;
					strs << " ";
					strs << pExperiment->cycles[i].pEnd;
					strs << " ";
					strs << pExperiment->cycles[i].pStep;
					strs << " ";
					s_params += strs.str();
					s_params += " ";
				}

				char * c_params = new char[s_params.size()];
				for (size_t i = 0; i < s_params.size(); ++i)
				{
					c_params[i] = s_params[i];
				}

				IBPP::Date dt;
				dt.SetDate(pExperiment->date.tm_year, pExperiment->date.tm_mon, pExperiment->date.tm_mday);
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trExperiment);
				st->Prepare("insert into EXPERIMENT (WRITEN_DATE, COMMENT, PARAMS, PARAMS_SIZE, ANTENNAS_COUNT) values (?, ?, ?, ?, ?) returning id");
				st->Set(1, dt);
				st->Set(2, pExperiment->comment);
				WriteBinDataToBlobField(st, 3, c_params, s_params.size());
				st->Set(4, (int)s_params.size());
				st->Set(5, (int)pExperiment->antennas_count);
				st->Execute();
				st->Get(1, idExperiment);
				trExperiment->Commit();

				delete[] c_params;
			}
			catch (...)
			{
				trExperiment->Rollback();
			}
		}
	}
	return idExperiment;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Запись антенны в базу данных
int FrbrdDatabase::WriteAntennaData(Antenna &_antenna, int idExperiment)
{
	//clock_t tStart;// , tStep;

	int idAntenna          = -1;
	int idOutputPar        = -1; 
	int idInputPar         = -1;
	int idOutputParOneFreq = -1;
	if ((*dataBase_)->Connected())
	{
		//tStart = clock();
		int size_fp = sizeof(double) * (_antenna.inputPar.Radiator.fr_N + 1) * 3;
		double * p_fp = new double[(_antenna.inputPar.Radiator.fr_N + 1) * 3];
		int size_at = sizeof(double) * _antenna.inputPar.Radiator.fr_N * 7;
		double * p_at = new double[_antenna.inputPar.Radiator.fr_N * 7];
		int size_f1p = sizeof(double) * _antenna.inputPar.Radiator.fr_pred1X.size() * 2;
		double * p_f1p = new double[_antenna.inputPar.Radiator.fr_pred1X.size() * 2];
		int size_fmp = sizeof(double) * _antenna.inputPar.Radiator.fr_predmX.size() * 2;
		double * p_fmp = new double[_antenna.inputPar.Radiator.fr_predmX.size() * 2];
		int size_grd = sizeof(double) * _antenna.inputPar.Ground.coordX.size() * 2;
		double * p_grd = new double[_antenna.inputPar.Ground.coordX.size() * 2];
		for (int j = 0; j <= _antenna.inputPar.Radiator.fr_N; ++j)
		{
			p_fp[0 + j * 3] = _antenna.inputPar.Radiator.fr_pT[j];
			p_fp[1 + j * 3] = _antenna.inputPar.Radiator.fr_pX[j];
			p_fp[2 + j * 3] = _antenna.inputPar.Radiator.fr_pY[j];
		}
		for (int j = 0; j < _antenna.inputPar.Radiator.fr_N; ++j)
		{
			p_at[0 + j * 7] = _antenna.inputPar.Radiator.fr_typeD[j];
			p_at[1 + j * 7] = _antenna.inputPar.Radiator.fr_D11[j];
			p_at[2 + j * 7] = _antenna.inputPar.Radiator.fr_D12[j];
			p_at[3 + j * 7] = _antenna.inputPar.Radiator.fr_D21[j];
			p_at[4 + j * 7] = _antenna.inputPar.Radiator.fr_D22[j];
			p_at[5 + j * 7] = _antenna.inputPar.Radiator.fr_lam[j];
			p_at[6 + j * 7] = _antenna.inputPar.Radiator.fr_al[j];
		}
		for (size_t j = 0; j < _antenna.inputPar.Radiator.fr_pred1X.size(); ++j)
		{
			p_f1p[0 + j * 2] = _antenna.inputPar.Radiator.fr_pred1X[j];
			p_f1p[1 + j * 2] = _antenna.inputPar.Radiator.fr_pred1Y[j];
		}
		for (size_t j = 0; j < _antenna.inputPar.Radiator.fr_predmX.size(); ++j)
		{
			p_fmp[0 + j * 2] = _antenna.inputPar.Radiator.fr_predmX[j];
			p_fmp[1 + j * 2] = _antenna.inputPar.Radiator.fr_predmY[j];
		}
		for (size_t j = 0; j < _antenna.inputPar.Ground.coordX.size(); ++j)
		{
			p_grd[0 + j * 2] = _antenna.inputPar.Ground.coordX[j];
			p_grd[1 + j * 2] = _antenna.inputPar.Ground.coordY[j];
		}
		char * cp_fp = (char *)p_fp;
		char * cp_at = (char *)p_at;
		char * cp_f1p = (char *)p_f1p;
		char * cp_fmp = (char *)p_fmp;
		char * cp_grd = (char *)p_grd;

		//std::cout << "input: " << (double)(clock() - tStart) / CLOCKS_PER_SEC << endl;
		//tStart = clock();
		int one_frq_size = 53 + (_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[0]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.size() * 12) + _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[0]._VEC_DATA_FOR_DIELECTRIC_MEDIA.size() * 6;
		int freq_data_size = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.size() * one_frq_size;
		double * p_freq = new double[freq_data_size];
		freq_data_size *= sizeof(double);
		for (size_t i = 0; i < _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++i)
		{
			p_freq[0  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.Frequency;
			p_freq[1  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.Wavelength;
			p_freq[2  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.OpenCircuitVoltage;
			p_freq[3  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.Phase;
			p_freq[4  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.ElectricalEdgeLength;
			p_freq[5  + i*one_frq_size] = static_cast<double>(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.SourceSegmLabel);
			p_freq[6  + i*one_frq_size] = static_cast<double>(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.AbsolNumSegms);
			p_freq[7  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitX;
			p_freq[8  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitY;
			p_freq[9  + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitZ;
			p_freq[10 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirX;
			p_freq[11 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirY;
			p_freq[12 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirZ;
			p_freq[13 + i*one_frq_size] = static_cast<double>(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SPortSinc);
			p_freq[14 + i*one_frq_size] = static_cast<double>(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SPortSource);
			p_freq[15 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SRealPart;
			p_freq[16 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SImagPart;
			p_freq[17 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SMagnitudeLinear;
			p_freq[18 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SMagnitudeDB;
			p_freq[19 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SPhase;
			p_freq[20 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SumS2MagnitudeLinear;
			p_freq[21 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SumS2MagnitudeDB;
			p_freq[22 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.S11;
			p_freq[23 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.VSWR;
			p_freq[24 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.EMAX;
			p_freq[25 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.MetallicElements;
			p_freq[26 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.MismatchFeed;
			p_freq[27 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.NonRadiatingNetworks;
			p_freq[28 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.BackPowerPassWaveguidePorts;
			p_freq[29 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.BackPowerPassModalPorts;
			p_freq[30 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.SumAllLosses;
			p_freq[31 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.EfficiencyTheAntenna;
			p_freq[32 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.BasedTotalActivePower;
			p_freq[33 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentRealPart;
			p_freq[34 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentImagPart;
			p_freq[35 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentMagnitude;
			p_freq[36 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.CurrentPhase;
			p_freq[37 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittRealPart;
			p_freq[38 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittImagPart;
			p_freq[39 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittMagnitude;
			p_freq[40 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.AdmittPhase;
			p_freq[41 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceRealPart;
			p_freq[42 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceImagPart;
			p_freq[43 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceMagnitude;
			p_freq[44 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.ImpedancePhase;
			p_freq[45 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.Inductance;
			p_freq[46 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.Capacitance;
			p_freq[47 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DATA_OF_THE_VOLTAGE_SOURCE.Power;
			p_freq[48 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.GainPower;
			p_freq[49 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss;
			p_freq[50 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.PowerLoss;
			p_freq[51 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.MaximumSARValue;
			p_freq[52 + i*one_frq_size] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.AveragedSARValue;
			int s = 53 + i*one_frq_size;
			for (size_t j = 0; j < _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.size(); ++j)
			{
				p_freq[s + 0 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].Tetta;
				p_freq[s + 1 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].Phi;
				p_freq[s + 2 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EthetaMagn;
				p_freq[s + 3 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EthetaPhase;
				p_freq[s + 4 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EphiMagn;
				p_freq[s + 5 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EphiPhase;
				p_freq[s + 6 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].DirectivityVert;
				p_freq[s + 7 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].DirectivityHoriz;
				p_freq[s + 8 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].DirectivityTotal;
				p_freq[s + 9 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].PolarizationAxial;
				p_freq[s + 10 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].PolarizationAngle;
				p_freq[s + 11 + j * 12] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].Gain;
			}
			s = 53 + 12*_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.size();
			for (size_t j = 0; j < _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA.size(); ++j)
			{
				p_freq[s + 0 + j * 6] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].RelPermittivity;
				p_freq[s + 1 + j * 6] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].RelPermeability;
				p_freq[s + 2 + j * 6] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].Conductivity;
				p_freq[s + 3 + j * 6] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].TanDeltaElectric;
				p_freq[s + 4 + j * 6] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].TanDeltaMagnetic;
				p_freq[s + 5 + j * 6] = _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].MassDensity;
			}
		}
		char * cp_freq = (char *) p_freq;

		{
			IBPP::Transaction trAntenna = IBPP::TransactionFactory(*dataBase_);
			trAntenna->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trAntenna);
				st->Prepare("insert into antenna (ID_EXPERIMENT, type, path_out, path_pre, IS_DIPOLE, SCALEX, SCALEY, RADIUS, FRACTAL_N, FRACTAL_M, POINTS, AFFINE, FPOINTS, FPOINTS_SIZE, MPOINTS, MPOINTS_SIZE, FEED_X, FEED_Y, S_PERMITTIVITY, S_LOSSTANGENT, S_DENSITY, S_THICKNESS, S_COORLEFTUPX, S_COORLEFTUPY, S_COORRIGHTDOWNX, S_COORRIGHTDOWNY, GROUND_SIZE, GROUND, ALL_FREQ_SIZE, TP_SIZE, DM_SIZE, ALL_FREQ, FST_S11, FST_W, FST_BANDWIDTH, SCND_S11, SCND_W, SCND_BANDWIDTH, THIRD_S11, THIRD_W, THIRD_BANDWIDTH, MU_LENGTHALLSEGMS_M, MU_SURFALLTRI_MM, MU_NUMMETALLICTRI, MU_NUMDIELECTRTRI, MU_NUMAPERTURETRI, MU_NUMGOTRI, MU_NUMWINDSCREENTRI, MU_NUMFEMSURFACETRI, MU_NUMMODALPORTTRI, MU_NUMMETALLICSEGMS, MU_NUMDIEMAGNCUBS, MU_NUMTETRAHEDRA, MU_NUMEDGESPOREGION, MU_NUMWEDGESPOREGION, MU_NUMFOCKREGIONS, MU_NUMPOLYSURFACES, MU_NUMUTDCYLINDRES, MU_NUMMETALLICEDGESMOM, MU_NUMMETALLICEDGESPO, MU_NUMDIELECTRICEDGESMOM, MU_NUMDIELECTRICEDGESPO, MU_NUMAPERTUREEDGESMOM, MU_NUMEDGESFEMMOMSURF, MU_NUMNODESBETWEENSEGMS, MU_NUMCONNECTIONPOINTS, MU_NUMDIELECTRICCUBOIDS, MU_NUMMAGNETICCUBOIDS, MU_NUMBASISFUNCTMOM, MU_NUMBASISFUNCTPO) values (?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?) returning id");
				st->Set(1, idExperiment);
				st->Set(2, _antenna.type);
				st->Set(3, _antenna.pathOut);
				st->Set(4, _antenna.pathPre);
				st->Set(5, static_cast<short>(_antenna.inputPar.isDipole));
				st->Set(6, _antenna.inputPar.Radiator.ScaleX);
				st->Set(7, _antenna.inputPar.Radiator.ScaleY);
				st->Set(8, _antenna.inputPar.Radiator.Radius_StripWidth_FeedLineWidth);
				st->Set(9, _antenna.inputPar.Radiator.fr_N);
				st->Set(10, _antenna.inputPar.Radiator.fr_m);
				WriteBinDataToBlobField(st, 11, cp_fp, size_fp);
				WriteBinDataToBlobField(st, 12, cp_at, size_at);
				WriteBinDataToBlobField(st, 13, cp_f1p, size_fmp);
				st->Set(14, size_f1p);
				WriteBinDataToBlobField(st, 15, cp_fmp, size_fmp);
				st->Set(16, size_fmp);
				st->Set(17, _antenna.inputPar.Feed.FeedX);
				st->Set(18, _antenna.inputPar.Feed.FeedY);
				st->Set(19, _antenna.inputPar.Substrate.Permittivity);
				st->Set(20, _antenna.inputPar.Substrate.LossTangent);
				st->Set(21, _antenna.inputPar.Substrate.Density);
				st->Set(22, _antenna.inputPar.Substrate.Thickness);
				st->Set(23, _antenna.inputPar.Substrate.CoorLeftUpX);
				st->Set(24, _antenna.inputPar.Substrate.CoorLeftUpY);
				st->Set(25, _antenna.inputPar.Substrate.CoorRightDownX);
				st->Set(26, _antenna.inputPar.Substrate.CoorRightDownY);
				st->Set(27, size_grd);
				WriteBinDataToBlobField(st, 28, cp_grd, size_grd);
				st->Set(29, freq_data_size);
				st->Set(30, (int)_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[0]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.size());
				st->Set(31, (int)_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[0]._VEC_DATA_FOR_DIELECTRIC_MEDIA.size());
				WriteBinDataToBlobField(st, 32, cp_freq, freq_data_size);
				st->Set(33, _antenna.outputPar.fst_s11);
				st->Set(34, _antenna.outputPar.fst_w);
				st->Set(35, _antenna.outputPar.fst_bandwidth);
				st->Set(36, _antenna.outputPar.scnd_s11);
				st->Set(37, _antenna.outputPar.scnd_w);
				st->Set(38, _antenna.outputPar.scnd_bandwidth);
				st->Set(39, _antenna.outputPar.third_s11);
				st->Set(40, _antenna.outputPar.third_w);
				st->Set(41, _antenna.outputPar.third_bandwidth);
				st->Set(42, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.LengthAllSegms_M);
				st->Set(43, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.SurfAllTri_MM);
				st->Set(44, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicTri);
				st->Set(45, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectrTri);
				st->Set(46, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureTri);
				st->Set(47, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumGoTri);
				st->Set(48, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWindscreenTri);
				st->Set(49, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFemSurfaceTri);
				st->Set(50, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumModalPortTri);
				st->Set(51, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicSegms);
				st->Set(52, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDieMagnCubs);
				st->Set(53, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumTetrahedra);
				st->Set(54, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesPORegion);
				st->Set(55, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWedgesPORegion);
				st->Set(56, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFockRegions);
				st->Set(57, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumPolySurfaces);
				st->Set(58, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumUTDCylindres);
				st->Set(59, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesMoM);
				st->Set(60, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesPO);
				st->Set(61, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesMoM);
				st->Set(62, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesPO);
				st->Set(63, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureEdgesMoM);
				st->Set(64, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesFEMMomSurf);
				st->Set(65, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumNodesBetweenSegms);
				st->Set(66, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumConnectionPoints);
				st->Set(67, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricCuboids);
				st->Set(68, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMagneticCuboids);
				st->Set(69, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctMoM);
				st->Set(70, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctPO);
				st->Execute();
				st->Get(1, idAntenna);
				trAntenna->Commit();
			}
			catch (...)
			{
				trAntenna->Rollback();
				return -1;
			}
		}

		delete[] p_fp;
		delete[] p_at;
		delete[] p_f1p;
		delete[] p_fmp;
		delete[] p_grd;
		delete[] p_freq;
		
		//std::cout << "output: " << (double)(clock() - tStart) / CLOCKS_PER_SEC << endl;
	}
	else
	{
		return 1;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////