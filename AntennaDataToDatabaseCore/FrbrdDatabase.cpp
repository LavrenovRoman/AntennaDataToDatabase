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
int FrbrdDatabase::Initialization(std::string server, std::string path, std::string login, std::string password)
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
int FrbrdDatabase::Request(std::string requestStr, int countSelect, std::vector<std::vector<double>>& result)
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
	int id;
	IBPP::Date date;
	std::string comm, name, word;
	double b,e,s;

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
				st->Prepare("select id, WRITEN_DATE, COMMENT from EXPERIMENT order by id");
				st->Execute();
				while (st->Fetch())
				{
					st->Get(1, id);
					st->Get(2, date);
					st->Get(3, comm);
					ids.push_back(id);
					Experiment newEx;
					if (!fullComment)
					{
						std::vector<std::string> vs;
						std::stringstream ss(comm);
						while (ss >> word) vs.push_back(word);
						if (vs.back()[0] == '(' && vs.back().back() == ')')
						{
							comm = "";
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
					exps.push_back(newEx);
				}
				trExperiments->Commit();
			}
			catch (...)
			{
				return -1;
			}
		}
		{
			for (size_t i=0; i<ids.size(); ++i)
			{
				IBPP::Transaction trExperiments = IBPP::TransactionFactory(*dataBase_);
				trExperiments->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trExperiments);
					st->Prepare("select EXP_NAME, EXP_BEGIN, EXP_END, EXP_STEP from EXPERIMENT_PARAM where EXPERIMENT_PARAM.ID_EXPERIMENT = ?");
					st->Set(1, ids[i]);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, name);
						st->Get(2, b);
						st->Get(3, e);
						st->Get(4, s);
						Experiment_Param newEx;
						newEx.name = name;
						newEx.pBegin = b;
						newEx.pEnd = e;
						newEx.pStep = s;
						exps[i].cycles.push_back(newEx);
					}
					trExperiments->Commit();
				}
				catch (...)
				{
					return -1;
				}
			}
		}
	}

	/*
	std::vector<Antenna> antennas;
	for (size_t i = 0; i < ids.size(); ++i)
	{
		GetAntennas(antennas, ids[i]);
	}
	*/

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
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trAntennas);
				st->Prepare("select type, path_out, path_pre, id from ANTENNA where ID_EXPERIMENT = ? order by id");
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
				}
				trAntennas->Commit();
			}
			catch (...)
			{
				return -1;
			}
		}
		{
			for (size_t i = 0; i < antennas.size(); ++i)
			{
				//INPUT PARAMETERS
				int input_id;
				IBPP::Transaction trInputAntennas = IBPP::TransactionFactory(*dataBase_);
				trInputAntennas->Start();
				try
				{
					double scalex, scaley, radius;
					int fractal_n, fractal_m;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInputAntennas);
					st->Prepare("select scalex, scaley, radius, fractal_n, fractal_m, id from INPUT_PARAMS where ID_ANTENNA = ?");
					st->Set(1, antennasID[i]);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, scalex);
						st->Get(2, scaley);
						st->Get(3, radius);
						st->Get(4, fractal_n);
						st->Get(5, fractal_m);
						st->Get(6, input_id);
						antennas[i].inputPar.Radiator.Radius_StripWidth_FeedLineWidth = radius;
						antennas[i].inputPar.Radiator.ScaleX = scalex;
						antennas[i].inputPar.Radiator.ScaleY = scaley;
						antennas[i].inputPar.Radiator.fr_m = fractal_m;
						antennas[i].inputPar.Radiator.fr_N = fractal_n;
					}
					trInputAntennas->Commit();
				}
				catch (...)
				{
					return -1;
				}

				IBPP::Transaction trPreFr = IBPP::TransactionFactory(*dataBase_);
				trPreFr->Start();
				try
				{
					double p_x, p_y;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trPreFr);
					st->Prepare("select p_x, p_y from INPUT_PREFRACTAL_FIRST_POINTS where ID_INPUT_PARAM = ?");
					st->Set(1, input_id);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, p_x);
						st->Get(2, p_y);
						antennas[i].inputPar.Radiator.fr_pred1X.push_back(p_x);
						antennas[i].inputPar.Radiator.fr_pred1Y.push_back(p_y);
					}
					trPreFr->Commit();
				}
				catch (...)
				{
					return -1;
				}

				IBPP::Transaction trPreFrM = IBPP::TransactionFactory(*dataBase_);
				trPreFrM->Start();
				try
				{
					double p_x, p_y;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trPreFrM);
					st->Prepare("select p_x, p_y from INPUT_PREFRACTAL_M_POINTS where ID_INPUT_PARAM = ?");
					st->Set(1, input_id);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, p_x);
						st->Get(2, p_y);
						antennas[i].inputPar.Radiator.fr_predmX.push_back(p_x);
						antennas[i].inputPar.Radiator.fr_predmY.push_back(p_y);
					}
					trPreFrM->Commit();
				}
				catch (...)
				{
					return -1;
				}

				IBPP::Transaction trFrP = IBPP::TransactionFactory(*dataBase_);
				trFrP->Start();
				try
				{
					double p_t, p_x, p_y;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trFrP);
					st->Prepare("select p_t, p_x, p_y from INPUT_FRACTAL_POINTS where ID_INPUT_PARAM = ?");
					st->Set(1, input_id);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, p_t);
						st->Get(2, p_x);
						st->Get(3, p_y);
						antennas[i].inputPar.Radiator.fr_pT.push_back(p_t);
						antennas[i].inputPar.Radiator.fr_pX.push_back(p_x);
						antennas[i].inputPar.Radiator.fr_pY.push_back(p_y);
					}
					trFrP->Commit();
				}
				catch (...)
				{
					return -1;
				}

				IBPP::Transaction trAff = IBPP::TransactionFactory(*dataBase_);
				trAff->Start();
				try
				{
					double d11, d12, d21, d22, lambda, alfa;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trAff);
					st->Prepare("select d11, d12, d21, d22, lambda, alfa from INPUT_FRACTAL_AFFINE_TRANS where ID_INPUT_PARAM = ?");
					st->Set(1, input_id);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, d11);
						st->Get(2, d12);
						st->Get(3, d21);
						st->Get(1, d22);
						st->Get(2, lambda);
						st->Get(3, alfa);
						antennas[i].inputPar.Radiator.fr_D11.push_back(d11);
						antennas[i].inputPar.Radiator.fr_D12.push_back(d12);
						antennas[i].inputPar.Radiator.fr_D21.push_back(d21);
						antennas[i].inputPar.Radiator.fr_D22.push_back(d22);
						antennas[i].inputPar.Radiator.fr_lam.push_back(lambda);
						antennas[i].inputPar.Radiator.fr_al.push_back(alfa);
					}
					trAff->Commit();
				}
				catch (...)
				{
					return -1;
				}

				//OUTPUT PARAMETERS
				int output_id;
				IBPP::Transaction trOutputAntennas = IBPP::TransactionFactory(*dataBase_);
				trOutputAntennas->Start();
				try
				{
					double fst_s11,   fst_w,   fst_bandwidth;
					double scnd_s11,  scnd_w,  scnd_bandwidth;
					double third_s11, third_w, third_bandwidth;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trOutputAntennas);
					st->Prepare("select ID, FST_S11, FST_W, FST_BANDWIDTH, SCND_S11, SCND_W, SCND_BANDWIDTH, THIRD_S11, THIRD_W, THIRD_BANDWIDTH from OUTPUT_PARAMS where ID_ANTENNA = ?");
					st->Set(1, antennasID[i]);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, output_id);
						st->Get(2, fst_s11);
						st->Get(3, fst_w);
						st->Get(4, fst_bandwidth);
						st->Get(5, scnd_s11);
						st->Get(6, scnd_w);
						st->Get(7, scnd_bandwidth);
						st->Get(8, third_s11);
						st->Get(9, third_w);
						st->Get(10, third_bandwidth);
						
						antennas[i].outputPar.fst_s11 = fst_s11;
						antennas[i].outputPar.fst_w = fst_w;
						antennas[i].outputPar.fst_bandwidth = fst_bandwidth;
						antennas[i].outputPar.scnd_s11 = scnd_s11;
						antennas[i].outputPar.scnd_w = scnd_w;
						antennas[i].outputPar.scnd_bandwidth = scnd_bandwidth;
						antennas[i].outputPar.third_s11 = third_s11;
						antennas[i].outputPar.third_w = third_w;
						antennas[i].outputPar.third_bandwidth = third_bandwidth;
						
						antennas[i].outputPar.findDATA_FOR_MEMORY_USAGE = false;
						antennas[i].outputPar.findDATA_FOR_DIELECTRIC_MEDIA = false;
						antennas[i].outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS = false;
						antennas[i].outputPar.findEXCITATION_BY_VOLTAGE_SOURCE = false;
						//antennas[i].outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX = false;
						antennas[i].outputPar.findDATA_OF_THE_VOLTAGE_SOURCE = false;
						antennas[i].outputPar.findSCATTERING_PARAMETERS = false;
						antennas[i].outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS = false;
						antennas[i].outputPar.findSUMMARY_OF_LOSSES = false;
						antennas[i].outputPar.findDIRECTIVITY_PATTERN_THETA_PHI = false;
						antennas[i].outputPar.findDIRECTIVITY_PATTERN_PARAMS = false;
					}
					trOutputAntennas->Commit();
				}
				catch (...)
				{
					return -1;
				}

				//DATA_FOR_MEMORY_USAGE
				IBPP::Transaction trMemoryUsage = IBPP::TransactionFactory(*dataBase_);
				trMemoryUsage->Start();
				try
				{
					double LengthAllSegms_M, SurfAllTri_MM;
					int NumMetallicTri, NumDielectrTri, NumApertureTri, NumGoTri, NumWindscreenTri, NumFemSurfaceTri, NumModalPortTri, NumMetallicSegms, NumDieMagnCubs, NumTetrahedra, NumEdgesPORegion, NumWedgesPORegion, NumFockRegions, NumPolySurfaces, NumUTDCylindres, NumMetallicEdgesMoM, NumMetallicEdgesPO, NumDielectricEdgesMoM, NumDielectricEdgesPO, NumApertureEdgesMoM, NumEdgesFEMMomSurf, NumNodesBetweenSegms, NumConnectionPoints, NumDielectricCuboids, NumMagneticCuboids, NumBasisFunctMoM, NumBasisFunctPO;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trMemoryUsage);
					st->Prepare("select LENGTHALLSEGMS_M, SURFALLTRI_MM, NUMMETALLICTRI, NUMDIELECTRTRI, NUMAPERTURETRI, NUMGOTRI, NUMWINDSCREENTRI, NUMFEMSURFACETRI, NUMMODALPORTTRI, NUMMETALLICSEGMS, NUMDIEMAGNCUBS, NUMTETRAHEDRA, NUMEDGESPOREGION, NUMWEDGESPOREGION, NUMFOCKREGIONS, NUMPOLYSURFACES, NUMUTDCYLINDRES, NUMMETALLICEDGESMOM, NUMMETALLICEDGESPO, NUMDIELECTRICEDGESMOM, NUMDIELECTRICEDGESPO, NUMAPERTUREEDGESMOM, NUMEDGESFEMMOMSURF, NUMNODESBETWEENSEGMS, NUMCONNECTIONPOINTS, NUMDIELECTRICCUBOIDS, NUMMAGNETICCUBOIDS, NUMBASISFUNCTMOM, NUMBASISFUNCTPO from DATA_FOR_MEMORY_USAGE where ID_OUTPUT_PARAM = ?");
					st->Set(1, output_id);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1,  LengthAllSegms_M);
						st->Get(2,  SurfAllTri_MM);
						st->Get(3,  NumMetallicTri);
						st->Get(4,  NumDielectrTri);
						st->Get(5,  NumApertureTri);
						st->Get(6,  NumGoTri);
						st->Get(7,  NumWindscreenTri);
						st->Get(8,  NumFemSurfaceTri);
						st->Get(9,  NumModalPortTri);
						st->Get(10, NumMetallicSegms);
						st->Get(11, NumDieMagnCubs);
						st->Get(12, NumTetrahedra);
						st->Get(13, NumEdgesPORegion);
						st->Get(14, NumWedgesPORegion);
						st->Get(15, NumFockRegions);
						st->Get(16, NumPolySurfaces);
						st->Get(17, NumUTDCylindres);
						st->Get(18, NumMetallicEdgesMoM);
						st->Get(19, NumMetallicEdgesPO);
						st->Get(20, NumDielectricEdgesMoM);
						st->Get(21, NumDielectricEdgesPO);
						st->Get(22, NumApertureEdgesMoM);
						st->Get(23, NumEdgesFEMMomSurf);
						st->Get(24, NumNodesBetweenSegms);
						st->Get(25, NumConnectionPoints);
						st->Get(26, NumDielectricCuboids);
						st->Get(27, NumMagneticCuboids);
						st->Get(28, NumBasisFunctMoM);
						st->Get(29, NumBasisFunctPO);
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.LengthAllSegms_M = LengthAllSegms_M;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.SurfAllTri_MM = SurfAllTri_MM;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicTri = NumMetallicTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumDielectrTri = NumDielectrTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumApertureTri = NumApertureTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumGoTri = NumGoTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumWindscreenTri = NumWindscreenTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumFemSurfaceTri = NumFemSurfaceTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumModalPortTri = NumModalPortTri;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicSegms = NumMetallicSegms;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumDieMagnCubs = NumDieMagnCubs;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumTetrahedra = NumTetrahedra;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesPORegion = NumEdgesPORegion;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumWedgesPORegion = NumWedgesPORegion;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumFockRegions = NumFockRegions;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumPolySurfaces = NumPolySurfaces;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumUTDCylindres = NumUTDCylindres;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesMoM = NumMetallicEdgesMoM;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesPO = NumMetallicEdgesPO;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesMoM = NumDielectricEdgesMoM;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesPO = NumDielectricEdgesPO;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumApertureEdgesMoM = NumApertureEdgesMoM;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesFEMMomSurf = NumEdgesFEMMomSurf;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumNodesBetweenSegms = NumNodesBetweenSegms;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumConnectionPoints = NumConnectionPoints;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricCuboids = NumDielectricCuboids;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumMagneticCuboids = NumMagneticCuboids;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctMoM = NumBasisFunctMoM;
						antennas[i].outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctPO = NumBasisFunctPO;
						antennas[i].outputPar.findDATA_FOR_MEMORY_USAGE = true;
					}
					trMemoryUsage->Commit();
				}
				catch (...)
				{
					return -1;
				}

				std::vector<int> output1freq_id;
				IBPP::Transaction trOutput1FreqAntennas = IBPP::TransactionFactory(*dataBase_);
				trOutput1FreqAntennas->Start();
				try
				{
					int out1fr;
					double Frequency;
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trOutput1FreqAntennas);
					st->Prepare("select ID, Frequency from OUTPUT_PARAMS_FOR_ONE_FREQ where ID_OUTPUT_PARAM = ?");
					st->Set(1, output_id);
					st->Execute();
					while (st->Fetch())
					{
						st->Get(1, out1fr);
						st->Get(2, Frequency);
						output1freq_id.push_back(out1fr);
						DATA_FOR_ONE_FREQ df1f;
						df1f._VEC_EXCITATION_BY_VOLTAGE_SOURCE.resize(1);
						antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ.push_back(df1f);
						antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ.back()._VEC_EXCITATION_BY_VOLTAGE_SOURCE[0].Frequency = Frequency;
					}
					trOutput1FreqAntennas->Commit();
				}
				catch (...)
				{
					return -1;
				}

				for (size_t j = 0; j < output1freq_id.size(); ++j)
				{
					IBPP::Transaction trScat = IBPP::TransactionFactory(*dataBase_);
					trScat->Start();
					try
					{
						int SPortSinc, SPortSource;
						double SRealPart, SImagPart, SMagnitudeLinear, SMagnitudeDB, SPhase, SumS2MagnitudeLinear, SumS2MagnitudeDB, S11, VSWR, EMAX;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trScat);
						st->Prepare("select SPORTSINC, SPORTSOURCE, SREALPART, SIMAGPART, SMAGNITUDELINEAR, SMAGNITUDEDB, SPHASE, SUMS2MAGNITUDELINEAR, SUMS2MAGNITUDEDB, S11, VSWR, EMAX from SCATTERING_PARAMETERS where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, SPortSinc);
							st->Get(2, SPortSource);
							st->Get(3, SRealPart);
							st->Get(4, SImagPart);
							st->Get(5, SMagnitudeLinear);
							st->Get(6, SMagnitudeDB);
							st->Get(7, SPhase);
							st->Get(8, SumS2MagnitudeLinear);
							st->Get(9, SumS2MagnitudeDB);
							st->Get(10, S11);
							st->Get(11, VSWR);
							st->Get(12, EMAX);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SPortSinc = SPortSinc;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SPortSource = SPortSource;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SRealPart = SRealPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SImagPart = SImagPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SMagnitudeLinear = SMagnitudeLinear;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SMagnitudeDB = SMagnitudeDB;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SPhase = SPhase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SumS2MagnitudeLinear = SumS2MagnitudeLinear;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.SumS2MagnitudeDB = SumS2MagnitudeDB;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.S11 = S11;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.VSWR = VSWR;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SCATTERING_PARAMETERS.EMAX = EMAX;
							antennas[i].outputPar.findSCATTERING_PARAMETERS = true;
						}
						trScat->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trDirPP = IBPP::TransactionFactory(*dataBase_);
					trDirPP->Start();
					try
					{
						double GainPower, GainPowerLoss, DTheta, DPhi, PolarHorizWatt, PolarHorizPers, PolarVertWatt, PolarVertPers, PolarSWatt, PolarSPers, PolarZWatt, PolarZPers, PolarLeftHandWatt, PolarLeftHandPers, PolarRightHandWatt, PolarRightHandPers;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trDirPP);
						st->Prepare("select GAINPOWER, GAINPOWERLOSS, DTHETA, DPHI, POLARHORIZWATT, POLARHORIZPERS, POLARVERTWATT, POLARVERTPERS, POLARSWATT, POLARSPERS, POLARZWATT, POLARZPERS, POLARLEFTHANDWATT, POLARLEFTHANDPERS, POLARRIGHTHANDWATT, POLARRIGHTHANDPERS from DIRECTIVITY_PATTERN_PARAMS where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, GainPower);
							st->Get(2, GainPowerLoss);
							st->Get(3, DTheta);
							st->Get(4, DPhi);
							st->Get(5, PolarHorizWatt);
							st->Get(6, PolarHorizPers);
							st->Get(7, PolarVertWatt);
							st->Get(8, PolarVertPers);
							st->Get(9, PolarSWatt);
							st->Get(10, PolarSPers);
							st->Get(11, PolarZWatt);
							st->Get(12, PolarZPers);
							st->Get(13, PolarLeftHandWatt);
							st->Get(14, PolarLeftHandPers);
							st->Get(15, PolarRightHandWatt);
							st->Get(16, PolarRightHandPers);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.GainPower = GainPower;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss = GainPowerLoss;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.DTheta = DTheta;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.DPhi = DPhi;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarHorizWatt = PolarHorizWatt;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarHorizPers = PolarHorizPers;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarVertWatt =	PolarVertWatt;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarVertPers =	PolarVertPers;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarSWatt = PolarSWatt;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarSPers = PolarSPers;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarZWatt = PolarZWatt;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarZPers = PolarZPers;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandWatt = PolarLeftHandWatt;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandPers = PolarLeftHandPers;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandWatt = PolarRightHandWatt;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandPers = PolarRightHandPers;
							antennas[i].outputPar.findDIRECTIVITY_PATTERN_PARAMS = true;
						}
						trDirPP->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trSumLos = IBPP::TransactionFactory(*dataBase_);
					trSumLos->Start();
					try
					{
						double MetallicElements, MismatchFeed, NonRadiatingNetworks, BackPowerPassWaveguidePorts, BackPowerPassModalPorts, SumAllLosses, EfficiencyTheAntenna, BasedTotalActivePower;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trSumLos);
						st->Prepare("select METALLICELEMENTS, MISMATCHFEED, NONRADIATINGNETWORKS, BACKPOWERPASSWAVEGUIDEPORTS, BACKPOWERPASSMODALPORTS, SUMALLLOSSES, EFFICIENCYTHEANTENNA, BASEDTOTALACTIVEPOWER from SUMMARY_OF_LOSSES where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, MetallicElements);
							st->Get(2, MismatchFeed);
							st->Get(3, NonRadiatingNetworks);
							st->Get(4, BackPowerPassWaveguidePorts);
							st->Get(5, BackPowerPassModalPorts);
							st->Get(6, SumAllLosses);
							st->Get(7, EfficiencyTheAntenna);
							st->Get(8, BasedTotalActivePower);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.MetallicElements = MetallicElements;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.MismatchFeed = MismatchFeed;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.NonRadiatingNetworks = NonRadiatingNetworks;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.BackPowerPassWaveguidePorts = BackPowerPassWaveguidePorts;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.BackPowerPassModalPorts = BackPowerPassModalPorts;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.SumAllLosses = SumAllLosses;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.EfficiencyTheAntenna = EfficiencyTheAntenna;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._SUMMARY_OF_LOSSES.BasedTotalActivePower = BasedTotalActivePower;
							antennas[i].outputPar.findSUMMARY_OF_LOSSES = true;
						}
						trSumLos->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trLosDEl = IBPP::TransactionFactory(*dataBase_);
					trLosDEl->Start();
					try
					{
						double PowerLoss, MaximumSARValue, AveragedSARValue;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trLosDEl);
						st->Prepare("select POWERLOSS, MAXIMUMSARVALUE, AVERAGEDSARVALUE from LOSSES_IN_DIELECTRIC_VOLUME_ELE where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, PowerLoss);
							st->Get(2, MaximumSARValue);
							st->Get(3, AveragedSARValue);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.PowerLoss = PowerLoss;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.MaximumSARValue = MaximumSARValue;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.AveragedSARValue = AveragedSARValue;
							antennas[i].outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS = true;
						}
						trLosDEl->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trDieM = IBPP::TransactionFactory(*dataBase_);
					trDieM->Start();
					try
					{
						int    InternalIndex;
						double RelPermittivity, RelPermeability, Conductivity, TanDeltaElectric, TanDeltaMagnetic, MassDensity;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trDieM);
						st->Prepare("select INTERNALINDEX, RELPERMITTIVITY, RELPERMEABILITY, CONDUCTIVITY, TANDELTAELECTRIC, TANDELTAMAGNETIC, MASSDENSITY from DATA_FOR_DIELECTRIC_MEDIA where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, InternalIndex);
							st->Get(2, RelPermittivity);
							st->Get(3, RelPermeability);
							st->Get(4, Conductivity);
							st->Get(5, TanDeltaElectric);
							st->Get(6, TanDeltaMagnetic);
							st->Get(7, MassDensity);
							DATA_FOR_DIELECTRIC_MEDIA dgdm;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.push_back(dgdm);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().InternalIndex = InternalIndex;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().RelPermittivity = RelPermittivity;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().RelPermeability = RelPermeability;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().Conductivity = Conductivity;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().TanDeltaElectric = TanDeltaElectric;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().TanDeltaMagnetic = TanDeltaMagnetic;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_FOR_DIELECTRIC_MEDIA.back().MassDensity = MassDensity;
							antennas[i].outputPar.findDATA_FOR_DIELECTRIC_MEDIA = true;
						}
						trDieM->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trExByVS = IBPP::TransactionFactory(*dataBase_);
					trExByVS->Start();
					try
					{
						int    ExcitationIndex, SourceSegmLabel, AbsolNumSegms;
						double Frequency, Wavelength, OpenCircuitVoltage, Phase, ElectricalEdgeLength, LocationExcitX, LocationExcitY, LocationExcitZ, PositiveFeedDirX, PositiveFeedDirY, PositiveFeedDirZ;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trExByVS);
						st->Prepare("select EXCITATIONINDEX, FREQUENCY, WAVELENGTH, OPENCIRCUITVOLTAGE, PHASE, ELECTRICALEDGELENGTH, SOURCESEGMLABEL, ABSOLNUMSEGMS, LOCATIONEXCITX, LOCATIONEXCITY, LOCATIONEXCITZ, POSITIVEFEEDDIRX, POSITIVEFEEDDIRY, POSITIVEFEEDDIRZ from EXCITATION_BY_VOLTAGE_SOURCE where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, ExcitationIndex);
							st->Get(2, Frequency);
							st->Get(3, Wavelength);
							st->Get(4, OpenCircuitVoltage);
							st->Get(5, Phase);
							st->Get(6, ElectricalEdgeLength);
							st->Get(7, SourceSegmLabel);
							st->Get(8, AbsolNumSegms);
							st->Get(9, LocationExcitX);
							st->Get(10, LocationExcitY);
							st->Get(11, LocationExcitZ);
							st->Get(12, PositiveFeedDirX);
							st->Get(13, PositiveFeedDirY);
							st->Get(14, PositiveFeedDirZ);
							EXCITATION_BY_VOLTAGE_SOURCE ebvs;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.push_back(ebvs);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().ExcitationIndex = ExcitationIndex;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().Frequency = Frequency;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().Wavelength = Wavelength;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().OpenCircuitVoltage = OpenCircuitVoltage;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().Phase = Phase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().ElectricalEdgeLength = ElectricalEdgeLength;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().SourceSegmLabel = SourceSegmLabel;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().AbsolNumSegms = AbsolNumSegms;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().LocationExcitX = LocationExcitX;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().LocationExcitY = LocationExcitY;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().LocationExcitZ = LocationExcitZ;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().PositiveFeedDirX = PositiveFeedDirX;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().PositiveFeedDirY = PositiveFeedDirY;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.back().PositiveFeedDirZ = PositiveFeedDirZ;
							antennas[i].outputPar.findEXCITATION_BY_VOLTAGE_SOURCE = true;
						}
						trExByVS->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trDOTVS = IBPP::TransactionFactory(*dataBase_);
					trDOTVS->Start();
					try
					{
						double CurrentRealPart, CurrentImagPart, CurrentMagnitude, CurrentPhase, AdmittRealPart, AdmittImagPart, AdmittMagnitude, AdmittPhase, ImpedanceRealPart, ImpedanceImagPart, ImpedanceMagnitude, ImpedancePhase, Inductance, Capacitance, Power;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trDOTVS);
						st->Prepare("select CURRENTREALPART, CURRENTIMAGPART, CURRENTMAGNITUDE, CURRENTPHASE, ADMITTREALPART, ADMITTIMAGPART, ADMITTMAGNITUDE, ADMITTPHASE, IMPEDANCEREALPART, IMPEDANCEIMAGPART, IMPEDANCEMAGNITUDE, IMPEDANCEPHASE, INDUCTANCE, CAPACITANCE, POWER from DATA_OF_THE_VOLTAGE_SOURCE where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, CurrentRealPart);
							st->Get(2, CurrentImagPart);
							st->Get(3, CurrentMagnitude);
							st->Get(4, CurrentPhase);
							st->Get(5, AdmittRealPart);
							st->Get(6, AdmittImagPart);
							st->Get(7, AdmittMagnitude);
							st->Get(8, AdmittPhase);
							st->Get(9, ImpedanceRealPart);
							st->Get(10, ImpedanceImagPart);
							st->Get(11, ImpedanceMagnitude);
							st->Get(12, ImpedancePhase);
							st->Get(13, Inductance);
							st->Get(14, Capacitance);
							st->Get(15, Power);
							DATA_OF_THE_VOLTAGE_SOURCE dotvs;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.push_back(dotvs);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().CurrentRealPart = CurrentRealPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().CurrentImagPart = CurrentImagPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().CurrentMagnitude = CurrentMagnitude;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().CurrentPhase = CurrentPhase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().AdmittRealPart = AdmittRealPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().AdmittImagPart = AdmittImagPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().AdmittMagnitude = AdmittMagnitude;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().AdmittPhase = AdmittPhase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().ImpedanceRealPart = ImpedanceRealPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().ImpedanceImagPart = ImpedanceImagPart;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().ImpedanceMagnitude = ImpedanceMagnitude;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().ImpedancePhase = ImpedancePhase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().Inductance = Inductance;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().Capacitance = Capacitance;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.back().Power = Power;
							antennas[i].outputPar.findDATA_OF_THE_VOLTAGE_SOURCE = true;
						}
						trDOTVS->Commit();
					}
					catch (...)
					{
						return -1;
					}

					IBPP::Transaction trDPTP = IBPP::TransactionFactory(*dataBase_);
					trDPTP->Start();
					try
					{
						double Tetta, Phi, EthetaMagn, EthetaPhase, EphiMagn, EphiPhase, DirectivityVert, DirectivityHoriz, DirectivityTotal, PolarizationAxial, PolarizationAngle, Gain;
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trDPTP);
						st->Prepare("select TETTA, PHI, ETHETAMAGN, ETHETAPHASE, EPHIMAGN, EPHIPHASE, DIRECTIVITYVERT, DIRECTIVITYHORIZ, DIRECTIVITYTOTAL, POLARIZATIONAXIAL, POLARIZATIONANGLE, GAIN from DIRECTIVITY_PATTERN_THETA_PHI where ID_OUTPUT = ?");
						st->Set(1, output1freq_id[j]);
						st->Execute();
						while (st->Fetch())
						{
							st->Get(1, Tetta);
							st->Get(2, Phi);
							st->Get(3, EthetaMagn);
							st->Get(4, EthetaPhase);
							st->Get(5, EphiMagn);
							st->Get(6, EphiPhase);
							st->Get(7, DirectivityVert);
							st->Get(8, DirectivityHoriz);
							st->Get(9, DirectivityTotal);
							st->Get(10, PolarizationAxial);
							st->Get(11, PolarizationAngle);
							st->Get(12, Gain);
							DIRECTIVITY_PATTERN_THETA_PHI dptp;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.push_back(dptp);
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().Tetta = Tetta;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().Phi = Phi;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().EthetaMagn = EthetaMagn;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().EthetaPhase = EthetaPhase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().EphiMagn = EphiMagn;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().EphiPhase = EphiPhase;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().DirectivityVert = DirectivityVert;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().DirectivityHoriz = DirectivityHoriz;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().DirectivityTotal = DirectivityTotal;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().PolarizationAxial = PolarizationAxial;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().PolarizationAngle = PolarizationAngle;
							antennas[i].outputPar._VEC_DATA_FOR_ONE_FREQ[j]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.back().Gain = Gain;
							antennas[i].outputPar.findDIRECTIVITY_PATTERN_THETA_PHI = true;
						}
						trDPTP->Commit();
					}
					catch (...)
					{
						return -1;
					}
				}
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
				IBPP::Date dt;
				dt.SetDate(pExperiment->date.tm_year, pExperiment->date.tm_mon, pExperiment->date.tm_mday);
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trExperiment);
				st->Prepare("insert into EXPERIMENT (WRITEN_DATE, COMMENT) values (?, ?) returning id");
				st->Set(1, dt);
				st->Set(2, pExperiment->comment);
				st->Execute();
				st->Get(1, idExperiment);
				trExperiment->Commit();
			}
			catch (...)
			{
				trExperiment->Rollback();
			}
		}

		{
			if (idExperiment > 0)
			for (size_t i=0; i<pExperiment->cycles.size(); ++i)
			{
				IBPP::Transaction trExperimentP = IBPP::TransactionFactory(*dataBase_);
				trExperimentP->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trExperimentP);
					st->Prepare("insert into experiment_param (ID_EXPERIMENT, exp_name, exp_begin, exp_end, exp_step) values (?, ?, ?, ?, ?)");
					st->Set(1, idExperiment);
					st->Set(2, pExperiment->cycles[i].name);
					st->Set(3, pExperiment->cycles[i].pBegin);
					st->Set(4, pExperiment->cycles[i].pEnd);
					st->Set(5, pExperiment->cycles[i].pStep);
					st->Execute();
					trExperimentP->Commit();
				}
				catch (...)
				{
					trExperimentP->Rollback();
				}
			}
		}
	}
	return idExperiment;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @details Запись антенны в базу данных
int FrbrdDatabase::WriteAntennaData(Antenna &_antenna, int idExperiment)
{
	//clock_t tStart;

	int idAntenna          = -1;
	int idOutputPar        = -1; 
	int idInputPar         = -1;
	int idOutputParOneFreq = -1;
	if ((*dataBase_)->Connected())
	{
		{
			IBPP::Transaction trAntenna = IBPP::TransactionFactory(*dataBase_);
			trAntenna->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trAntenna);
				st->Prepare("insert into antenna (ID_EXPERIMENT, type, path_out, path_pre) values (?, ?, ?, ?) returning id");
				st->Set(1, idExperiment);
				st->Set(2, _antenna.type);
				st->Set(3, _antenna.pathOut);
				st->Set(4, _antenna.pathPre);
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

		//tStart = clock();
		{
			IBPP::Transaction trInput = IBPP::TransactionFactory(*dataBase_);
			trInput->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInput);
				st->Prepare("insert into INPUT_PARAMS (ID_ANTENNA, SCALEX, SCALEY, RADIUS, FRACTAL_N, FRACTAL_M) values (?, ?, ?, ?, ?, ?) returning id");
				st->Set(1, idAntenna);
				st->Set(2, _antenna.inputPar.Radiator.ScaleX);
				st->Set(3, _antenna.inputPar.Radiator.ScaleY);
				st->Set(4, _antenna.inputPar.Radiator.Radius_StripWidth_FeedLineWidth);
				st->Set(5, _antenna.inputPar.Radiator.fr_N);
				st->Set(6, _antenna.inputPar.Radiator.fr_m);
				st->Execute();
				st->Get(1, idInputPar);
				trInput->Commit();
			}
			catch (...)
			{
				trInput->Rollback();
				return -1;
			}
		}

		{
			for (size_t i=0; i<_antenna.inputPar.Radiator.fr_D11.size(); ++i)
			{
				IBPP::Transaction trInputFractalAff = IBPP::TransactionFactory(*dataBase_);
				trInputFractalAff->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInputFractalAff);
					st->Prepare("insert into INPUT_FRACTAL_AFFINE_TRANS (ID_INPUT_PARAM, D11, D12, D21, D22, LAMBDA, ALFA) values (?, ?, ?, ?, ?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.Radiator.fr_D11[i]);
					st->Set(3, _antenna.inputPar.Radiator.fr_D12[i]);
					st->Set(4, _antenna.inputPar.Radiator.fr_D21[i]);
					st->Set(5, _antenna.inputPar.Radiator.fr_D22[i]);
					st->Set(6, _antenna.inputPar.Radiator.fr_lam[i]);
					st->Set(7, _antenna.inputPar.Radiator.fr_al[i]);
					st->Execute();
					trInputFractalAff->Commit();
				}
				catch (...)
				{
					trInputFractalAff->Rollback();
				}
			}
		}

		{
			for (size_t i=0; i<_antenna.inputPar.Radiator.fr_pT.size(); ++i)
			{
				IBPP::Transaction trInputFractalPoints = IBPP::TransactionFactory(*dataBase_);
				trInputFractalPoints->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInputFractalPoints);
					st->Prepare("insert into INPUT_FRACTAL_POINTS (ID_INPUT_PARAM, P_T, P_X, P_Y) values (?, ?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.Radiator.fr_pT[i]);
					st->Set(3, _antenna.inputPar.Radiator.fr_pX[i]);
					st->Set(4, _antenna.inputPar.Radiator.fr_pY[i]);
					st->Execute();
					trInputFractalPoints->Commit();
				}
				catch (...)
				{
					trInputFractalPoints->Rollback();
				}
			}
		}

		{
			for (size_t i=0; i<_antenna.inputPar.Radiator.fr_pred1X.size(); ++i)
			{
				IBPP::Transaction trInput1PreFractalPoints = IBPP::TransactionFactory(*dataBase_);
				trInput1PreFractalPoints->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInput1PreFractalPoints);
					st->Prepare("insert into INPUT_PREFRACTAL_FIRST_POINTS (ID_INPUT_PARAM, P_X, P_Y) values (?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.Radiator.fr_pred1X[i]);
					st->Set(3, _antenna.inputPar.Radiator.fr_pred1Y[i]);
					st->Execute();
					trInput1PreFractalPoints->Commit();
				}
				catch (...)
				{
					trInput1PreFractalPoints->Rollback();
				}
			}
		}

		{
			for (size_t i=0; i<_antenna.inputPar.Radiator.fr_predmX.size(); ++i)
			{
				IBPP::Transaction trInput1PreFractalPoints = IBPP::TransactionFactory(*dataBase_);
				trInput1PreFractalPoints->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInput1PreFractalPoints);
					st->Prepare("insert into INPUT_PREFRACTAL_M_POINTS (ID_INPUT_PARAM, P_X, P_Y) values (?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.Radiator.fr_predmX[i]);
					st->Set(3, _antenna.inputPar.Radiator.fr_predmY[i]);
					st->Execute();
					trInput1PreFractalPoints->Commit();
				}
				catch (...)
				{
					trInput1PreFractalPoints->Rollback();
				}
			}
		}
		//cout << "input: " << (double)(clock() - tStart) / CLOCKS_PER_SEC << endl;
		//tStart = clock();
		{
			IBPP::Transaction trOutput = IBPP::TransactionFactory(*dataBase_);
			trOutput->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trOutput);
				st->Prepare("insert into OUTPUT_PARAMS (ID_ANTENNA, FST_S11, FST_W, FST_BANDWIDTH, SCND_S11, SCND_W, SCND_BANDWIDTH, THIRD_S11, THIRD_W, THIRD_BANDWIDTH) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) returning id");
				st->Set(1, idAntenna);
				st->Set(2, _antenna.outputPar.fst_s11);
				st->Set(3, _antenna.outputPar.fst_w);
				st->Set(4, _antenna.outputPar.fst_bandwidth);
				st->Set(5, _antenna.outputPar.scnd_s11);
				st->Set(6, _antenna.outputPar.scnd_w);
				st->Set(7, _antenna.outputPar.scnd_bandwidth);
				st->Set(8, _antenna.outputPar.third_s11);
				st->Set(9, _antenna.outputPar.third_w);
				st->Set(10, _antenna.outputPar.third_bandwidth);
				st->Execute();
				st->Get(1, idOutputPar);
				trOutput->Commit();
			}
			catch (...)
			{
				trOutput->Rollback();
				return -1;
			}
		}

		if (_antenna.outputPar.findDATA_FOR_MEMORY_USAGE)
		{
			IBPP::Transaction trDataMemoryUsage = IBPP::TransactionFactory(*dataBase_);
			trDataMemoryUsage->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trDataMemoryUsage);
				st->Prepare("insert into DATA_FOR_MEMORY_USAGE (ID_OUTPUT_PARAM, LENGTHALLSEGMS_M, SURFALLTRI_MM, NUMMETALLICTRI, NUMDIELECTRTRI, NUMAPERTURETRI, NUMGOTRI, NUMWINDSCREENTRI, NUMFEMSURFACETRI, NUMMODALPORTTRI, NUMMETALLICSEGMS, NUMDIEMAGNCUBS, NUMTETRAHEDRA, NUMEDGESPOREGION, NUMWEDGESPOREGION, NUMFOCKREGIONS, NUMPOLYSURFACES, NUMUTDCYLINDRES, NUMMETALLICEDGESMOM, NUMMETALLICEDGESPO, NUMDIELECTRICEDGESMOM, NUMDIELECTRICEDGESPO, NUMAPERTUREEDGESMOM, NUMEDGESFEMMOMSURF, NUMNODESBETWEENSEGMS, NUMCONNECTIONPOINTS, NUMDIELECTRICCUBOIDS, NUMMAGNETICCUBOIDS, NUMBASISFUNCTMOM, NUMBASISFUNCTPO) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
				st->Set(1, idOutputPar);
				st->Set(2, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.LengthAllSegms_M);
				st->Set(3, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.SurfAllTri_MM);
				st->Set(4, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicTri);
				st->Set(5, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectrTri);
				st->Set(6, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureTri);
				st->Set(7, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumGoTri);
				st->Set(8, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWindscreenTri);
				st->Set(9, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFemSurfaceTri);
				st->Set(10, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumModalPortTri);
				st->Set(11, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicSegms);
				st->Set(12, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDieMagnCubs);
				st->Set(13, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumTetrahedra);
				st->Set(14, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesPORegion);
				st->Set(15, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWedgesPORegion);
				st->Set(16, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFockRegions);
				st->Set(17, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumPolySurfaces);
				st->Set(18, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumUTDCylindres);
				st->Set(19, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesMoM);
				st->Set(20, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesPO);
				st->Set(21, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesMoM);
				st->Set(22, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesPO);
				st->Set(23, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureEdgesMoM);
				st->Set(24, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesFEMMomSurf);
				st->Set(25, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumNodesBetweenSegms);
				st->Set(26, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumConnectionPoints);
				st->Set(27, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricCuboids);
				st->Set(28, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMagneticCuboids);
				st->Set(29, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctMoM);
				st->Set(30, _antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctPO);
				st->Execute();
				trDataMemoryUsage->Commit();
			}
			catch (...)
			{
				trDataMemoryUsage->Rollback();
			}
		}

		for (size_t i=0; i<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++i)
		{
			if (_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE)
			{
				IBPP::Transaction trOutput = IBPP::TransactionFactory(*dataBase_);
				trOutput->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trOutput);
					st->Prepare("insert into OUTPUT_PARAMS_FOR_ONE_FREQ (ID_OUTPUT_PARAM, FREQUENCY) values (?, ?) returning id");
					st->Set(1, idOutputPar);
					st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[0].Frequency);
					st->Execute();
					st->Get(1, idOutputParOneFreq);
					trOutput->Commit();
				}
				catch (...)
				{
					trOutput->Rollback();
					return -1;
				}
			}

			if (_antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE)
			{
				for(size_t j = 0; j<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE.size(); ++j)
				{
					IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
					tr->Start();
					try
					{
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
						st->Prepare("insert into DATA_OF_THE_VOLTAGE_SOURCE (ID_OUTPUT, CURRENTREALPART, CURRENTIMAGPART, CURRENTMAGNITUDE, CURRENTPHASE, ADMITTREALPART, ADMITTIMAGPART, ADMITTMAGNITUDE, ADMITTPHASE, IMPEDANCEREALPART, IMPEDANCEIMAGPART, IMPEDANCEMAGNITUDE, IMPEDANCEPHASE, INDUCTANCE, CAPACITANCE, POWER) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
						st->Set(1, idOutputParOneFreq);
						st->Set(2,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].CurrentRealPart);
						st->Set(3,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].CurrentImagPart);
						st->Set(4,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].CurrentMagnitude);
						st->Set(5,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].CurrentPhase);
						st->Set(6,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].AdmittRealPart);
						st->Set(7,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].AdmittImagPart);
						st->Set(8,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].AdmittMagnitude);
						st->Set(9,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].AdmittPhase);
						st->Set(10, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].ImpedanceRealPart);
						st->Set(11, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].ImpedanceImagPart);
						st->Set(12, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].ImpedanceMagnitude);
						st->Set(13, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].ImpedancePhase);
						st->Set(14, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].Inductance);
						st->Set(15, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].Capacitance);
						st->Set(16, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_OF_THE_VOLTAGE_SOURCE[j].Power);
						st->Execute();
						tr->Commit();
					}
					catch (...)
					{
						tr->Rollback();
					}
				}
			}

			if (_antenna.outputPar.findSCATTERING_PARAMETERS)
			{
				IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
				tr->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
					st->Prepare("insert into SCATTERING_PARAMETERS (ID_OUTPUT, SPORTSINC, SPORTSOURCE, SREALPART, SIMAGPART, SMAGNITUDELINEAR, SMAGNITUDEDB, SPHASE, SUMS2MAGNITUDELINEAR, SUMS2MAGNITUDEDB, S11, VSWR, EMAX) values (?,?,?,?,?,?,?,?,?,?,?,?,?)");
					st->Set(1, idOutputParOneFreq);
					st->Set(2,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SPortSinc);
					st->Set(3,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SPortSource);
					st->Set(4,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SRealPart);
					st->Set(5,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SImagPart);
					st->Set(6,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SMagnitudeLinear);
					st->Set(7,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SMagnitudeDB);
					st->Set(8,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SPhase);
					st->Set(9,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SumS2MagnitudeLinear);
					st->Set(10, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SumS2MagnitudeDB);
					st->Set(11, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.S11);
					st->Set(12, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.VSWR);
					st->Set(13, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.EMAX);
					st->Execute();
					tr->Commit();
				}
				catch (...)
				{
					tr->Rollback();
				}
			}

			if (_antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA || _antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS)
			{
				for(size_t j = 0; j<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA.size(); ++j)
				{
					IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
					tr->Start();
					try
					{
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
						st->Prepare("insert into DATA_FOR_DIELECTRIC_MEDIA (ID_OUTPUT, INTERNALINDEX, RELPERMITTIVITY, RELPERMEABILITY, CONDUCTIVITY, TANDELTAELECTRIC, TANDELTAMAGNETIC, MASSDENSITY) values (?,?,?,?,?,?,?,?)");
						st->Set(1, idOutputParOneFreq);
						st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].InternalIndex);
						st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].RelPermittivity);
						st->Set(4, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].RelPermeability);
						st->Set(5, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].Conductivity);
						st->Set(6, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].TanDeltaElectric);
						st->Set(7, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].TanDeltaMagnetic);
						st->Set(8, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DATA_FOR_DIELECTRIC_MEDIA[j].MassDensity);
						st->Execute();
						tr->Commit();
					}
					catch (...)
					{
						tr->Rollback();
					}
				}
			}

			if (_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE)
			{
				if (_antenna.type == STRIPE || _antenna.type == PLANE)
				{
					for(size_t j = 0; j<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.size(); ++j)
					{
						IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
						tr->Start();
						try
						{
							IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
							st->Prepare("insert into EXCITATION_BY_VOLTAGE_SOURCE (ID_OUTPUT, EXCITATIONINDEX, FREQUENCY, WAVELENGTH, OPENCIRCUITVOLTAGE, PHASE, ELECTRICALEDGELENGTH) values (?,?,?,?,?,?,?)");
							st->Set(1, idOutputParOneFreq);
							st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].ExcitationIndex);
							st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].Frequency);
							st->Set(4, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].Wavelength);
							st->Set(5, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].OpenCircuitVoltage);
							st->Set(6, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].Phase);
							st->Set(7, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].ElectricalEdgeLength);
							st->Execute();
							tr->Commit();
						}
						catch (...)
						{
							tr->Rollback();
						}
					}
				}
				if (_antenna.type == STRIPE)
				{
					for(size_t j = 0; j<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE.size(); ++j)
					{
						IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
						tr->Start();
						try
						{
							IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
							st->Prepare("insert into EXCITATION_BY_VOLTAGE_SOURCE (ID_OUTPUT, EXCITATIONINDEX, FREQUENCY, WAVELENGTH, OPENCIRCUITVOLTAGE, PHASE, SOURCESEGMLABEL, ABSOLNUMSEGMS, LOCATIONEXCITX, LOCATIONEXCITY, LOCATIONEXCITZ, POSITIVEFEEDDIRX, POSITIVEFEEDDIRY, POSITIVEFEEDDIRZ) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
							st->Set(1, idOutputParOneFreq);
							st->Set(2,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].ExcitationIndex);
							st->Set(3,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].Frequency);
							st->Set(4,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].Wavelength);
							st->Set(5,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].OpenCircuitVoltage);
							st->Set(6,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].Phase);
							st->Set(7,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].SourceSegmLabel);
							st->Set(8,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].AbsolNumSegms);
							st->Set(9,  _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].LocationExcitX);
							st->Set(10, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].LocationExcitY);
							st->Set(11, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].LocationExcitZ);
							st->Set(12, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].PositiveFeedDirX);
							st->Set(13, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].PositiveFeedDirY);
							st->Set(14, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[j].PositiveFeedDirZ);
							st->Execute();
							tr->Commit();
						}
						catch (...)
						{
							tr->Rollback();
						}
					}
				}
			}

			if (_antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS)
			{
				if (_antenna.type == WIRE)
				{
					IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
					tr->Start();
					try
					{
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
						st->Prepare("insert into DIRECTIVITY_PATTERN_PARAMS (ID_OUTPUT, GAINPOWER, GAINPOWERLOSS, DTHETA, DPHI, POLARHORIZWATT, POLARHORIZPERS, POLARVERTWATT, POLARVERTPERS, POLARSWATT, POLARSPERS, POLARZWATT, POLARZPERS, POLARLEFTHANDWATT, POLARLEFTHANDPERS, POLARRIGHTHANDWATT, POLARRIGHTHANDPERS) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
						st->Set(1, idOutputParOneFreq);
						st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.GainPower);
						st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss);
						st->Set(4, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.DTheta);
						st->Set(5, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.DPhi);
						st->Set(6, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarHorizWatt);
						st->Set(7, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarHorizPers);
						st->Set(8, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarVertWatt);
						st->Set(9, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarVertPers);
						st->Set(10, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarSWatt);
						st->Set(11, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarSPers);
						st->Set(12, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarZWatt);
						st->Set(13, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarZPers);
						st->Set(14, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandWatt);
						st->Set(15, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandPers);
						st->Set(16, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandWatt);
						st->Set(17, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandPers);
						st->Execute();
						tr->Commit();
					}
					catch (...)
					{
						tr->Rollback();
					}
				}
				if (_antenna.type == STRIPE || _antenna.type == PLANE)
				{
					IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
					tr->Start();
					try
					{
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
						st->Prepare("insert into DIRECTIVITY_PATTERN_PARAMS (ID_OUTPUT, GAINPOWER, GAINPOWERLOSS) values (?,?,?)");
						st->Set(1, idOutputParOneFreq);
						st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.GainPower);
						st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss);
						st->Execute();
						tr->Commit();
					}
					catch (...)
					{
						tr->Rollback();
					}
				}
			}

			if (_antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS)
			{
				if (_antenna.type == STRIPE)
				{
					IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
					tr->Start();
					try
					{
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
						st->Prepare("insert into LOSSES_IN_DIELECTRIC_VOLUME_ELE (ID_OUTPUT, POWERLOSS, MAXIMUMSARVALUE, AVERAGEDSARVALUE) values (?,?,?,?)");
						st->Set(1, idOutputParOneFreq);
						st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.PowerLoss);
						st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.MaximumSARValue);
						st->Set(4, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.AveragedSARValue);
						st->Execute();
						tr->Commit();
					}
					catch (...)
					{
						tr->Rollback();
					}
				}
			}

			if (_antenna.outputPar.findSUMMARY_OF_LOSSES)
			{
				IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
				tr->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
					st->Prepare("insert into SUMMARY_OF_LOSSES (ID_OUTPUT, METALLICELEMENTS, MISMATCHFEED, NONRADIATINGNETWORKS, BACKPOWERPASSWAVEGUIDEPORTS, BACKPOWERPASSMODALPORTS, SUMALLLOSSES, EFFICIENCYTHEANTENNA, BASEDTOTALACTIVEPOWER) values (?,?,?,?,?,?,?,?,?)");
					st->Set(1, idOutputParOneFreq);
					st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.MetallicElements);
					st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.MismatchFeed);
					st->Set(4, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.NonRadiatingNetworks);
					st->Set(5, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.BackPowerPassWaveguidePorts);
					st->Set(6, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.BackPowerPassModalPorts);
					st->Set(7, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.SumAllLosses);
					st->Set(8, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.EfficiencyTheAntenna);
					st->Set(9, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SUMMARY_OF_LOSSES.BasedTotalActivePower);
					st->Execute();
					tr->Commit();
				}
				catch (...)
				{
					tr->Rollback();
				}
			}

			if (_antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI)
			{
				for(size_t j = 0; j<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI.size(); ++j)
				{
					IBPP::Transaction tr = IBPP::TransactionFactory(*dataBase_);
					tr->Start();
					try
					{
						IBPP::Statement st = IBPP::StatementFactory(*dataBase_, tr);
						st->Prepare("insert into DIRECTIVITY_PATTERN_THETA_PHI (ID_OUTPUT, TETTA, PHI, ETHETAMAGN, ETHETAPHASE, EPHIMAGN, EPHIPHASE, DIRECTIVITYVERT, DIRECTIVITYHORIZ, DIRECTIVITYTOTAL, POLARIZATIONAXIAL, POLARIZATIONANGLE, GAIN) values (?,?,?,?,?,?,?,?,?,?,?,?,?)");
						st->Set(1, idOutputParOneFreq);
						st->Set(2, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].Tetta);
						st->Set(3, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].Phi);
						st->Set(4, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EthetaMagn);
						st->Set(5, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EthetaPhase);
						st->Set(6, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EphiMagn);
						st->Set(7, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].EphiPhase);
						st->Set(8, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].DirectivityVert);
						st->Set(9, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].DirectivityHoriz);
						st->Set(10, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].DirectivityTotal);
						st->Set(11, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].PolarizationAxial);
						st->Set(12, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].PolarizationAngle);
						st->Set(13, _antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_DIRECTIVITY_PATTERN_THETA_PHI[j].Gain);
						st->Execute();
						tr->Commit();
					}
					catch (...)
					{
						tr->Rollback();
					}
				}
			}
		}
		//cout << "output: " << (double)(clock() - tStart) / CLOCKS_PER_SEC << endl;
	}


	else
	{
		return 1;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////