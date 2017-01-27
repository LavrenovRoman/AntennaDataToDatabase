#include <vector>
#include <ctime>

enum TYPE {WIRE, STRIPE, PLANE};

struct InputParameters 
{
	struct RADIATOR
	{
		double ScaleX, ScaleY;
		double Radius_StripWidth_FeedLineWidth;
		int fr_N;
		int fr_m;
		std::vector<double> fr_pT, fr_pX, fr_pY, fr_lam, fr_al;
		std::vector<double> fr_D11, fr_D12, fr_D21, fr_D22;
		std::vector<double> fr_pred1X, fr_pred1Y, fr_predmX, fr_predmY;
	} Radiator;
	
	struct FEED
	{
		double FeedX, FeedY;
	} Feed;

	struct SUBSTRATE
	{
		double Permittivity;
		double LossTangent;
		double Density;
		double Thickness;
		double CoorLeftUpX,    CoorLeftUpY;
		double CoorRightDownX, CoorRightDownY;
	} Substrate;

	struct GROUND
	{
		std::vector<double> coordX, coordY;
	} Ground;

	bool findRADIATOR;
	bool findFEED;
	bool findSUBSTRATE;
	bool findGROUND;
};

struct DATA_FOR_MEMORY_USAGE
{
	double LengthAllSegms_M;	//WIRE
	double SurfAllTri_MM;		//STRIPE
	int NumMetallicTri;
	int NumDielectrTri;
	int NumApertureTri;
	int NumGoTri;
	int NumWindscreenTri;
	int NumFemSurfaceTri;
	int NumModalPortTri;
	int NumMetallicSegms;
	int NumDieMagnCubs;
	int NumTetrahedra;
	int NumEdgesPORegion;
	int NumWedgesPORegion;
	int NumFockRegions;
	int NumPolySurfaces;
	int NumUTDCylindres;
	int NumMetallicEdgesMoM;
	int NumMetallicEdgesPO;
	int NumDielectricEdgesMoM;
	int NumDielectricEdgesPO;
	int NumApertureEdgesMoM;
	int NumEdgesFEMMomSurf;
	int NumNodesBetweenSegms;
	int NumConnectionPoints;
	int NumDielectricCuboids;
	int NumMagneticCuboids;
	int NumBasisFunctMoM;
	int NumBasisFunctPO;
};

struct DATA_FOR_DIELECTRIC_MEDIA
{
	int    InternalIndex;
	double RelPermittivity;
	double RelPermeability;
	double Conductivity;
	double TanDeltaElectric;
	double TanDeltaMagnetic;
	double MassDensity;
};

struct EXCITATION_BY_VOLTAGE_SOURCE
{
	int    ExcitationIndex;
	double Frequency;
	double Wavelength;
	double OpenCircuitVoltage;
	double Phase;
	double ElectricalEdgeLength;	//STRIPE
	int    SourceSegmLabel;			//WIRE
	int    AbsolNumSegms;			//WIRE
	double LocationExcitX;			//WIRE
	double LocationExcitY;			//WIRE
	double LocationExcitZ;			//WIRE
	double PositiveFeedDirX;		//WIRE
	double PositiveFeedDirY;		//WIRE
	double PositiveFeedDirZ;		//WIRE
};

struct DISTRIBUTED_STORAGE_OF_MATRIX
{
	int NumRowsMatrix;			//STRIPE
	int NumColsMatrix;			//STRIPE
	int NumRowsProcessGrid;		//STRIPE
	int NumColsProcessGrid;		//STRIPE
	int BlockSize;				//STRIPE
	double TheoreticalLoadPercent;	//STRIPE
	int NumRowsLocalMatrix;		//STRIPE
	int NumColsLocalMatrix;		//STRIPE
	int LocalMemoryReqMatrix;	//STRIPE
	int MemoryReqMoMMatrix;		//STRIPE
	double CPUTimePreconditioning;
	double ConditionNumMatrix;
	double CPUTimeSolvLinearEquations;
};

struct DATA_OF_THE_VOLTAGE_SOURCE
{
	double CurrentRealPart;
	double CurrentImagPart;
	double CurrentMagnitude;
	double CurrentPhase;
	double AdmittRealPart;
	double AdmittImagPart;
	double AdmittMagnitude;
	double AdmittPhase;
	double ImpedanceRealPart;
	double ImpedanceImagPart;
	double ImpedanceMagnitude;
	double ImpedancePhase;
	double Inductance;
	double Capacitance;
	double Power;
};

struct SCATTERING_PARAMETERS
{
	int    SPortSinc;
	int    SPortSource;
	double SRealPart;
	double SImagPart;
	double SMagnitudeLinear;
	double SMagnitudeDB;
	double SPhase;
	double SumS2MagnitudeLinear;
	double SumS2MagnitudeDB;
	double S11;
	double VSWR;
	double EMAX;
};

struct LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS
{
	double PowerLoss;				//STRIPE
	double MaximumSARValue;			//STRIPE
	double AveragedSARValue;		//STRIPE
};

struct SUMMARY_OF_LOSSES
{
	double MetallicElements;
	double MismatchFeed;
	double NonRadiatingNetworks;
	double BackPowerPassWaveguidePorts;
	double BackPowerPassModalPorts;
	double SumAllLosses;
	double EfficiencyTheAntenna;
	double BasedTotalActivePower;
};

struct DIRECTIVITY_PATTERN_THETA_PHI
{
	double Tetta;
	double Phi;
	double EthetaMagn;
	double EthetaPhase;
	double EphiMagn;
	double EphiPhase;
	double DirectivityVert;
	double DirectivityHoriz;
	double DirectivityTotal;
	double PolarizationAxial;
	double PolarizationAngle;
	double Gain;
};

struct DIRECTIVITY_PATTERN_PARAMS
{
	double GainPower;
	double GainPowerLoss;
	double DTheta;					//STRIPE
	double DPhi;					//STRIPE
	double PolarHorizWatt;			//STRIPE
	double PolarHorizPers;			//STRIPE
	double PolarVertWatt;			//STRIPE
	double PolarVertPers;			//STRIPE
	double PolarSWatt;				//STRIPE
	double PolarSPers;				//STRIPE
	double PolarZWatt;				//STRIPE
	double PolarZPers;				//STRIPE
	double PolarLeftHandWatt;		//STRIPE
	double PolarLeftHandPers;		//STRIPE
	double PolarRightHandWatt;		//STRIPE
	double PolarRightHandPers;		//STRIPE
};

struct DATA_FOR_ONE_FREQ
{
	std::vector<DATA_FOR_DIELECTRIC_MEDIA> _VEC_DATA_FOR_DIELECTRIC_MEDIA;
	std::vector<EXCITATION_BY_VOLTAGE_SOURCE> _VEC_EXCITATION_BY_VOLTAGE_SOURCE; //can delete vector
//	DISTRIBUTED_STORAGE_OF_MATRIX _DISTRIBUTED_STORAGE_OF_MATRIX;
	std::vector<DATA_OF_THE_VOLTAGE_SOURCE> _VEC_DATA_OF_THE_VOLTAGE_SOURCE; //can delete vector ??
	SCATTERING_PARAMETERS _SCATTERING_PARAMETERS;
 	LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS _LOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS;
	SUMMARY_OF_LOSSES _SUMMARY_OF_LOSSES;
	std::vector<DIRECTIVITY_PATTERN_THETA_PHI> _VEC_DIRECTIVITY_PATTERN_THETA_PHI;
	DIRECTIVITY_PATTERN_PARAMS _DIRECTIVITY_PATTERN_PARAMS;
};

struct OutputParameters
{
	DATA_FOR_MEMORY_USAGE _DATA_FOR_MEMORY_USAGE;
	std::vector<DATA_FOR_ONE_FREQ> _VEC_DATA_FOR_ONE_FREQ;
	double fst_s11;
	double fst_w;
	double fst_bandwidth;
	double scnd_s11;
	double scnd_w;
	double scnd_bandwidth;
	double third_s11;
	double third_w;
	double third_bandwidth;

	bool findDATA_FOR_MEMORY_USAGE;
	bool findDATA_FOR_DIELECTRIC_MEDIA;
	bool findDATA_FOR_THE_INDIVIDUAL_LAYERS;
	bool findEXCITATION_BY_VOLTAGE_SOURCE;
//	bool findDISTRIBUTED_STORAGE_OF_MATRIX;
	bool findDATA_OF_THE_VOLTAGE_SOURCE;
	bool findSCATTERING_PARAMETERS;
	bool findLOSSES_IN_DIELECTRIC_VOLUME_ELEMENTS;
	bool findSUMMARY_OF_LOSSES;
	bool findDIRECTIVITY_PATTERN_THETA_PHI;
	bool findDIRECTIVITY_PATTERN_PARAMS;
};

struct Antenna
{
	TYPE type;
	bool aborted;
	std::string pathOut;
	std::string pathPre;
	InputParameters inputPar;
	OutputParameters outputPar;
};

struct Experiment_Param
{
	std::string name;
	double pBegin, pEnd, pStep;
};

struct Experiment
{
	tm date;
	std::string comment;
	std::vector<Experiment_Param> cycles;
};
