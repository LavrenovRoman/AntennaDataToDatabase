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
#include <memory>

using namespace std;

ParseFekoFile::ParseFekoFile()
{
	vsPre.reserve(100);
	vsOut.reserve(50000);
}

void ParseFekoFile::ParseFileComment(const std::string &_file, Experiment& _experiment)
{
	std::string current_str, word;
	std::vector<std::string> vs;
	ifstream file(_file.c_str());
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

void ParseFekoFile::ParseFileOut(const std::string &_file, Antenna& _antenna)
{
	int valueInt = 0;
	double valueDouble;
	std::string words, token;
	_antenna.pathOut = _file;

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

	std::string allFile(
		(std::istreambuf_iterator<char>(
		*(std::unique_ptr<std::ifstream>(
		new std::ifstream(_file.c_str()))).get())),
		std::istreambuf_iterator<char>());

	vsOut.clear();
	vsOut.reserve(50000);
	for (size_t i = 0; i < allFile.size(); ++i)
	{
		size_t beginS = i;
		while (allFile.at(i) != '\n') i++;
		size_t endS = i;
		vsOut.push_back(allFile.substr(beginS, endS - beginS));

		if (!_antenna.outputPar.findDATA_FOR_MEMORY_USAGE                && vsOut.back().find("DATA FOR MEMORY USAGE") != string::npos)				  { _antenna.outputPar.findDATA_FOR_MEMORY_USAGE = true; }
		if (!_antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA            && vsOut.back().find("DATA FOR DIELECTRIC MEDIA") != string::npos)            { _antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA = true; }
		if (!_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE         && vsOut.back().find("EXCITATION BY VOLTAGE SOURCE AT") != string::npos)      { _antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE = true; }
	//	if (!_antenna.outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX        && vsOut.back().find("STORAGE") != string::npos)                              { _antenna.outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX = true;}
		if (!_antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE           && vsOut.back().find("DATA OF THE VOLTAGE SOURCE") != string::npos)           { _antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE = true; }
		if (!_antenna.outputPar.findSCATTERING_PARAMETERS                && vsOut.back().find("SCATTERING PARAMETERS") != string::npos)                { _antenna.outputPar.findSCATTERING_PARAMETERS = true; }
		if (!_antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS && vsOut.back().find("LOSSES IN DIELECTRIC VOLUME ELEMENTS") != string::npos) { _antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS = true; }
		if (!_antenna.outputPar.findSUMMARY_OF_LOSSES                    && vsOut.back().find("SUMMARY OF LOSSES") != string::npos)                    { _antenna.outputPar.findSUMMARY_OF_LOSSES = true; }
		if (!_antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI        && vsOut.back().find("SCATTERED ELECTRIC FIELD STRENGTH") != string::npos)    { _antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI = true; }
		if (!_antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS           && vsOut.back().find("directivity/gain") != string::npos)                     { _antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS = true; }
		if (!_antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS       && vsOut.back().find("Data for the individual layers") != string::npos)       { _antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS = true; }

		i++;
	}

	if (vsOut.back().find("Finished:") == string::npos)
	{
		_antenna.aborted = true;
		return;
	}

	size_t cs = 0;
	if (_antenna.outputPar.findDATA_FOR_MEMORY_USAGE)
	{
		while (vsOut[cs].find("Surface of all triangles in m*m:") == string::npos && vsOut[cs].find("Length of the segments in m:") == string::npos) { cs++; }
		if (_antenna.type == STRIPE || _antenna.type == PLANE)
		{
			words = "Surface of all triangles in m*m:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				_antenna.outputPar._DATA_FOR_MEMORY_USAGE.SurfAllTri_MM = valueDouble;
			}
		}
		if (_antenna.type == STRIPE || _antenna.type == WIRE)
		{
			words = "Length of the segments in m:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				_antenna.outputPar._DATA_FOR_MEMORY_USAGE.LengthAllSegms_M = valueDouble;
			}
		}

		words = "DATA FOR MEMORY USAGE";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		words = "Number of metallic triangles:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicTri = valueInt;
		}
		words = "Number of dielectric triangles:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectrTri = valueInt;
		}
		words = "Number of aperture triangles:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureTri = valueInt;
		}
		words = "GO tr";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumGoTri = valueInt;
		}
		words = "windscreen tr";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWindscreenTri = valueInt;
		}
		words = "Number of FEM surface triangles:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFemSurfaceTri = valueInt;
		}
		words = "Number of modal port triangles:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumModalPortTri = valueInt;
		}
		words = "Number of metallic segments:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicSegms = valueInt;
		}
		words = "dielectr./magnet. cuboids:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDieMagnCubs = valueInt;
		}
		words = "tetrahedra";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumTetrahedra = valueInt;
		}
		words = "Number of edges in PO region:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesPORegion = valueInt;
		}
		words = "Number of wedges in PO region:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumWedgesPORegion = valueInt;
		}
		words = "Number of Fock regions:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumFockRegions = valueInt;
		}
		words = "Number of polygonal surfaces:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumPolySurfaces = valueInt;
		}
		words = "Number of UTD cylinders:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumUTDCylindres = valueInt;
		}
		words = "Number of metallic edges (MoM):";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesMoM = valueInt;
		}
		words = "Number of metallic edges (PO):";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMetallicEdgesPO = valueInt;
		}
		words = "Number of dielectric edges (MoM):";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesMoM = valueInt;
		}
		words = "Number of dielectric edges (PO):";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricEdgesPO = valueInt;
		}
		words = "Number of aperture edges (MoM):";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumApertureEdgesMoM = valueInt;
		}
		words = "Number of edges FEM/MoM surface:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumEdgesFEMMomSurf = valueInt;
		}
		words = "Number of nodes between segments:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumNodesBetweenSegms = valueInt;
		}
		words = "Number of connection points:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumConnectionPoints = valueInt;
		}
		words = "Number of dielectric cuboids:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumDielectricCuboids = valueInt;
		}
		words = "Number of magnetic cuboids:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumMagneticCuboids = valueInt;
		}
		words = "Number of basis funct. for MoM:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctMoM = valueInt;
		}
		words = "Number of basis funct. for PO:";
		while (vsOut[cs].find(words) == string::npos) { cs++; }
		{
			stringstream ss(vsOut[cs]);
			getline(ss, token, ':');
			getline(ss, token, ':');
			valueInt = stoi(token);
			_antenna.outputPar._DATA_FOR_MEMORY_USAGE.NumBasisFunctPO = valueInt;
		}
	}
	while (cs < vsOut.size())
	{
		bool stop = false;
		DATA_FOR_ONE_FREQ NEW_DATA_FOR_ONE_FREQ;
		if (_antenna.outputPar.findDATA_FOR_DIELECTRIC_MEDIA)
		{
			words = "DATA FOR DIELECTRIC MEDIA";
			while (vsOut[cs].find(words) == string::npos)
			{
				cs++;
				if (cs == vsOut.size()) { stop = true;  break; }
			}
			if (stop) { break; }

			cs++;
			cs++;
			cs++;
			while (vsOut[cs].size() > 2 && vsOut[cs].find("EXCITATION") == string::npos)
			{
				DATA_FOR_DIELECTRIC_MEDIA NEW_DATA_FOR_DIELECTRIC_MEDIA;
				stringstream ss(vsOut[cs]);
				ss >> NEW_DATA_FOR_DIELECTRIC_MEDIA.InternalIndex
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermittivity
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermeability
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.Conductivity
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaElectric
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaMagnetic
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.MassDensity;
				NEW_DATA_FOR_ONE_FREQ._VEC_DATA_FOR_DIELECTRIC_MEDIA.push_back(NEW_DATA_FOR_DIELECTRIC_MEDIA);
				cs++;
			}
		}

		if (_antenna.outputPar.findEXCITATION_BY_VOLTAGE_SOURCE)
		{
			//while (true)
			//{
				EXCITATION_BY_VOLTAGE_SOURCE NEW_EXCITATION_BY_VOLTAGE_SOURCE;
				words = "EXCITATION BY VOLTAGE SOURCE AT";
				while (vsOut[cs].find(words) == string::npos)
				{
					cs++;
					if (cs == vsOut.size()) { stop = true;  break; }
				}
				if (stop) { break; }
				//words = "N =";
				//while (vsOut[cs].find(words) == string::npos) { cs++; }
				//{
				//	stringstream ss(vsOut[cs]);
				//	getline(ss, token, '=');
				//	getline(ss, token, '=');
				//	valueInt = stoi(token);
				//	NEW_EXCITATION_BY_VOLTAGE_SOURCE.ExcitationIndex = valueInt;
				//}
				words = "Frequency in Hz:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, '=');
					getline(ss, token, '=');
					valueDouble = stod(token);
					NEW_EXCITATION_BY_VOLTAGE_SOURCE.Frequency = valueDouble;
				}
				words = "Wavelength in m:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, '=');
					getline(ss, token, '=');
					valueDouble = stod(token);
					NEW_EXCITATION_BY_VOLTAGE_SOURCE.Wavelength = valueDouble;
				}
				words = "Open circuit voltage in V:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, '=');
					getline(ss, token, '=');
					valueDouble = stod(token);
					NEW_EXCITATION_BY_VOLTAGE_SOURCE.OpenCircuitVoltage = valueDouble;
				}
				words = "Phase in deg.:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, '=');
					getline(ss, token, '=');
					valueDouble = stod(token);
					NEW_EXCITATION_BY_VOLTAGE_SOURCE.Phase = valueDouble;
				}
				if (_antenna.type == PLANE)
				{
					words = "Electrical edge length in m:";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.ElectricalEdgeLength = valueDouble;
					}
				}
				if (_antenna.type == STRIPE || _antenna.type == WIRE)
				{
					words = "ULA";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueInt = stoi(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.SourceSegmLabel = valueInt;
					}
					words = "UNR";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueInt = stoi(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.AbsolNumSegms = valueInt;
					}
					words = "Location of the excit. in m:";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitX = valueDouble;
					}
					words = "Y";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitY = valueDouble;
					}
					words = "Z";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.LocationExcitZ = valueDouble;
					}
					words = "Positive feed direction:";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirX = valueDouble;
					}
					words = "Y";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirY = valueDouble;
					}
					words = "Z";
					while (vsOut[cs].find(words) == string::npos) { cs++; }
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, '=');
						getline(ss, token, '=');
						valueDouble = stod(token);
						NEW_EXCITATION_BY_VOLTAGE_SOURCE.PositiveFeedDirZ = valueDouble;
					}
				}
				NEW_DATA_FOR_ONE_FREQ._EXCITATION_BY_VOLTAGE_SOURCE = NEW_EXCITATION_BY_VOLTAGE_SOURCE;

				//while (vsOut[cs].find("BY VOLTAGE SOURCE") == string::npos && vsOut[cs].find("GREEN") == string::npos && vsOut[cs].find("MATRIX") == string::npos) { cs++; }
				//if (vsOut[cs].find("GREEN") != string::npos || vsOut[cs].find("MATRIX") != string::npos) { break; }
			//}
		}

		if (_antenna.outputPar.findDATA_FOR_THE_INDIVIDUAL_LAYERS)
		{
			//double dx,dy,dz;
			words = "Data for the individual layers";
			while (vsOut[cs].find(words) == string::npos)
			{
				cs++;
				if (cs == vsOut.size()) { stop = true;  break; }
			}
			if (stop) { break; }

			cs++;
			cs++;
			cs++;
			while (vsOut[cs].size() > 2)
			{
				DATA_FOR_DIELECTRIC_MEDIA NEW_DATA_FOR_DIELECTRIC_MEDIA;
				stringstream ss(vsOut[cs]);
				ss >> NEW_DATA_FOR_DIELECTRIC_MEDIA.InternalIndex;
				vsOut[cs].erase(0, 44);
				stringstream ss1(vsOut[cs]);
				ss1 >> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermittivity
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.RelPermeability
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.Conductivity
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaElectric
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.TanDeltaMagnetic
					>> NEW_DATA_FOR_DIELECTRIC_MEDIA.MassDensity;
				NEW_DATA_FOR_ONE_FREQ._VEC_DATA_FOR_DIELECTRIC_MEDIA.push_back(NEW_DATA_FOR_DIELECTRIC_MEDIA);
				cs++;
			}
		}

		// 			if (_antenna.outputPar.findDISTRIBUTED_STORAGE_OF_MATRIX)
		// 			{
		// 				if (_antenna.type == STRIPE)
		// 				{
		// 					words = "STORAGE";
		// 					while (vsOut[cs].find(words) == string::npos) 
		// 					{
		//						cs++;
		//						if (cs == vsOut.size()) { stop = true;  break; }
		// 					}
		// 					if (stop) {break;}
		// 					words = "Number of rows of the matrix";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumRowsMatrix = valueInt;
		// 					}
		// 					words = "Number of columns of the matrix";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumColsMatrix = valueInt;
		// 					}
		// 					words = "Number of rows of the process grid";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumRowsProcessGrid = valueInt;
		// 					}
		// 					words = "Number of columns of the process grid";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumColsProcessGrid = valueInt;
		// 					}
		// 					words = "Block size";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.BlockSize = valueInt;
		// 					}
		// 					words = "Theoretical load in percent";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueDouble = stod(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.TheoreticalLoadPercent = valueDouble;
		// 					}
		// 					words = "Number of rows of local matrix";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumRowsLocalMatrix = valueInt;
		// 					}
		// 					words = "Number of columns of local mat";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.NumColsLocalMatrix = valueInt;
		// 					}
		// 					words = "Local memory requirement for matrix";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.LocalMemoryReqMatrix = valueInt;
		// 					}
		// 					words = "Memory requirement for MoM matrix:";
		// 					while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 					{
		// 						stringstream ss(vsOut[cs]);
		// 						getline(ss, token, '=');
		// 						getline(ss, token, '=');
		// 						valueInt = stoi(token);
		// 						NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.MemoryReqMoMMatrix = valueInt;
		// 					}
		// 				}
		// 				words = "CPU time for preconditioning:";
		// 				while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 				{
		// 					stringstream ss(vsOut[cs]);
		// 					getline(ss, token, ':');
		// 					getline(ss, token, ':');
		// 					valueDouble = stod(token);
		// 					NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.CPUTimePreconditioning = valueDouble;
		// 				}
		// 				words = "Condition number of the matrix:";
		// 				while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 				{
		// 					stringstream ss(vsOut[cs]);
		// 					getline(ss, token, ':');
		// 					getline(ss, token, ':');
		// 					valueDouble = stod(token);
		// 					NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.ConditionNumMatrix = valueDouble;
		// 				}
		// 				words = "CPU time for solving the linear set of equations:";
		// 				while (vsOut[cs].find(words) == string::npos) {cs++;}
		// 				{
		// 					stringstream ss(vsOut[cs]);
		// 					getline(ss, token, ':');
		// 					getline(ss, token, ':');
		// 					valueDouble = stod(token);
		// 					NEW_DATA_FOR_ONE_FREQ._DISTRIBUTED_STORAGE_OF_MATRIX.CPUTimeSolvLinearEquations = valueDouble;
		// 				}
		// 			}

		if (_antenna.outputPar.findDATA_OF_THE_VOLTAGE_SOURCE)
		{
			//while (true)
			//{
				DATA_OF_THE_VOLTAGE_SOURCE NEW_DATA_OF_THE_VOLTAGE_SOURCE;

				words = "DATA OF THE VOLTAGE SOURCE";
				while (vsOut[cs].find(words) == string::npos)
				{
					cs++;
					if (cs == vsOut.size()) { stop = true;  break; }
				}
				if (stop) { break; }
				words = "Current";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, 'A');
					getline(ss, token, 'A');
					stringstream ss1(token);
					ss1 >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentRealPart
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentImagPart
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentMagnitude
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.CurrentPhase;
				}
				words = "Admitt";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, 'V');
					getline(ss, token, 'V');
					stringstream ss1(token);
					ss1 >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittRealPart
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittImagPart
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittMagnitude
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.AdmittPhase;
				}
				words = "Impedance";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, 'm');
					getline(ss, token, 'm');
					getline(ss, token, 'm');
					stringstream ss1(token);
					ss1 >> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceRealPart
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceImagPart
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedanceMagnitude
						>> NEW_DATA_OF_THE_VOLTAGE_SOURCE.ImpedancePhase;
				}
				while (vsOut[cs].find("Inductance") == string::npos && vsOut[cs].find("Capacitance") == string::npos) { cs++; }
				{
					if (vsOut[cs].find("Inductance") != string::npos)
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, 'H');
						getline(ss, token, 'H');
						valueDouble = stod(token);
						NEW_DATA_OF_THE_VOLTAGE_SOURCE.Inductance = valueDouble;
						NEW_DATA_OF_THE_VOLTAGE_SOURCE.Capacitance = 0;
					}
					if (vsOut[cs].find("Capacitance") != string::npos)
					{
						stringstream ss(vsOut[cs]);
						getline(ss, token, 'F');
						getline(ss, token, 'F');
						valueDouble = stod(token);
						NEW_DATA_OF_THE_VOLTAGE_SOURCE.Inductance = 0;
						NEW_DATA_OF_THE_VOLTAGE_SOURCE.Capacitance = valueDouble;
					}
				}
				words = "Power in Watt:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_OF_THE_VOLTAGE_SOURCE.Power = valueDouble;
				}
				NEW_DATA_FOR_ONE_FREQ._DATA_OF_THE_VOLTAGE_SOURCE = NEW_DATA_OF_THE_VOLTAGE_SOURCE;
				//while (vsOut[cs].find("DATA OF THE VOLTAGE SOURCE") == string::npos && vsOut[cs].find("SCATTERING PARAMETERS") == string::npos && vsOut[cs].find("LOSSES") == string::npos) { cs++; }
				//if (vsOut[cs].find("SCATTERING PARAMETERS") != string::npos || vsOut[cs].find("LOSSES") != string::npos) { break; }
			//}
		}

		if (_antenna.outputPar.findSCATTERING_PARAMETERS)
		{
			words = "SCATTERING PARAMETERS";
			while (vsOut[cs].find(words) == string::npos)
			{
				cs++;
				if (cs == vsOut.size()) { stop = true;  break; }
			}
			if (stop) { break; }
			{
				cs++;
				cs++;
				cs++;
				string _s;
				stringstream ss(vsOut[cs]);
				ss >> _s
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SPortSinc
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SPortSource
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SRealPart
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SImagPart
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SMagnitudeLinear
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SMagnitudeDB
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SPhase;
			}
			words = "Sum |S|^2 of these S-parameters:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				stringstream ss1(token);
				ss1 >> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SumS2MagnitudeLinear
					>> NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SumS2MagnitudeDB;
			}

			NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.S11 = sqrt(NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SRealPart*NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SRealPart +
				NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SImagPart*NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.SImagPart);

			NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.VSWR = (1 + abs(NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.S11)) /
				(1 - abs(NEW_DATA_FOR_ONE_FREQ._SCATTERING_PARAMETERS.S11));
		}

		if (_antenna.outputPar.findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS)
		{
			if (_antenna.type == STRIPE)
			{
				words = "LOSSES IN DIELECTRIC VOLUME ELEMENTS";
				while (vsOut[cs].find(words) == string::npos)
				{
					cs++;
					if (cs == vsOut.size()) { stop = true;  break; }
				}
				if (stop) { break; }
				words = "Power loss in Watt:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.PowerLoss = valueDouble;
				}
				words = "Maximum SAR value in Watt";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS.MaximumSARValue = valueDouble;
				}
				words = "Averaged SAR value in Watt";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
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
			while (vsOut[cs].find(words) == string::npos)
			{
				cs++;
				if (cs == vsOut.size()) { stop = true;  break; }
			}
			if (stop) { break; }
			words = "Metallic elements:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.MetallicElements = valueDouble;
			}
			words = "Mismatch at feed:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.MismatchFeed = valueDouble;
			}
			words = "Non-radiating networks:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.NonRadiatingNetworks = valueDouble;
			}
			words = "Backward power at passive waveguide ports:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.BackPowerPassWaveguidePorts = valueDouble;
			}
			words = "Backward power at passive modal ports:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.BackPowerPassModalPorts = valueDouble;
			}
			words = "Sum of all losses:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.SumAllLosses = valueDouble;
			}
			words = "Efficiency of the antenna:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.EfficiencyTheAntenna = valueDouble;
			}
			words = "based on a total active power:";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, ':');
				getline(ss, token, ':');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._SUMMARY_OF_LOSSES.BasedTotalActivePower = valueDouble;
			}
		}

		if (_antenna.outputPar.findDIRECTIVITY_PATTERN_THETA_PHI)
		{
			words = "SCATTERED ELECTRIC FIELD STRENGTH";
			while (vsOut[cs].find(words) == string::npos)
			{
				cs++;
				if (cs == vsOut.size()) { stop = true;  break; }
			}
			if (stop) { break; }
			cs++;
			cs++;
			cs++;
			cs++;
			while (vsOut[cs].size() > 2 && vsOut[cs].find("Gain") == string::npos)
			{
				DIRECTIVITY_PATTERN_THETA_PHI NEW_DIRECTIVITY_PATTERN_THETA_PHI;
				stringstream ss(vsOut[cs]);
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
				cs++;
			}
		}

		if (_antenna.outputPar.findDIRECTIVITY_PATTERN_PARAMS)
		{
			words = "directivity/gain";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, 'f');
				getline(ss, token, 'f');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.GainPower = valueDouble;
			}
			words = "power loss";
			while (vsOut[cs].find(words) == string::npos) { cs++; }
			{
				stringstream ss(vsOut[cs]);
				getline(ss, token, 'f');
				getline(ss, token, 'f');
				valueDouble = stod(token);
				NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.GainPowerLoss = valueDouble;
			}

			if (_antenna.type == WIRE)
			{
				words = "grid DTHETA";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, '=');
					getline(ss, token, '=');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.DTheta = valueDouble;
					getline(ss, token, '=');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.DPhi = valueDouble;
				}
				words = "horizontal polarisation:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarHorizWatt = valueDouble;
					stringstream ss1(vsOut[cs]);
					getline(ss1, token, '(');
					getline(ss1, token, '(');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarHorizPers = valueDouble;
				}
				words = "vertical polarisation:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarVertWatt = valueDouble;
					stringstream ss1(vsOut[cs]);
					getline(ss1, token, '(');
					getline(ss1, token, '(');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarVertPers = valueDouble;
				}
				words = "S polarisation:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarSWatt = valueDouble;
					stringstream ss1(vsOut[cs]);
					getline(ss1, token, '(');
					getline(ss1, token, '(');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarSPers = valueDouble;
				}
				words = "Z polarisation:";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarZWatt = valueDouble;
					stringstream ss1(vsOut[cs]);
					getline(ss1, token, '(');
					getline(ss1, token, '(');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarZPers = valueDouble;
				}
				words = "left hand circular pol";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandWatt = valueDouble;
					stringstream ss1(vsOut[cs]);
					getline(ss1, token, '(');
					getline(ss1, token, '(');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarLeftHandPers = valueDouble;
				}
				words = "right hand circular pol";
				while (vsOut[cs].find(words) == string::npos) { cs++; }
				{
					stringstream ss(vsOut[cs]);
					getline(ss, token, ':');
					getline(ss, token, ':');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandWatt = valueDouble;
					stringstream ss1(vsOut[cs]);
					getline(ss1, token, '(');
					getline(ss1, token, '(');
					valueDouble = stod(token);
					NEW_DATA_FOR_ONE_FREQ._DIRECTIVITY_PATTERN_PARAMS.PolarRightHandPers = valueDouble;
				}
			}
		}

		_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.push_back(NEW_DATA_FOR_ONE_FREQ);
	}

	std::vector<double> vecS11, vecW, vecS11Db;
	for (size_t i=0; i<_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ.size(); ++i)
	{
		vecS11.push_back(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.S11);
		vecW.push_back(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._EXCITATION_BY_VOLTAGE_SOURCE.Frequency);
		vecS11Db.push_back(_antenna.outputPar._VEC_DATA_FOR_ONE_FREQ[i]._SCATTERING_PARAMETERS.SMagnitudeDB);
	}
	size_t N = vecS11.size();
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

void ParseFekoFile::ParseFilePre(const std::string &_file, Antenna& _antenna)
{
	//setlocale(LC_ALL, "Russian");
	std::string word;
	_antenna.pathPre = _file;
	std::vector<std::string> vs;

	_antenna.inputPar.findRADIATOR  = false;
	_antenna.inputPar.findFEED      = false;
	_antenna.inputPar.findSUBSTRATE = false;
	_antenna.inputPar.findGROUND    = false;

	std::string allFile(
		(std::istreambuf_iterator<char>(
		*(std::shared_ptr<std::ifstream>(
		new std::ifstream(_file.c_str()))).get())),
		std::istreambuf_iterator<char>());

	vsPre.clear();
	vsPre.reserve(100);
	for (size_t i = 0; i < allFile.size(); ++i)
	{
		size_t beginS = i;
		while (allFile.at(i) != '\n') i++;
		size_t endS = i;
		vsPre.push_back(allFile.substr(beginS, endS - beginS));
		if (!_antenna.inputPar.findRADIATOR && vsPre.back().find("RADIATOR") != string::npos)    { _antenna.inputPar.findRADIATOR = true; }
		if (!_antenna.inputPar.findFEED && vsPre.back().find("FEED") != string::npos)            { _antenna.inputPar.findFEED = true; }
		if (!_antenna.inputPar.findSUBSTRATE && vsPre.back().find("SUBSTRATE") != string::npos)  { _antenna.inputPar.findSUBSTRATE = true; }
		if (!_antenna.inputPar.findGROUND && vsPre.back().find("GROUND") != string::npos)        { _antenna.inputPar.findGROUND = true; }

		if (_antenna.inputPar.findRADIATOR && _antenna.inputPar.findFEED &&
			_antenna.inputPar.findSUBSTRATE && _antenna.inputPar.findGROUND &&
			vsPre.back().find("END PHYSICAL ANTENNA PARAMS") != string::npos)
		{
			allFile = allFile.substr(0, ++i);
			break;
		}
		i++;
	}

	if (vsPre.back().find("END PHYSICAL ANTENNA PARAMS") == string::npos || !_antenna.inputPar.findRADIATOR)
	{
		_antenna.aborted = true;
		return;
	}

	size_t cs = 0;
	while (cs < vsPre.size())
	{
		while (vsPre[cs].find("TYPE") == string::npos) {cs++;}
		{
			cs++;
			stringstream ss(vsPre[cs]);
			vs.clear();
			while (ss >> word) vs.push_back(word);
			if (vs[1] == "wire")	   _antenna.type = WIRE;
			if (vs[1] == "microstrip") _antenna.type = STRIPE;
			if (vs[1] == "plane")      _antenna.type = PLANE;
		}

		if (_antenna.inputPar.findRADIATOR)
		{
			while (vsPre[cs].find("RADIATOR") == string::npos) {cs++;}
			while (vsPre[cs].find("ScaleX") == string::npos) {cs++;}
			{
				cs++;
				stringstream ss(vsPre[cs]);
				vs.clear();
				while (ss >> word) vs.push_back(word);
				_antenna.inputPar.Radiator.ScaleX = stod(vs[1]);
				_antenna.inputPar.Radiator.ScaleY = stod(vs[2]);
				_antenna.inputPar.Radiator.Radius_StripWidth_FeedLineWidth = stod(vs[3]);
			}
			while (vsPre[cs].find("N, m:") == string::npos) {cs++;}
			{
				cs++;
				stringstream ss(vsPre[cs]);
				vs.clear();
				while (ss >> word) vs.push_back(word);
				_antenna.inputPar.Radiator.fr_N = stoi(vs[1]);
				_antenna.inputPar.Radiator.fr_m = stoi(vs[2]);
			}
			while (vsPre[cs].find("x[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_pX.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("y[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_pY.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("t[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_pT.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("D11[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_D11.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("D12[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_D12.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("D21[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_D21.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("D22[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_D22.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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

			while (vsPre[cs].find("lam[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_lam.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("al[]") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_al.clear();
				while (true)
				{
					cs++;
					stringstream ss(vsPre[cs]);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					for (size_t i = 1; i < vs.size() - 1; ++i)
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
			while (vsPre[cs].find("first order prefractal") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_pred1X.clear();
				_antenna.inputPar.Radiator.fr_pred1Y.clear();
				int c = 0;
				while (true)
				{
					cs++;
					size_t s_ = 0;
					while (s_ < vsPre[cs].size())
					{
						if (vsPre[cs][s_] == '(' || vsPre[cs][s_] == ')' || vsPre[cs][s_] == ',')
						{
							vsPre[cs].erase(vsPre[cs].begin() + s_);
							continue;
						}
						++s_;
					}
					stringstream ss(vsPre[cs]);
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

			while (vsPre[cs].find("m-th order prefractal") == string::npos) {cs++;}
			{
				_antenna.inputPar.Radiator.fr_predmX.clear();
				_antenna.inputPar.Radiator.fr_predmY.clear();
				int c = 0;
				while (true)
				{
					cs++;
					size_t s_ = 0;
					while (s_ < vsPre[cs].size())
					{
						if (vsPre[cs][s_] == '(' || vsPre[cs][s_] == ')' || vsPre[cs][s_] == ',')
						{
							vsPre[cs].erase(vsPre[cs].begin() + s_);
							continue;
						}
						++s_;
					}
					stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("FEED") == string::npos) {cs++;}
			while (vsPre[cs].find("Coordinates of feedpoint:") == string::npos) {cs++;}
			{
				cs++;
				stringstream ss(vsPre[cs]);
				vs.clear();
				while (ss >> word) vs.push_back(word);
				_antenna.inputPar.Feed.FeedX = stod(vs[1]);
				_antenna.inputPar.Feed.FeedY = stod(vs[2]);
			}
		}

		if (_antenna.inputPar.findSUBSTRATE)
		{
			while (vsPre[cs].find("SUBSTRATE") == string::npos) {cs++;}
			while (vsPre[cs].find("permittivity") == string::npos) {cs++;}
			{
				cs++;
				stringstream ss(vsPre[cs]);
				vs.clear();
				while (ss >> word) vs.push_back(word);
				_antenna.inputPar.Substrate.Permittivity = stod(vs[1]);
				_antenna.inputPar.Substrate.LossTangent = stod(vs[2]);
				_antenna.inputPar.Substrate.Density = stod(vs[3]);
				_antenna.inputPar.Substrate.Thickness = stod(vs[4]);
			}
			while (vsPre[cs].find("coordinate:") == string::npos) {cs++;}
			{
				cs++;
				stringstream ss(vsPre[cs]);
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
			while (vsPre[cs].find("GROUND") == string::npos) {cs++;}
			while (vsPre[cs].find("of ground:") == string::npos) {cs++;}
			{
				_antenna.inputPar.Ground.coordX.clear();
				_antenna.inputPar.Ground.coordY.clear();
				int c = 0;
				while (true)
				{
					cs++;
					size_t s_ = 0;
					while (s_ < vsPre[cs].size())
					{
						if (vsPre[cs][s_] == '(' || vsPre[cs][s_] == ')' || vsPre[cs][s_] == ',')
						{
							vsPre[cs].erase(vsPre[cs].begin() + s_);
							continue;
						}
						++s_;
					}
					stringstream ss(vsPre[cs]);
					vs.clear();
					while (ss >> word) vs.push_back(word);
					for (size_t i = 1; i < vs.size() - 1; ++i, ++c)
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