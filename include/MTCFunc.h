#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
//#include <unordered_set>

#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TGLabel.h"
#include "TGStatusBar.h"
#include "TGWindow.h"
#include "TGMsgBox.h"
#include "TFile.h"
#include "TTree.h"
#include "TThread.h"
#include "TRandom3.h"

#include <TGButton.h>

#define MAX_CH 16 
#define b_width 2 //2 ns bin width for 500MS/s V1730S
#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b
#define SUBTRAC(a,b) a>b ? (double)(a-b) : - (double)(b-a)


typedef enum{
	InputRange_2Vpp = 0,
	InputRange_0_5Vpp = 1
}CAEN_DGTZ_InputRange_t;

typedef struct{
	int b;
	int ch;
	uint32_t read_num;
	uint32_t timetag; // initially, then go to uint64_t
	//uint64_t timestamp; // initially, then go to uint64_t
}TimeStruct_t;

typedef struct{
	int b;
	int ch;
	uint32_t read_num;
	uint32_t extra; 
	uint32_t timetag; 
	uint64_t timestamp; // initially, then go to uint64_t
	std::vector <double> vec_bl{}, vec{};
	
}DataStruct_t;

typedef struct
{
    //CAEN_DGTZ_ConnectionType LinkType;
	uint32_t PID_M, PID_S;
	int Nch;
	uint32_t SerialNumber; 
	int BL_CUT;
  
	int RecordLength[MAX_CH];
	uint32_t ChannelMask;
	int PreTrigger[MAX_CH];
	int  DCOffset[MAX_CH]; //uint32_t
    int EventAggr;
	CAEN_DGTZ_InputRange_t InputRange[MAX_CH];
    CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_CH];
    CAEN_DGTZ_DPP_AcqMode_t AcqMode;
    CAEN_DGTZ_IOLevel_t IOlev;
	
	//part from CAEN_DGTZ_DPP_PSD_Params_t to not use two structures
	
	int thr[MAX_CH];
	int nsbl[MAX_CH];
	int tvaw[MAX_CH];
	int pgate[MAX_CH];
	int sgate[MAX_CH];
	int lgate[MAX_CH];
	int selft[MAX_CH];
	CAEN_DGTZ_DPP_TriggerConfig_t trgc[MAX_CH];
	int discr[MAX_CH];
	int cfdd[MAX_CH];
	int cfdf[MAX_CH];
	int csens[MAX_CH];
	CAEN_DGTZ_DPP_PUR_t purh; 
	int purgap;
	int blthr;
	int  bltmo; //not used in example but exist in structure of CAEN_DGTZ_DPP_PSD_Params_t
	int trgho;
	
} DigitizerConfig_t;

typedef struct
{	
	bool fPrint;
	bool fTimer;
	bool fInit;
	int timer_val;
	bool fStoreTraces;
	int Nch;
	int N_B; //number of boards
		
	int loop;
	uint32_t Nb[2];
	int Nev[2];
	int TrgCnt[2][MAX_CH];
	uint64_t StartTime;
	double DrawTime;
	
	TGMainFrame *main;
	
	int WindX, WindY;

	TGLabel  *TLabel; // Time Label
	TGLabel  *StatusLabel[2]; // Time Label
	//TGStatusBar *StatusBar;
	TFile *ff; // to store traces
	TTree *tree;
	DataStruct_t Data;
	
} ReadoutConfig_t;

typedef struct
{	
	int WF_XMIN[MAX_CH], WF_XMAX[MAX_CH], WF_YMIN[MAX_CH], WF_YMAX[MAX_CH];
	int ALBound[MAX_CH], ARBound[MAX_CH], ILBound[MAX_CH], IRBound[MAX_CH];
		
	int b; // board
	int handle;

	char h2Style[10];
	bool fDraw[MAX_CH]; // channel draw flag
	bool fTraceAxisModified, fXYPadModified;
	int FirstToDraw; //first channel to draw
	//TCanvas *can;
	int NPad, NPadT;
	int cCharge, cAmpl, cInt, cdT, cPSD_ampl, cPSD_int, cQsl, cIA, cLayers, cCounts, cXY;		
	bool fBL, fTrace, fCharge, fAmpl, fInt, fdT, fPSD_ampl, fPSD_int, fQsl, fIA, fLayers, fLayersCoeff, fCounts, fXY;  // flags for every type of histograms
	
	bool fDChannels, fDTypes, fTTrace, fTCharge, fTAmpl, fTInt, fTPSD_ampl, fTPSD_int, fTIA; // flags for fTab draw "Types"
	int cTPos[MAX_CH];//
	
	int CH_2D; // channel to draw th2d
	double layers_coeff[MAX_CH];
	TH1D *dt, *counts, *layers;	
	TH1D *trace[MAX_CH], *ampl[MAX_CH], *integral[MAX_CH], *charge[MAX_CH];
	TH2D *int_ampl[MAX_CH], *psd_ampl[MAX_CH], *psd_int[MAX_CH], *qs_ql, *xy; 
		
	int PSD_BIN[MAX_CH];
	
	int evt, evt_out, ch_out;
	uint32_t ext_out, tst_out;
	uint64_t time_out;
	std::vector <double> vec_bl, vec;
		
} Histograms_t;


long get_time();

bool compareByTime(const TimeStruct_t& a, const TimeStruct_t& b);

void SortTime(std::vector<TimeStruct_t>&times, TimeStruct_t newElement);

void DrawRndmHisto(TH1D *h, TCanvas *can);

void ManageCanvas( TCanvas *can, Histograms_t *Histo, int N_CH, TGCheckButton *fCa[16], TGCheckButton *fC[15], TGCheckButton *fCsub[2], int id);

char* CorrectUnits(float CountRate);

void CalcRate(uint64_t &PrevRateTime);

CAEN_DGTZ_ErrorCode ForceClockSync(int handle);

CAEN_DGTZ_ErrorCode SwitchOffLogic(int handle, int N_CH);

CAEN_DGTZ_ErrorCode SetLogic(int handle, uint32_t reg_val[2][8], int N_CH);

CAEN_DGTZ_ErrorCode  SetInputRange(int handle, int channel, CAEN_DGTZ_InputRange_t InputRange);
 
CAEN_DGTZ_ErrorCode SetDPPParameters(int handle, DigitizerConfig_t Dcfg);

CAEN_DGTZ_ErrorCode ProgramDigitizer(int handle, DigitizerConfig_t Dcfg); 
 
CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms);

void ReturnErrorHandler(CAEN_DGTZ_ErrorCode ret, char* CName, int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms);
 
void SetDefaultConfiguration(DigitizerConfig_t *Dcfg); 

void ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg, ReadoutConfig_t *Rcfg); 

 
 
 