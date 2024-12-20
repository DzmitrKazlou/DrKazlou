#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <iterator>
#include <future>
//#include <../include/CAENDigitizerType.h>
//#include "CAENDigitizer.h"
//#include "CAENComm.h"
//#include "CAENVMElib.h"


//#include "MTCconfig.h"
#include "MTCFunc.h"
#include "MTCRoot.h"
#include "MTCFrame.h"
#include "MTCLogic.h"
#include "MTCParams.h"
#include "MTCOpt.h"


#include "TROOT.h"

//#include "ROOT/TFuture.hxx"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TROOT.h"
#include "TStyle.h"

#define CAEN_USE_DIGITIZERS



int N_CH;
uint32_t log_val[2][8] = {};
int handle = -1;


	DigitizerConfig_t   Dcfg;
	ReadoutConfig_t   Rcfg;
	CAEN_DGTZ_ErrorCode ret;
	Histograms_t Histo;


using namespace std;
	
	
	char *buffer = NULL;
	CAEN_DGTZ_DPP_PSD_Event_t   *Events[MAX_CH];  // events buffer
    CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms=NULL;         // waveforms buffer
	
	TCanvas *c1;
		
	Int_t ec_out = 0, ch_out = 0;
	uint32_t tst_out = 0;
	
	vector < vector <double>> v_out;


/*
void MainFrame::FillHisto(Int_t ch, Double_t &ampl, uint32_t timestamp){ 
	
		
				
		if (fSTCheck->GetState() == kButtonDown){
			ch_out = ch;
			tst_out = timestamp;
			v_out.push_back(vec);	
		}	
		vec.clear();
		
	
	if (fSTCheck->GetState() == kButtonDown){
		tree->Fill();
		ec_out++;
		v_out.clear();
	}
}
*/

/*
void MainFrame::DrawHisto( ){
	
	
	
	if (fRubik == true){
		c1->cd(cRubik);
		sprintf(str,"Ecanvas1_%i", cRubik);
		pad = (TPad*)c1->GetPrimitive(str);
		pad->SetGrid( );
		h_rubik->Draw("COLZ");
		h_rubik->GetXaxis()->SetLabelSize(0.08);
		h_rubik->GetYaxis()->SetLabelSize(0.08);
	}
	
	
	
	
	c1->Update( );
	
		
}

*/



// for far far future
/* 
auto gui = [ ]( ){
	new MainFrame(gClient->GetRoot(), 1800, 800);
	return 13;
};

auto daq = [ ]( ){
	
	ret = DataAcquisition(N_CH, &Histo);
};
*/


//---- Main program ------------------------------------------------------------

int main(int argc, char **argv)
{
	//ROOT::EnableThreadSafety( );		
	
   TApplication theApp("App", &argc, argv);

   if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return 1;
   }
   
   memset(&Dcfg, 0, sizeof(Dcfg));
   memset(&Rcfg, 0, sizeof(Rcfg));
   memset(&Histo, 0, sizeof(Histo));
	
	double layers_k[MAX_CH] = {0.007918776257675321,0.00814822204487009,0.006916635302973279,0.007615564718314238,0.007028262199785835,
							   0.006849098091830651,0.0071033763021084995,0.0069586701755050264,0.006939619364096369,0.006660565753945963,
							   0.007049831210679761,0.01355122958374811,0.012808851716326888,0.01264014031226357,0.012181420475259339, 0.0 }; // cosmic	
	double layers_b[MAX_CH] = {-0.2138339976300765,-0.21100236100026626,-0.12362866014301854,-0.1370449205395441,-0.25864511626128195,
							   0.0696751546660701,0.0032797769746628624,-0.031364045135273244,0.10704783437806786,0.11909165866299376, 
							   -0.07039630006137809,-0.29771365630645574,-0.2843372538769824,-0.16698160121053762,0.1722032611937825, 0.0 }; // cosmic	
	for (int i = 0; i<MAX_CH; i++){
		Histo.layers_k[i] = layers_k[i];
		Histo.layers_b[i] = layers_b[i];
	}
	//Configuration file routine
	
	FILE *f_ini = fopen(DEFAULT_CONFIG_FILE, "r");
	if (f_ini == NULL) {
		printf("Config file not found!\n");
		exit(0);
	}
	
	
	ParseConfigFile(f_ini, &Dcfg);
	fclose(f_ini);
	
	N_CH = Dcfg.Nch;
		
	printf("Config's abtained successful TraceLength[0] %i  Polarity[0] %i Threshold[0] %i \n", Dcfg.RecordLength[0], Dcfg.PulsePolarity[0], Dcfg.thr[0]);
	//Configuration file routine
	
	InitReadoutConfig(&Rcfg, N_CH);	
	
	
	
   //GUI;
   handle = 0;
   new MainFrame(gClient->GetRoot(), 1900, 900);
    
   // Create the task group and give work to it
   //auto t1  = async( gui);
   //auto t2  = async( daq);
   //t1.get();
   //t2.get();
   
   
   ret = DataAcquisition(N_CH, &Histo);
   

   theApp.Run();

   return 0;
}

