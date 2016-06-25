
#include "ibpp/ibpp.h"
#include "FrbrdDatabase.h"
#include <time.h>
#include "Antenna.h"

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
	unsigned long largestChunkSize = 64 * 1024 - 1;

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
	//   catch (IBPP::Exception*)
	//  {
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

/// @details Запись антенны в базу данных
int FrbrdDatabase::WriteAntennaData(Antenna &_antenna)
{
	int idAntenna, idOutputPar, idInputPar, idOutputParOneFreq;
	if ((*dataBase_)->Connected())
	{
		{
			IBPP::Transaction trAntenna = IBPP::TransactionFactory(*dataBase_);
			trAntenna->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trAntenna);
				st->Prepare("insert into antenna (type, path_out, path_pre) values (?, ?, ?) returning id");
				st->Set(1, _antenna.type);
				st->Set(2, _antenna.pathOut);
				st->Set(3, _antenna.pathPre);
				st->Execute();
				st->Get(1, idAntenna);
				trAntenna->Commit();
			}
			catch (...)
			{
				trAntenna->Rollback();
			}
		}

		{
			IBPP::Transaction trInput = IBPP::TransactionFactory(*dataBase_);
			trInput->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInput);
				st->Prepare("insert into INPUT_PARAMS (ID_ANTENNA, SCALE, RADIUS, DLIN, SEG_LEN, SEG_RAD, FREQ_MIN, FREQ_MAX, FREQ_NUM, FRACTAL_N, FRACTAL_M, ANGLE_DIPOLE_X, ANGLE_DIPOLE_Y, ANGLE_DIPOLE_Z) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) returning id");
				st->Set(1, idAntenna);
				st->Set(2, _antenna.inputPar.Scale);
				st->Set(3, _antenna.inputPar.Radius);
				st->Set(4, _antenna.inputPar.dlin);
				st->Set(5, _antenna.inputPar.seg_len);
				st->Set(6, _antenna.inputPar.seg_rad);
				st->Set(7, _antenna.inputPar.Fmin);
				st->Set(8, _antenna.inputPar.Fmax);
				st->Set(9, _antenna.inputPar.Fnum);
				st->Set(10, _antenna.inputPar.fr_N);
				st->Set(11, _antenna.inputPar.fr_m);
				st->Set(12, _antenna.inputPar.AngleDipoleX);
				st->Set(13, _antenna.inputPar.AngleDipoleY);
				st->Set(14, _antenna.inputPar.AngleDipoleZ);
				st->Execute();
				st->Get(1, idInputPar);
				trInput->Commit();
			}
			catch (...)
			{
				trInput->Rollback();
			}
		}

		{
			for (size_t i=0; i<_antenna.inputPar.fr_D11.size(); ++i)
			{
				IBPP::Transaction trInputFractalAff = IBPP::TransactionFactory(*dataBase_);
				trInputFractalAff->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInputFractalAff);
					st->Prepare("insert into INPUT_FRACTAL_AFFINE_TRANS (ID_INPUT_PARAM, D11, D12, D21, D22, LAMBDA, ALFA) values (?, ?, ?, ?, ?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.fr_D11[i]);
					st->Set(3, _antenna.inputPar.fr_D12[i]);
					st->Set(4, _antenna.inputPar.fr_D21[i]);
					st->Set(5, _antenna.inputPar.fr_D22[i]);
					st->Set(6, _antenna.inputPar.fr_lam[i]);
					st->Set(7, _antenna.inputPar.fr_al[i]);
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
			for (size_t i=0; i<_antenna.inputPar.fr_pT.size(); ++i)
			{
				IBPP::Transaction trInputFractalPoints = IBPP::TransactionFactory(*dataBase_);
				trInputFractalPoints->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInputFractalPoints);
					st->Prepare("insert into INPUT_FRACTAL_POINTS (ID_INPUT_PARAM, P_T, P_X, P_Y) values (?, ?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.fr_pT[i]);
					st->Set(3, _antenna.inputPar.fr_pX[i]);
					st->Set(4, _antenna.inputPar.fr_pY[i]);
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
			for (size_t i=0; i<_antenna.inputPar.fr_pred1X.size(); ++i)
			{
				IBPP::Transaction trInput1PreFractalPoints = IBPP::TransactionFactory(*dataBase_);
				trInput1PreFractalPoints->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInput1PreFractalPoints);
					st->Prepare("insert into INPUT_PREFRACTAL_FIRST_POINTS (ID_INPUT_PARAM, P_X, P_Y) values (?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.fr_pred1X[i]);
					st->Set(3, _antenna.inputPar.fr_pred1Y[i]);
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
			for (size_t i=0; i<_antenna.inputPar.fr_predmX.size(); ++i)
			{
				IBPP::Transaction trInput1PreFractalPoints = IBPP::TransactionFactory(*dataBase_);
				trInput1PreFractalPoints->Start();
				try
				{
					IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trInput1PreFractalPoints);
					st->Prepare("insert into INPUT_PREFRACTAL_M_POINTS (ID_INPUT_PARAM, P_X, P_Y) values (?, ?, ?)");
					st->Set(1, idInputPar);
					st->Set(2, _antenna.inputPar.fr_predmX[i]);
					st->Set(3, _antenna.inputPar.fr_predmY[i]);
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
			IBPP::Transaction trOutput = IBPP::TransactionFactory(*dataBase_);
			trOutput->Start();
			try
			{
				IBPP::Statement st = IBPP::StatementFactory(*dataBase_, trOutput);
				st->Prepare("insert into OUTPUT_PARAMS (ID_ANTENNA, FST_S11, FST_W, FST_BANDWIDTH, SCND_S11, SCND_W, SCND_BANDWIDTH) values (?, ?, ?, ?, ?, ?, ?) returning id");
				st->Set(1, idAntenna);
				st->Set(2, _antenna.outputPar.fst_s11);
				st->Set(3, _antenna.outputPar.fst_w);
				st->Set(4, _antenna.outputPar.fst_bandwidth);
				st->Set(5, _antenna.outputPar.scnd_s11);
				st->Set(6, _antenna.outputPar.scnd_w);
				st->Set(7, _antenna.outputPar.scnd_bandwidth);
				st->Execute();
				st->Get(1, idOutputPar);
				trOutput->Commit();
			}
			catch (...)
			{
				trOutput->Rollback();
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
				if (_antenna.type == STRIPE)
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
				if (_antenna.type == WIRE)
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
				if (_antenna.type == STRIPE)
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
				if (_antenna.type == WIRE)
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
	}


	else
	{
		return 1;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////