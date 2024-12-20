#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <iterator>
//#include <pthread.h>
#include <X11/Xlib.h>

//#include "MTCFunc.h"
#include "MTCReadout.h"
#include "MTCRoot.h"
#include "MTCFrame.h"
#include "MTCLogic.h"
#include "MTCParams.h"
#include "MTCOpt.h"


#include "TROOT.h"
#include "TThread.h"
//#include "ROOT/TFuture.hxx"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"



int N_CH;
uint32_t log_val[2][2][8] = {};
int handle[2] = {-1, -1};


	DigitizerConfig_t   Dcfg[2];
	ReadoutConfig_t   Rcfg;
	CAEN_DGTZ_ErrorCode ret;
	Histograms_t Histo[2];


//using namespace std;
	
	//TCanvas *can;

	char *buffer[2] = {NULL};
	CAEN_DGTZ_DPP_PSD_Event_t   *Events[2][MAX_CH];  // events buffer
    CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms[2] = {NULL};         // waveforms buffer


//  the future is nearby

/*
void* gui_thread(void* ptr){
	
	ReadoutConfig_t  *Cfg;
	Cfg = (ReadoutConfig_t*)ptr;
	
	printf("gui thr: %i x %i\n", Cfg->WindX, Cfg->WindY);

	TThread::Lock( );
	MainFrame  *mf = new MainFrame(gClient->GetRoot(), Cfg->WindX, Cfg->WindY);
	TThread::UnLock( );

	char CName[20];
	bool flag = false; 
	uint64_t start_time = get_time( ), PrevDrawTime = get_time(), ElapsedTime;
	


	while(1) {
		
		
		if ( (get_time( ) - start_time) % 1000 == 0){
			if (ElapsedTime < (get_time( ) - start_time) / 1000)
				flag = false;
			ElapsedTime = (get_time( ) - start_time) / 1000;
			if (!flag){
				//printf("GUI T: %li \n", (get_time( ) - start_time)/1000);
				flag = true;
			}
			gSystem->ProcessEvents(); 
			//PrevPrintTime = get_time( );
		}
		

		
		if (Rcfg.loop == 1)
			CalcRate(start_time);
			
	}

	
	return 0;
}
*/

//---- Main program ------------------------------------------------------------

int main(int argc, char **argv)
{
	//ROOT::EnableThreadSafety( );		
	
   TApplication theApp("App", &argc, argv);

	gStyle->SetOptFit(0);
	gStyle->SetOptStat(0);

   if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return 1;
   }
   
   //memset(&Dcfg, 0, sizeof(Dcfg));
	memset(Dcfg, 0, (size_t)2 * sizeof(DigitizerConfig_t) );
	memset(&Rcfg, 0, sizeof(Rcfg));

	memset(&Histo[0], 0, sizeof(Histo[0]));
	memset(&Histo[1], 0, sizeof(Histo[1]));	
	
	double layers_coeff[MAX_CH] = {1.0, 1.0, 0.9, 1.0, 1.06, 1.16, 1.02, 0.94, 0.16, 0.84, 0.96, 1.16, 0.14, 0.16, 0.14, 0.11 }; // cosmic	
	for (int i = 0; i<MAX_CH; i++)
		Histo[0].layers_coeff[i] = layers_coeff[i];
	
	//Configuration file routine
	
	FILE *f_ini = fopen(DEFAULT_CONFIG_FILE, "r");
	if (f_ini == NULL) {
		printf("Config file not found!\n");
		exit(0);
	}
	
	
	ParseConfigFile(f_ini, &Dcfg[0], &Rcfg);
	Dcfg[1] = Dcfg[0];
	//ParseConfigFile(f_ini, &Dcfg[1], &Rcfg);
	fclose(f_ini);
	
	//N_CH = Dcfg[0].Nch;
		
	//printf("Config's abtained successful TraceLength[0] %i TraceLength[5] %i  Polarity[0] %i Threshold[0] %i \n", Dcfg.RecordLength[0], Dcfg.RecordLength[5], Dcfg.PulsePolarity[0], Dcfg.thr[0]);
	printf("Config's obtained successfully\n");
	//Configuration file routine
	
	InitReadoutConfig(&Rcfg, Dcfg[0].Nch);	
	
		
	//Thanks to Simon; 
	//Display* d = XOpenDisplay(NULL);
	//Screen*  s = DefaultScreenOfDisplay(d);

	handle[0] = 0;
	handle[1] = 0;
		
	//InitHisto(&Histo[0], Dcfg.RecordLength, N_CH, 0, handle[0]);	
	//InitHisto(&Histo[1], Dcfg.RecordLength, N_CH, 1, handle[1]);	
	//Histo.handle = handle;

	
	Rcfg.WindX = 1900;//Rcfg.WindX = s->width;
	Rcfg.WindY = 900;//Rcfg.WindY = s->height;
	
	new MainFrame(gClient->GetRoot(), Rcfg.WindX, Rcfg.WindY);

	//ret = DataAcquisition(handle); //, &Histo);

	printf("Finished \n");
	
	//new MainFrame(gClient->GetRoot(), s->width, s->height);
	
      
   
   
   //ret = DataAcquisition(handle); //, &Histo);
   	

	theApp.Run( );

   return 0;
}

