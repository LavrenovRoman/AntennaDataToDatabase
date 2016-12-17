#include "ParseFekoFile.h"
#include "Antenna.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <locale.h>
#include <string>
#include <locale>
#include <windows.h>
#include <cstdlib>
#include <cstddef>

#include <QString>

using namespace std;

void ParseFekoFile::ParseFileComment(QString _file, Experiment& _experiment)
{
	std::string current_str, word, token;
	std::vector<std::string> vs;
	ifstream file(_file.toLocal8Bit().constData());
	if (file.is_open())
	{
		getline(file, current_str);
		size_t s_ = 0;
		while (s_ < current_str.size())
		{
			if (current_str[s_] == '.') current_str[s_] = ' ';
			if (current_str[s_] == '/') current_str[s_] = ' ';
			++s_;
		}
		stringstream ss(current_str);
		vs.clear();
		while (ss >> word) vs.push_back(word);
		_experiment.date.tm_year = stoi(vs[2]);
		_experiment.date.tm_mon  = stoi(vs[1]);
		_experiment.date.tm_mday = stoi(vs[0]);

		getline(file, current_str);
		_experiment.comment = current_str;
		
		getline(file, current_str);
		while(!(file.eof()))
		{
			Experiment_Param _experiment_Param;
			vs.clear();
			stringstream ss(current_str);
			while (ss >> word) vs.push_back(word);
			_experiment_Param.name =   vs[0];
			_experiment_Param.pBegin = stod(vs[1]);
			_experiment_Param.pEnd =   stod(vs[2]);
			_experiment_Param.pStep =  stod(vs[3]);
			_experiment.cycles.push_back(_experiment_Param);
			getline(file, current_str);
			if (current_str.size() < 2) break;
		}
	}
}

void ParseFekoFile::ParseFileOut(QString _file, Antenna& _antenna)
{
	int valueInt, nomer = 0;
	double valueDouble;
	std::string current_str, words, token;

	_antenna.outputPar.findDATA_FOR_MEMORY_USAGE                = false;
	_antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA            = false;
	_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE         = false;
//	_antenna.outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX        = false;
	_antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE           = false;
	_antenna.outputPar.findSCATTERING_PARAMETERS                = false;
	_antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS = false;
	_antenna.outputPar.findSUMMARY_OF_LOSSES                    = false;
	_antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI        = false;
	_antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS           = false;
	_antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS       = false;

	_antenna.aborted = false;
	bool findFinish = false;
	ifstream fileFinish(_file.toLocal8Bit().constData());
	_antenna.pathOut = _file.toLocal8Bit().constData();
	while(!(fileFinish.eof()))
	{
		if (current_str.find("Finished:") == string::npos)
		{
			if (current_str.find("DATA FOR MEMORY USAGE") != string::npos)	              {_antenna.outputPar.findDATA_FOR_MEMORY_USAGE                = true;}
			if (current_str.find("DATA FOR DIELECTRIC MEDIA") != string::npos)            {_antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA            = true;}
			if (current_str.find("EXCITATION BY VOLTAGE SOURCE AT") != string::npos)      {_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE         = true;}
//			if (current_str.find("STORAGE") != string::npos)                              {_antenna.outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX        = true;}
			if (current_str.find("DATA OF THE VOLTAGE SOURCE") != string::npos)           {_antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE           = true;}
			if (current_str.find("SCATTERING PARAMETERS") != string::npos)                {_antenna.outputPar.findSCATTERING_PARAMETERS                = true;}
			if (current_str.find("LOSSES IN DIELECTRIC VOLUME ELEMENTS") != string::npos) {_antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS = true;}
			if (current_str.find("SUMMARY OF LOSSES") != string::npos)                    {_antenna.outputPar.findSUMMARY_OF_LOSSES                    = true;}
			if (current_str.find("SCATTERED ELECTRIC FIELD STRENGTH") != string::npos)    {_antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI        = true;}
			if (current_str.find("directivity/gain") != string::npos)                     {_antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS           = true;}
			if (current_str.find("Data for the individual layers") != string::npos)       {_antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS       = true;}

			getline(fileFinish, current_str);
		}
		else
		{
			findFinish = true;
			fileFinish.close();
			break;
		}
	}
	if (!findFinish) 
	{
		_antenna.aborted = true;
		return;
	}
	
	ifstream file(_file.toLocal8Bit().constData());
	if (file.is_open())
	{
#pragma region REG_DATA_FOR_MEMORY_USAGE
		if (_antenna.outputPar.findDATA_FOR_MEMORY_USAGE)
		{
			while(!(file.eof()))
			{
				while (current_str.find("Surface of all triangles in m*m:") == string::npos &&
					   current_str.find("Length of the segments in m:") == string::npos)
				{getline(file, current_str);}
			
//				if (current_str.find("Surface of all triangles in m*m:") != string::npos)
//				{
//					_antenna.type = STRIPE;
//				}
//				if (current_str.find("Length of the segments in m:") != string::npos)
//				{
//					_antenna.type = WIRE;
//				}

				if (_antenna.type == STRIPE || _antenna.type == PLANE)
				{
					words = "Surface of all triangles in m*m:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						_antenna.outputPar._DATA_FOR_MEMORY_USAGE.SurfAllTri_MM = valueDouble;
					}
				}

				if (_antenna.type == STRIPE || _antenna.type == WIRE)
				{
					words = "Length of the segments in m:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						_antenna.outputPar._DATA_FOR_MEMORY_USAGE.LengthAllSegms_M = valueDouble;
					}
				}

				words = "DATA FOR MEMORY USAGE";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				words = "Number of metallic triangles:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicTri = valueInt;
				}
				words = "Number of dielectric triangles:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectrTri = valueInt;
				}
				words = "Number of aperture triangles:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureTri = valueInt;
				}
				words = "GO tr";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumGoTri = valueInt;
				}
				words = "windscreen tr";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWindscreenTri = valueInt;
				}
				words = "Number of FEM surface triangles:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFemSurfaceTri = valueInt;
				}
				words = "Number of modal port triangles:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumModalPortTri = valueInt;
				}
				words = "Number of metallic segments:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicSegms = valueInt;
				}
				words = "dielectr./magnet. cuboids:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDieMagnCubs = valueInt;
				}
				words = "tetrahedra";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumTetrahedra = valueInt;
				}
				words = "Number of edges in PO region:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesPORegion = valueInt;
				}
				words = "Number of wedges in PO region:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWedgesPORegion = valueInt;
				}
				words = "Number of Fock regions:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFockRegions = valueInt;
				}
				words = "Number of polygonal surfaces:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumPolySurfaces = valueInt;
				}
				words = "Number of UTD cylinders:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumUTDCylindres = valueInt;
				}
				words = "Number of metallic edges (MoM):";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesMoM = valueInt;
				}
				words = "Number of metallic edges (PO):";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesPO = valueInt;
				}
				words = "Number of dielectric edges (MoM):";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesMoM = valueInt;
				}
				words = "Number of dielectric edges (PO):";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesPO = valueInt;
				}
				words = "Number of aperture edges (MoM):";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureEdgesMoM = valueInt;
				}
				words = "Number of edges FEM/MoM surface:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesFEMMomSurf = valueInt;
				}
				words = "Number of nodes between segments:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumNodesBetweenSegms = valueInt;
				}
				words = "Number of connection points:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumConnectionPoints = valueInt;
				}
				words = "Number of dielectric cuboids:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricCuboids = valueInt;
				}
				words = "Number of magnetic cuboids:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMagneticCuboids = valueInt;
				}
				words = "Number of basis funct. for MoM:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctMoM = valueInt;
				}
				words = "Number of basis funct. for PO:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueInt = stoi(token);
					_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctPO = valueInt;
				}
				break;
			}
		}
#pragma endregion
#pragma region REG_VEC_DATA_FOR_ONE_FREQ
		while(!(file.eof()))
		{
			bool stop = false;
			DATA_FOR_ONE_FREQ NEW_DATA_FOR_ONE_FREQ;
			if (_antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA)
			{
				words = "DATA FOR DIELECTRIC MEDIA";
				while (current_str.find(words) == string::npos) 
				{
					if (!(file.eof())) {getline(file, current_str);}
					else			   {stop = true;  break;}
				}
				if (stop) {break;}

				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				while (current_str.size() > 2)
				{
					DATA_FOR_DIELECTRIC_MEDIA NEW_DATA_FOR_DIELECTRIC_MEDIA;
					stringstream ss(current_str);
					ss >> NEW_DATA_FOR_DIELECTRIC_MEDIA.InternalIndex
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermittivity
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermeability
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.Conductivity
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaElectric
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaMagnetic
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.MassDensity;
					NEW_DATA_FOR_ONE_FREQ._VEC_DATA_FOR_DIELECTRIC_MEDIA.push_back(NEW_DATA_FOR_DIELECTRIC_MEDIA);
					getline(file, current_str);
				}
			}

			if (_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE)
			{
				while (true)
				{
					EXCITATION_BY_VOLTAGE_SOURCE NEW_EXCITATION_BY_VOLTAGE_SOURCE;
					words = "EXCITATION BY VOLTAGE SOURCE AT";
					while (current_str.find(words) == string::npos) 
					{
						if (!(file.eof())) {getline(file, current_str);}
						else			   {stop = true;  break;}
					}
					if (stop) {break;}
					words = "N =";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueInt = stoi(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.ExcitationIndex = valueInt;
					}
					words = "Frequency in Hz:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.Frequency = valueDouble;
					}
					words = "Wavelength in m:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.Wavelength = valueDouble;
					}
					words = "Open circuit voltage in V:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.OpenCircuitVoltage = valueDouble;
					}
					words = "Phase in deg.:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.Phase = valueDouble;
					}
					if (_antenna.type == PLANE)
					{
						words = "Electrical edge length in m:";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.ElectricalEdgeLength = valueDouble;
						}
					}
					if (_antenna.type == STRIPE || _antenna.type == WIRE)
					{
						words = "ULA";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueInt = stoi(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.SourceSegmLabel = valueInt;
						}
						words = "UNR";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueInt = stoi(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.AbsolNumSegms = valueInt;
						}
						words = "Location of the excit. in m:";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitX = valueDouble;
						}
						words = "Y";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitY = valueDouble;
						}
						words = "Z";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitZ = valueDouble;
						}
						words = "Positive feed direction:";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirX = valueDouble;
						}
						words = "Y";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirY = valueDouble;
						}
						words = "Z";
						while (current_str.find(words) == string::npos) {getline(file, current_str);}
						{
							stringstream ss(current_str);
							getline(ss, token, '=');
							getline(ss, token, '=');
							valueDouble = stod(token);
							NEW_EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirZ = valueDouble;
						}
					}
					NEW_DATA_FOR_ONE_FREQ._VEC_EXCITATION_BY_VOLTAGE_SOURCE.push_back(NEW_EXCITATION_BY_VOLTAGE_SOURCE);

					while (current_str.find("BY VOLTAGE SOURCE") == string::npos && current_str.find("GREEN") == string::npos && current_str.find("MATRIX") == string::npos) {getline(file, current_str);}
					if (current_str.find("GREEN") != string::npos || current_str.find("MATRIX") != string::npos) {break;}
				}
			}

			if (_antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS)
			{
				//double dx,dy,dz;
				words = "Data for the individual layers";
				while (current_str.find(words) == string::npos) 
				{
					if (!(file.eof())) {getline(file, current_str);}
					else			   {stop = true;  break;}
				}
				if (stop) {break;}

				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				while (current_str.size() > 2)
				{
					DATA_FOR_DIELECTRIC_MEDIA NEW_DATA_FOR_DIELECTRIC_MEDIA;
					stringstream ss(current_str);
					ss >> NEW_DATA_FOR_DIELECTRIC_MEDIA.InternalIndex;
					current_str.erase(0,44);
					stringstream ss1(current_str);
					ss1	>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermittivity
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermeability
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.Conductivity
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaElectric
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaMagnetic
						>> NEW_DATA_FOR_DIELECTRIC_MEDIA.MassDensity;
					NEW_DATA_FOR_ONE_FREQ._VEC_DATA_FOR_DIELECTRIC_MEDIA.push_back(NEW_DATA_FOR_DIELECTRIC_MEDIA);
					getline(file, current_str);
				}
			}

// 			if (_antenna.outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX)
// 			{
// 				if (_antenna.type == STRIPE)
// 				{
// 					words = "STORAGE";
// 					while (current_str.find(words) == string::npos) 
// 					{
// 						if (!(file.eof())) {getline(file, current_str);}
// 						else			   {stop = true;  break;}
// 					}
// 					if (stop) {break;}
// 					words = "Number of rows of the matrix";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumRowsMatrix = valueInt;
// 					}
// 					words = "Number of columns of the matrix";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumColsMatrix = valueInt;
// 					}
// 					words = "Number of rows of the process grid";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumRowsProcessGrid = valueInt;
// 					}
// 					words = "Number of columns of the process grid";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumColsProcessGrid = valueInt;
// 					}
// 					words = "Block size";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.BlockSize = valueInt;
// 					}
// 					words = "Theoretical load in percent";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueDouble = stod(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.TheoreticalLoadPercent = valueDouble;
// 					}
// 					words = "Number of rows of local matrix";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumRowsLocalMatrix = valueInt;
// 					}
// 					words = "Number of columns of local mat";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumColsLocalMatrix = valueInt;
// 					}
// 					words = "Local memory requirement for matrix";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.LocalMemoryReqMatrix = valueInt;
// 					}
// 					words = "Memory requirement for MoM matrix:";
// 					while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 					{
// 						stringstream ss(current_str);
// 						getline(ss, token, '=');
// 						getline(ss, token, '=');
// 						valueInt = stoi(token);
// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.MemoryReqMoMMatrix = valueInt;
// 					}
// 				}
// 				words = "CPU time for preconditioning:";
// 				while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 				{
// 					stringstream ss(current_str);
// 					getline(ss, token, ':');
// 					getline(ss, token, ':');
// 					valueDouble = stod(token);
// 					NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.CPUTimePreconditioning = valueDouble;
// 				}
// 				words = "Condition number of the matrix:";
// 				while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 				{
// 					stringstream ss(current_str);
// 					getline(ss, token, ':');
// 					getline(ss, token, ':');
// 					valueDouble = stod(token);
// 					NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.ConditionNumMatrix = valueDouble;
// 				}
// 				words = "CPU time for solving the linear set of equations:";
// 				while (current_str.find(words) == string::npos) {getline(file, current_str);}
// 				{
// 					stringstream ss(current_str);
// 					getline(ss, token, ':');
// 					getline(ss, token, ':');
// 					valueDouble = stod(token);
// 					NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.CPUTimeSolvLinearEquations = valueDouble;
// 				}
// 			}

			if (_antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE)
			{
				while (true)
				{
					DATA_OF_THE_VOLTAGE_SOURCE NEW_DATA_OF_THE_VOLTAGE_SOURCE;

					words = "DATA OF THE VOLTAGE SOURCE";
					while (current_str.find(words) == string::npos) 
					{
						if (!(file.eof())) {getline(file, current_str);}
						else			   {stop = true;  break;}
					}
					if (stop) {break;}
					words = "Current";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, 'A');
						getline(ss, token, 'A');
						stringstream ss1(token);
						ss1 >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentRealPart
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentImagPart
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentMagnitude
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentPhase;
					}
					words = "Admitt";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, 'V');
						getline(ss, token, 'V');
						stringstream ss1(token);
						ss1 >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittRealPart
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittImagPart
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittMagnitude
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittPhase;
					}
					words = "Impedance";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, 'm');
						getline(ss, token, 'm');
						getline(ss, token, 'm');
						stringstream ss1(token);
						ss1 >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceRealPart
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceImagPart
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceMagnitude
						   >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedancePhase;
					}
					while (current_str.find("Inductance") == string::npos && current_str.find("Capacitance") == string::npos) {getline(file, current_str);}
					{
						if (current_str.find("Inductance") != string::npos)
						{
							stringstream ss(current_str);
							getline(ss, token, 'H');
							getline(ss, token, 'H');
							valueDouble = stod(token);
							NEW_DATA_OF_THE_VOLTAGE_SOURCE.Inductance = valueDouble;
							NEW_DATA_OF_THE_VOLTAGE_SOURCE.Capacitance = 0;
						}
						if (current_str.find("Capacitance") != string::npos)
						{
							stringstream ss(current_str);
							getline(ss, token, 'F');
							getline(ss, token, 'F');
							valueDouble = stod(token);
							NEW_DATA_OF_THE_VOLTAGE_SOURCE.Inductance = 0;
							NEW_DATA_OF_THE_VOLTAGE_SOURCE.Capacitance = valueDouble;
						}
					}
					words = "Power in Watt:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_OF_THE_VOLTAGE_SOURCE.Power = valueDouble;
					}
					NEW_DATA_FOR_ONE_FREQ._VEC_DATA_OF_THE_VOLTAGE_SOURCE.push_back(NEW_DATA_OF_THE_VOLTAGE_SOURCE);
					while (current_str.find("DATA OF THE VOLTAGE SOURCE") == string::npos && current_str.find("SCATTERING PARAMETERS") == string::npos && current_str.find("LOSSES") == string::npos) {getline(file, current_str);}
					if (current_str.find("SCATTERING PARAMETERS") != string::npos || current_str.find("LOSSES") != string::npos) {break;}
				}
			}

			if (_antenna.outputPar.findSCATTERING_PARAMETERS)
			{
				words = "SCATTERING PARAMETERS";
				while (current_str.find(words) == string::npos) 
				{
					if (!(file.eof())) {getline(file, current_str);}
					else			   {stop = true;  break;}
				}
				if (stop) {break;}
				{
					getline(file, current_str);
					getline(file, current_str);
					getline(file, current_str);
					getline(file, current_str);
					string _s;
					stringstream ss(current_str);
					ss  >> _s
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SPortSinc
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SPortSource
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SRealPart
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SImagPart
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SMagnitudeLinear
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SMagnitudeDB
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SPhase;
				}
				words = "Sum |S|^2 of these S-parameters:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					stringstream ss1(token);
					ss1 >> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SumS2MagnitudeLinear
						>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SumS2MagnitudeDB;
				}

				NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.S11 = sqrt(NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SRealPart*NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SRealPart +
																		NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SImagPart*NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SImagPart);

				NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.VSWR = (1+abs(NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.S11))/
																	(1-abs(NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.S11));
			}

			if (_antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS)
			{
				if (_antenna.type == STRIPE)
				{
					words = "LOSSES IN DIELECTRIC VOLUME ELEMENTS";
					while (current_str.find(words) == string::npos) 
					{
						if (!(file.eof())) {getline(file, current_str);}
						else			   {stop = true;  break;}
					}
					if (stop) {break;}
					words = "Power loss in Watt:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.PowerLoss = valueDouble;
					}
					words = "Maximum SAR value in Watt";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.MaximumSARValue = valueDouble;
					}
					words = "Averaged SAR value in Watt";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.AveragedSARValue = valueDouble;
					}
				}
			}

			if (_antenna.outputPar.findSUMMARY_OF_LOSSES)
			{
				words = "SUMMARY OF LOSSES";
				while (current_str.find(words) == string::npos) 
				{
					if (!(file.eof())) {getline(file, current_str);}
					else			   {stop = true;  break;}
				}
				if (stop) {break;}
				words = "Metallic elements:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.MetallicElements = valueDouble;
				}
				words = "Mismatch at feed:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.MismatchFeed = valueDouble;
				}
				words = "Non-radiating networks:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.NonRadiatingNetworks = valueDouble;
				}
				words = "Backward power at passive waveguide ports:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.BackPowerPassWaveguidePorts = valueDouble;
				}
				words = "Backward power at passive modal ports:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.BackPowerPassModalPorts = valueDouble;
				}
				words = "Sum of all losses:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.SumAllLosses = valueDouble;
				}
				words = "Efficiency of the antenna:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.EfficiencyTheAntenna = valueDouble;
				}
				words = "based on a total active power:";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.BasedTotalActivePower = valueDouble;
				}
			}

			if (_antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI)
			{
				words = "SCATTERED ELECTRIC FIELD STRENGTH";
				while (current_str.find(words) == string::npos) 
				{
					if (!(file.eof())) {getline(file, current_str);}
					else			   {stop = true;  break;}
				}
				if (stop) {break;}
				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				getline(file, current_str);
				while (current_str.size() > 2)
				{
					DIRECTIVITY_PATTERN_THETA_PHI NEW_DIRECTIVITY_PATTERN_THETA_PHI;
					stringstream ss(current_str);
					ss >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.Tetta
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.Phi
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.EthetaMagn
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.EthetaPhase
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.EphiMagn
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.EphiPhase
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.DirectivityVert
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.DirectivityHoriz
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.DirectivityTotal
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.PolarizationAxial
					   >> NEW_DIRECTIVITY_PATTERN_THETA_PHI.PolarizationAngle;
					NEW_DIRECTIVITY_PATTERN_THETA_PHI.Gain = sqrt(NEW_DIRECTIVITY_PATTERN_THETA_PHI.EthetaMagn * NEW_DIRECTIVITY_PATTERN_THETA_PHI.EthetaMagn +
																  NEW_DIRECTIVITY_PATTERN_THETA_PHI.EphiMagn   * NEW_DIRECTIVITY_PATTERN_THETA_PHI.EphiMagn);
					if (NEW_DIRECTIVITY_PATTERN_THETA_PHI.Gain > NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.EMAX)
					{
						NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.EMAX = NEW_DIRECTIVITY_PATTERN_THETA_PHI.Gain;
					}

					NEW_DATA_FOR_ONE_FREQ._VEC_DIRECTIVITY_PATTERN_THETA_PHI.push_back(NEW_DIRECTIVITY_PATTERN_THETA_PHI);
					getline(file, current_str);
				}
			}

			if (_antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS)
			{
				words = "directivity/gain";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, 'f');
					getline(ss, token, 'f');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.GainPower = valueDouble;
				}
				words = "power loss";
				while (current_str.find(words) == string::npos) {getline(file, current_str);}
				{
					stringstream ss(current_str);
					getline(ss, token, 'f');
					getline(ss, token, 'f');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss = valueDouble;
				}

				if (_antenna.type == WIRE)
				{
					words = "grid DTHETA";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.DTheta = valueDouble;
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.DPhi = valueDouble;
					}
					words = "horizontal polarisation:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarHorizWatt = valueDouble;
						stringstream ss1(current_str);
						getline(ss1, token, '(');
						getline(ss1, token, '(');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarHorizPers = valueDouble;
					}
					words = "vertical polarisation:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarVertWatt = valueDouble;
						stringstream ss1(current_str);
						getline(ss1, token, '(');
						getline(ss1, token, '(');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarVertPers = valueDouble;
					}
					words = "S polarisation:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarSWatt = valueDouble;
						stringstream ss1(current_str);
						getline(ss1, token, '(');
						getline(ss1, token, '(');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarSPers = valueDouble;
					}
					words = "Z polarisation:";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarZWatt = valueDouble;
						stringstream ss1(current_str);
						getline(ss1, token, '(');
						getline(ss1, token, '(');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarZPers = valueDouble;
					}
					words = "left hand circular pol";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandWatt = valueDouble;
						stringstream ss1(current_str);
						getline(ss1, token, '(');
						getline(ss1, token, '(');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandPers = valueDouble;
					}
					words = "right hand circular pol";
					while (current_str.find(words) == string::npos) {getline(file, current_str);}
					{
						stringstream ss(current_str);
						getline(ss, token, ':');
						getline(ss, token, ':');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandWatt = valueDouble;
						stringstream ss1(current_str);
						getline(ss1, token, '(');
						getline(ss1, token, '(');
						valueDouble = stod(token);
						NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandPers = valueDouble;
					}
				}
			}

			_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.push_back(NEW_DATA_FOR_ONE_FREQ);
		}
#pragma endregion
	}

	std::vector<double> vecS11, vecW, vecS11Db;
	for (size_t i=0; i<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++i)
	{
		vecS11.push_back(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.S11);
		vecW.push_back(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._VEC_EXCITATION_BY_VOLTAGE_SOURCE[0].Frequency);
		vecS11Db.push_back(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SMagnitudeDB);
	}
	int N = vecS11.size();
	int j = 0;
	while ((vecS11[j] > vecS11[j + 1]) && (j < N - 1)) j++;
	_antenna.outputPar.fst_s11 = vecS11[j];
	_antenna.outputPar.fst_w = vecW[j];
	int fst_min = j;
	if (j < (N - 1))
	{
		while ((j < N - 1) && (vecS11[j] <= vecS11[j + 1])) j++;
		while ((j < N - 1) && (vecS11[j] > vecS11[j + 1])) j++;
	}
	_antenna.outputPar.scnd_s11 = vecS11[j];
	_antenna.outputPar.scnd_w = vecW[j];
	int scnd_min = j;
	if (j < (N - 1))
	{
		while ((j < N - 1) && (vecS11[j] <= vecS11[j + 1])) j++;
		while ((j < N - 1) && (vecS11[j] > vecS11[j + 1])) j++;
	}
	_antenna.outputPar.third_s11 = vecS11[j];
	_antenna.outputPar.third_w = vecW[j];
	int third_min = j;


	double maxDB = -10.0;

	{
		if (vecS11Db[fst_min] > maxDB) _antenna.outputPar.fst_bandwidth = 0.0;
		else
		{
			size_t i = fst_min;
			double leftFR, rightFR;
			double a, k;
			while ((i > 0) && (vecS11Db[i] < maxDB)) { i--; };
			k = (vecW[i + 1] - vecW[i]) / (vecS11Db[i + 1] - vecS11Db[i]);
			a = vecW[i] - vecS11Db[i] * k;
			leftFR = k * maxDB + a;
			i = fst_min;
			while ((i < vecS11Db.size() - 2) && (vecS11Db[i] < maxDB)) { i++; };
			k = (vecW[i + 1] - vecW[i]) / (vecS11Db[i + 1] - vecS11Db[i]);
			a = vecW[i] - vecS11Db[i] * k;
			rightFR = k * maxDB + a;
			_antenna.outputPar.fst_bandwidth = rightFR - leftFR;
		}
	}
	{
		if (vecS11Db[scnd_min] > maxDB) _antenna.outputPar.scnd_bandwidth = 0.0;
		else
		{
			size_t i = scnd_min;
			double leftFR, rightFR;
			double a, k;
			while ((i > 0) && (vecS11Db[i] < maxDB)) { i--; };
			k = (vecW[i + 1] - vecW[i]) / (vecS11Db[i + 1] - vecS11Db[i]);
			a = vecW[i] - vecS11Db[i] * k;
			leftFR = k * maxDB + a;
			i = scnd_min;
			while ((i < vecS11Db.size() - 2) && (vecS11Db[i] < maxDB)) { i++; };
			k = (vecW[i + 1] - vecW[i]) / (vecS11Db[i + 1] - vecS11Db[i]);
			a = vecW[i] - vecS11Db[i] * k;
			rightFR = k * maxDB + a;
			_antenna.outputPar.scnd_bandwidth = rightFR - leftFR;
		}
	}
	{
		if (vecS11Db[third_min] > maxDB) _antenna.outputPar.third_bandwidth = 0.0;
		else
		{
			size_t i = third_min;
			double leftFR, rightFR;
			double a, k;
			while ((i > 0) && (vecS11Db[i] < maxDB)) { i--; };
			k = (vecW[i + 1] - vecW[i]) / (vecS11Db[i + 1] - vecS11Db[i]);
			a = vecW[i] - vecS11Db[i] * k;
			leftFR = k * maxDB + a;
			i = third_min;
			while ((i < vecS11Db.size() - 2) && (vecS11Db[i] < maxDB)) { i++; };
			k = (vecW[i + 1] - vecW[i]) / (vecS11Db[i + 1] - vecS11Db[i]);
			a = vecW[i] - vecS11Db[i] * k;
			rightFR = k * maxDB + a;
			_antenna.outputPar.third_bandwidth = rightFR - leftFR;
		}
	}

	valueInt = 0;
}

void ParseFekoFile::ParseFilePre(QString _file, Antenna& _antenna)
{
	//setlocale(LC_ALL, "Russian");
	int nomer = 0;
	std::string current_str, word, token;

	bool findFinish = false;
	ifstream fileFinishEnd(_file.toLocal8Bit().constData());
	_antenna.pathPre = _file.toLocal8Bit().constData();
	while(!(fileFinishEnd.eof()))
	{
		if (current_str.find("END PHYSICAL ANTENNA PARAMS") == string::npos)
		{
			if (current_str.find("RADIATOR") != string::npos)  {_antenna.inputPar.findRADIATOR  = true;}
			if (current_str.find("FEED") != string::npos)      {_antenna.inputPar.findFEED      = true;}
			if (current_str.find("SUBSTRATE") != string::npos) {_antenna.inputPar.findSUBSTRATE = true;}
			if (current_str.find("GROUND") != string::npos)    {_antenna.inputPar.findGROUND    = true;}
			getline(fileFinishEnd, current_str);
		}
		else
		{
			findFinish = true;
			fileFinishEnd.close();
			break;
		}
	}
	if (!findFinish || !_antenna.inputPar.findRADIATOR) 
	{
		_antenna.aborted = true;
		return;
	}

	std::vector<std::string> vs;
	ifstream file(_file.toLocal8Bit().constData());
	if (file.is_open())
	{
		while(!(file.eof()))
		{
			while (current_str.find("TYPE") == string::npos) {getline(file, current_str);}
			{
				getline(file, current_str);
				stringstream ss(current_str);
				vs.clear();
				while (ss >> word) vs.push_back(word);
				if (vs[1] == "wire")	   _antenna.type = WIRE;
				if (vs[1] == "microstrip") _antenna.type = STRIPE;
				if (vs[1] == "plane")      _antenna.type = PLANE;
			}

			if (_antenna.inputPar.findRADIATOR)
			{
				while (current_str.find("RADIATOR") == string::npos) {getline(file, current_str);}
				while (current_str.find("ScaleX") == string::npos) {getline(file, current_str);}
				{
					getline(file, current_str);
					stringstream ss(current_str);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					_antenna.inputPar.Radiator.ScaleX = stod(vs[1]);
					_antenna.inputPar.Radiator.ScaleY = stod(vs[2]);
					_antenna.inputPar.Radiator.Radius_StripWidth_FeedLineWidth = stod(vs[3]);
				}
				while (current_str.find("N, m:") == string::npos) {getline(file, current_str);}
				{
					getline(file, current_str);
					stringstream ss(current_str);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					_antenna.inputPar.Radiator.fr_N = stoi(vs[1]);
					_antenna.inputPar.Radiator.fr_m = stoi(vs[2]);
				}
				while (current_str.find("x[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_pX.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_pX.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_pX.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("y[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_pY.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_pY.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_pY.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("t[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_pT.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_pT.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_pT.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("D11[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_D11.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_D11.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_D11.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("D12[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_D12.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_D12.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_D12.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("D21[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_D21.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_D21.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_D21.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("D22[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_D22.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_D22.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_D22.push_back(stod(vs.back()));
							break;
						}
					}
				}

				while (current_str.find("lam[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_lam.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_lam.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_lam.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("al[]") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_al.clear();
					while (true)
					{
						getline(file, current_str);
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i=1; i<vs.size() - 1; ++i)
						{
							_antenna.inputPar.Radiator.fr_al.push_back(stod(vs[i]));
						}
						if (vs.back() != "**")
						{
							_antenna.inputPar.Radiator.fr_al.push_back(stod(vs.back()));
							break;
						}
					}
				}
				while (current_str.find("first order prefractal") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_pred1X.clear();
					_antenna.inputPar.Radiator.fr_pred1Y.clear();
					int c = 0;
					while (true)
					{
						getline(file, current_str);
						size_t s_ = 0;
						while (s_ < current_str.size())
						{
							if (current_str[s_] == '(' || current_str[s_] == ')' || current_str[s_] == ',')
							{
								current_str.erase(current_str.begin() + s_);
								continue;
							}
							++s_;
						}
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i, ++c)
						{
							if (c % 2 == 0)
							{
								_antenna.inputPar.Radiator.fr_pred1X.push_back(stod(vs[i]));
							}
							if (c % 2 == 1)
							{
								_antenna.inputPar.Radiator.fr_pred1Y.push_back(stod(vs[i]));
							}
						}
						if (vs.back() != "**")
						{
							if (c % 2 == 0)
							{
								_antenna.inputPar.Radiator.fr_pred1X.push_back(stod(vs.back()));
							}
							if (c % 2 == 1)
							{
								_antenna.inputPar.Radiator.fr_pred1Y.push_back(stod(vs.back()));
							}
							break;
						}
					}
				}

				while (current_str.find("m-th order prefractal") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Radiator.fr_predmX.clear();
					_antenna.inputPar.Radiator.fr_predmY.clear();
					int c = 0;
					while (true)
					{
						getline(file, current_str);
						size_t s_ = 0;
						while (s_ < current_str.size())
						{
							if (current_str[s_] == '(' || current_str[s_] == ')' || current_str[s_] == ',')
							{
								current_str.erase(current_str.begin() + s_);
								continue;
							}
							++s_;
						}
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i = 1; i < vs.size() - 1; ++i, ++c)
						{
							if (c % 2 == 0)
							{
								_antenna.inputPar.Radiator.fr_predmX.push_back(stod(vs[i]));
							}
							if (c % 2 == 1)
							{
								_antenna.inputPar.Radiator.fr_predmY.push_back(stod(vs[i]));
							}
						}
						if (vs.back() != "**")
						{
							if (c % 2 == 0)
							{
								_antenna.inputPar.Radiator.fr_predmX.push_back(stod(vs.back()));
							}
							if (c % 2 == 1)
							{
								_antenna.inputPar.Radiator.fr_predmY.push_back(stod(vs.back()));
							}
							break;
						}
					}
				}
			}

			if (_antenna.inputPar.findFEED)
			{
				while (current_str.find("FEED") == string::npos) {getline(file, current_str);}
				while (current_str.find("Coordinates of feedpoint:") == string::npos) {getline(file, current_str);}
				{
					getline(file, current_str);
					stringstream ss(current_str);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					_antenna.inputPar.Feed.FeedX = stod(vs[1]);
					_antenna.inputPar.Feed.FeedY = stod(vs[2]);
				}
			}

			if (_antenna.inputPar.findSUBSTRATE)
			{
				while (current_str.find("SUBSTRATE") == string::npos) {getline(file, current_str);}
				while (current_str.find("permittivity") == string::npos) {getline(file, current_str);}
				{
					getline(file, current_str);
					stringstream ss(current_str);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					_antenna.inputPar.Substrate.Permittivity = stod(vs[1]);
					_antenna.inputPar.Substrate.LossTangent = stod(vs[2]);
					_antenna.inputPar.Substrate.Density = stod(vs[3]);
					_antenna.inputPar.Substrate.Thickness = stod(vs[4]);
				}
				while (current_str.find("coordinate:") == string::npos) {getline(file, current_str);}
				{
					getline(file, current_str);
					stringstream ss(current_str);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					_antenna.inputPar.Substrate.CoorLeftUpX = stod(vs[1]);
					_antenna.inputPar.Substrate.CoorLeftUpY = stod(vs[2]);
					_antenna.inputPar.Substrate.CoorRightDownX = stod(vs[3]);
					_antenna.inputPar.Substrate.CoorRightDownY = stod(vs[4]);
				}
			}

			if (_antenna.inputPar.findGROUND)
			{
				while (current_str.find("GROUND") == string::npos) {getline(file, current_str);}
				while (current_str.find("of ground:") == string::npos) {getline(file, current_str);}
				{
					_antenna.inputPar.Ground.coordX.clear();
					_antenna.inputPar.Ground.coordY.clear();
					int c = 0;
					while (true)
					{
						getline(file, current_str);
						size_t s_ = 0;
						while (s_ < current_str.size())
						{
							if (current_str[s_] == '(' || current_str[s_] == ')' || current_str[s_] == ',')
							{
								current_str.erase(current_str.begin() + s_);
								continue;
							}
							++s_;
						}
						stringstream ss(current_str);
						vs.clear();
						while (ss >> word) vs.push_back(word);
						for (size_t i=1; i<vs.size()-1; ++i, ++c)
						{
							if (c % 2 == 0)
							{
								_antenna.inputPar.Ground.coordX.push_back(stod(vs[i]));
							}
							if (c % 2 == 1)
							{
								_antenna.inputPar.Ground.coordY.push_back(stod(vs[i]));
							}
						}
						if (vs.back() != "**")
						{
							if (c % 2 == 0)
							{
								_antenna.inputPar.Ground.coordX.push_back(stod(vs.back()));
							}
							if (c % 2 == 1)
							{
								_antenna.inputPar.Ground.coordY.push_back(stod(vs.back()));
							}
							break;
						}
					}
				}
			}

			break;
		}
	}
}