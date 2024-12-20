#include "TColor.h"
#include "MTCFunc.h"

#include "TROOT.h"

#include <TSystem.h>
#include "TStyle.h"

#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"



void InitHisto(Histograms_t *Histo, int RecordLength[MAX_CH], int N_CH, int b, int handle);

void DrawTH2D(bool flag, TCanvas *can, TH2D *h, int cPos, char *opt);

void DrawHistoCh(Histograms_t *Histo, int N_CH, TCanvas *can);
void DrawHistoT(Histograms_t Histo, int N_CH, TCanvas *can);
	
void InitReadoutConfig(ReadoutConfig_t *Rcfg, int N_CH); 

void FillTraces(int b, int ch, TH1D *hist, bool fBL);

void FillHisto(int b, int ch, uint32_t ev, Histograms_t Histo, double &ampl, int BL_CUT);
//void FillHisto(int b, int ch, uint32_t ev, Histograms_t *Histo, double &ampl, int BL_CUT);
//void FillHisto(int b, int ch, uint32_t ev, Histograms_t &Histo, double &ampl);
//void FillHisto(int ch, uint32_t ev, Histograms_t *Histo, double &ampl);

void ReadoutLoop(Histograms_t Histo[2]);
//void ReadoutLoop(int handle, int b, Histograms_t *Histo);

//CAEN_DGTZ_ErrorCode DataAcquisition(Histograms_t Histo[2]);
//CAEN_DGTZ_ErrorCode DataAcquisition(Histograms_t *Histo, Histograms_t *Histo_b);
CAEN_DGTZ_ErrorCode DataAcquisition( );

void* daq_thread(void* ptr);