

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <iterator>

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
#include "ROOT/TFuture.hxx"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TROOT.h"
#include "TStyle.h"

#define CAEN_USE_DIGITIZERS



int N_CH = 2;
uint32_t log_val[2][8] = {};
int handle = -1;


	DigitizerConfig_t   Dcfg;
	ReadoutConfig_t   Rcfg;
	CAEN_DGTZ_ErrorCode ret;
	Histograms_t Histo;
	
	


using namespace std;

	// used for histograms in DrawHisto function
	//Color_t color[16] = {kBlue, kRed, kViolet, kGreen+1, kPink-9, kOrange, kMagenta, kCyan-7, kGray, kBlack, kBlue, kRed, kGreen, kOrange-2, kBlack, kOrange+2}; 			
	
		
		
	
	char *buffer = NULL;
	CAEN_DGTZ_DPP_PSD_Event_t   *Events[MAX_CH];  // events buffer
    CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms=NULL;         // waveforms buffer
	
	TCanvas *c1;
	
	
	
	//char * EventPtr = NULL;
	//uint32_t AllocatedSize, BufferSize, NumEvents[MAX_CH];
	
	//int DrawTime = 1000; //ms
	
			
	//uint64_t StartTime, CurrentTime, PrevRateTime, ElapsedTime, time1, time2;
    
		
	//TH1D *h_trace[MAX_CH];
	//TH1D *h_ampl[MAX_CH], *h_integral[MAX_CH];
	//TH1D *h_dt = new TH1D("hist_dt","hist dt", 400, -200, 200);
	
		
	//TH2D *h_psd_ampl = new TH2D("h_psd_ampl", "h_psd_ampl", 1000, 0, 20000, 1000, 0, 1);
	//TH2D *h_psd_int = new TH2D("h_psd_int", "h_psd_int", 1000, 0, 1000000, 1000, 0, 1);
	//TH2D *h_int_ampl = new TH2D("h_int_ampl", "h_int_ampl", 1000, 0, 20000, 1000, 0, 1000000);
	//TH2D *h_qs_ql = new TH2D("h_qs_ql", "h_qs_ql", 1000, 0, 500000, 1000, 0, 100000);
	
		
		
		
	Int_t CH_2D = 0;
	
	TFile *ff;		
	TTree * tree;		
	Int_t ec_out = 0, ch_out = 0;
	uint32_t tst_out = 0;
	
	vector < vector <double>> v_out;


/*
void MainFrame::FillHisto(Int_t ch, Double_t &ampl, uint32_t timestamp){ 
	
	bool fPSD_ampl  = fC[6]->GetState() == kButtonDown ? true : false; // psd vs ampl
	bool fPSD_int  = fC[7]->GetState() == kButtonDown ? true : false; // psd vs int
	bool fQsl  = fC[8]->GetState() == kButtonDown ? true : false; // qs vs ql
	
	Int_t BL_CUT = fNumericEntries[1]->GetNumber();
	bool BL_flag  = fC[0]->GetState() == kButtonDown ? true : false;
	CH_2D = fNumericEntries[3]->GetNumber();	
	
	Double_t BL_mean = 0,  integral = 0;
	ampl = 0;
	Int_t m_stamp;
	Double_t psd_val =0, Qs = 0, Ql = 0;
	
	//Int_t PSD_BIN = 2;
	Int_t p = Dcfg.PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive ? 1: -1; //POLARITY
	
	vector <Double_t> vec, vec_bl; 
	uint16_t *WaveLine;
	WaveLine = Waveforms->Trace1;
		
	h_trace[ch]->Reset("ICESM");	
			
		for (int j=0; j<(int)Waveforms->Ns; j++)
			vec_bl.push_back((double)WaveLine[j]);
	
		for ( int j=0; j<BL_CUT; j++)
			BL_mean = BL_mean + vec_bl[j];	
		BL_mean /= BL_CUT;	
		
		
		for ( int j=0; j<vec_bl.size( ); j++){
						
			vec.push_back(vec_bl[j] - BL_mean);
						
			if (vec[j] * p > ampl){
				ampl = vec[j] * p;
				m_stamp = j;
			}	
			
			if (j * b_width > LBound && j * b_width < RBound)
				integral += vec[j] * p;
		}
		
		if ( ( (fPSD_ampl == true) || (fPSD_int == true) || (fQsl == true)  )  && (ch == CH_2D) ){
			for (int j=m_stamp; j<vec.size( ); j++){
				if (j<(m_stamp + PSD_BIN) )
					Qs = Qs + p * vec[j];
				Ql = Ql + p * vec[j];
			}
			psd_val = 1 - ( Qs/Ql );
			
			if (fPSD_ampl == true) 
				h_psd_ampl->Fill(ampl, psd_val);
			if (fPSD_int == true) 
				h_psd_int->Fill(integral, psd_val);	
			if (fQsl == true) 
				h_qs_ql->Fill(Ql, Qs);	
		}
		
		h_integral[ch]->Fill(integral);
		h_ampl[ch]->Fill(ampl);
		if (ch == CH_2D)
			h_int_ampl->Fill(ampl, integral);
		
		if (fPrint == true)
			printf(" CH[%i]  Ampl %0.2f Time %d PSD %lg Qs %lg Ql %lg \n", ch, ampl, timestamp, psd_val, Qs, Ql);	
		
			if (BL_flag == true){
				for ( int j=0; j<vec.size( ); j++)
					h_trace[ch]->Fill(j * b_width, vec[j]);
			}
			else{
				for ( int j=0; j<vec_bl.size( ); j++)
					h_trace[ch]->Fill(j * b_width, vec_bl[j]);
			}	
				
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
	char str[20], h2Style[5] ="COLZ";
		
	if (WF_XMAX > Dcfg.RecordLength[0] * b_width)
		WF_XMAX = Dcfg.RecordLength[0];
	
	Int_t ChBoxFlag = 0, lCH = MAX_CH, cAmpl, cInt, cdT, cPSD_ampl = 0, cPSD_int = 0, cQsl = 0, cIA = 0, cRubik;
	for (int i = 1; i<10; i++)
		if (fC[i]->GetState() == kButtonDown) 
			//i < 6 ? ChBoxFlag++ : ChBoxFlag += 2;
			ChBoxFlag++;
		
	bool dCH[MAX_CH];
	for (int i = 0; i<N_CH; i++){
		dCH[i] = fCa[i]->GetState() == kButtonDown ? true : false;
		if (fCa[i]->GetState() == kButtonDown && i<lCH) 
			lCH = i;
	}
	
	bool fTrace  = fC[1]->GetState() == kButtonDown ? true : false;
	bool fAmpl  = fC[2]->GetState() == kButtonDown ? true : false;
	bool fInt  = fC[3]->GetState() == kButtonDown ? true : false;
	bool fdT  = fC[4]->GetState() == kButtonDown ? true : false;
	bool fRubik  = fC[5]->GetState() == kButtonDown ? true : false;
	bool fPSD_ampl  = fC[6]->GetState() == kButtonDown ? true : false;
	bool fPSD_int  = fC[7]->GetState() == kButtonDown ? true : false;
	bool fQsl  = fC[8]->GetState() == kButtonDown ? true : false;
	bool fIA  = fC[9]->GetState() == kButtonDown ? true : false;
	
	if (fTrace == true && ChBoxFlag >1)
		cAmpl = 2;
	if (fTrace == false && fAmpl == true)
		cAmpl = 1;
	
	if (fTrace == true && fAmpl == true && fInt ==true)
		cInt= 3;
	if (fTrace == true && fAmpl == false && fInt ==true)
		cInt= 2;
	if (fTrace == false && fAmpl == true && fInt ==true)
		cInt = 2;
	if (fTrace == false && fAmpl == false && fInt ==true)
		cInt = 1;
	
	if (ChBoxFlag ==1){
		cPSD_ampl = 1;
		cPSD_int = 1;
		cQsl = 1;
		cIA = 1;
		cdT = 1;
		cRubik = 1;
	}
	else{
		if (fQsl == true)
			cQsl = ChBoxFlag;
		if (fPSD_int == true)
			cPSD_int = ChBoxFlag - 1 * (fQsl == true ? 1 : 0);
		if (fPSD_ampl == true)
			cPSD_ampl = ChBoxFlag - 1 * (fQsl == true ? 1 : 0)  - 1 * (fPSD_int == true ? 1 : 0);
		if (fIA == true)
			cIA = ChBoxFlag - 1 * (fQsl == true ? 1 : 0)  - 1 * (fPSD_int == true ? 1 : 0) - 1 * (fPSD_ampl == true ? 1 : 0);
		cdT = ChBoxFlag;
		cRubik = ChBoxFlag;
	}
	
	c1->Clear();
	
	c1->SetGrid( );
		
	
	if (ChBoxFlag == 1)
		c1->Divide(1, 1, 0.001, 0.001);
	if (ChBoxFlag == 2)
		c1->Divide(2, 1, 0.001, 0.001);
	if (ChBoxFlag > 2 && ChBoxFlag < 5)
		c1->Divide(2, 2, 0.001, 0.001);
	if (ChBoxFlag > 4 && ChBoxFlag < 7)
		c1->Divide(3, 2, 0.001, 0.001);
	if (ChBoxFlag > 6)
		c1->Divide(3, 3, 0.001, 0.001);
	
	c1->Modified();
	
	if (fTrace == true){
		c1->cd(1);
		for (int i=lCH; i<N_CH; i++){ //(int)BoardInfo.Channels
			if (dCH[i] == true){
				h_trace[i]->SetLineColor(color[i]);
				if (i == lCH){
					h_trace[i]->Draw("HIST");
					h_trace[i]->GetYaxis()->SetRangeUser(WF_YMIN, WF_YMAX);
					h_trace[i]->GetYaxis()->SetTitleOffset(1.1);
					h_trace[i]->GetYaxis()->SetTitle(" Channels, lbs");
					h_trace[i]->GetXaxis()->SetRangeUser(WF_XMIN, WF_XMAX);
					h_trace[i]->GetXaxis()->SetTitle(" Time, ns");
				}	
				else	
					h_trace[i]->Draw("HIST SAME");
			}
		}
	}	
	
	if (fAmpl == true){
		c1->cd(cAmpl);
		for (int i=lCH; i<N_CH; i++){
			if (dCH[i] == true){
				h_ampl[i]->SetLineColor(color[i]);
			
				if (i	== lCH)
					h_ampl[i]->Draw("HIST");
				else
					h_ampl[i]->Draw("HIST SAME");	
			}
		}	
	}
		
	if (fInt == true){		
		c1->cd(cInt);
		for (int i=lCH; i<N_CH; i++){
			if (dCH[i] == true){
				h_integral[i]->SetLineColor(color[i]);
			
				if (i	== lCH)
					h_integral[i]->Draw("HIST");
				else
					h_integral[i]->Draw("HIST SAME");	
			}	
		}	
	}
	
	if (fdT == true){
		c1->cd(cdT);
		h_dt->SetLineColor(color[2]);
		h_dt->Draw("HIST");
		h_dt->GetXaxis()->SetTitle(" Time, ns");
	}			
	
	if (fIA == true){
		c1->cd(cIA);
		h_int_ampl->SetMarkerStyle(21);
		h_int_ampl->SetMarkerSize(0.4);
		h_int_ampl->SetMarkerColor(kBlue);
		h_int_ampl->Draw(h2Style);
	}	
	
	
	if (fPSD_ampl == true){
		c1->cd(cPSD_ampl);
		h_psd_ampl->SetMarkerStyle(21);
		h_psd_ampl->SetMarkerSize(0.4);
		h_psd_ampl->SetMarkerColor(kBlue);
		h_psd_ampl->Draw(h2Style);
	}		
	
	
	if (fRubik == true){
		c1->cd(cRubik);
		sprintf(str,"Ecanvas1_%i", cRubik);
		pad = (TPad*)c1->GetPrimitive(str);
		pad->SetGrid( );
		h_rubik->Draw("COLZ");
		h_rubik->GetXaxis()->SetLabelSize(0.08);
		h_rubik->GetYaxis()->SetLabelSize(0.08);
	}
	
	if (fPSD_int == true){
		c1->cd(cPSD_int);
		h_psd_int->SetMarkerStyle(21);
		h_psd_int->SetMarkerSize(0.4);
		h_psd_int->SetMarkerColor(kBlue);
		h_psd_int->Draw(h2Style);
	}		
	
	if (fQsl == true){
		c1->cd(cQsl);
		h_qs_ql->SetMarkerStyle(21);
		h_qs_ql->SetMarkerSize(0.4);
		h_qs_ql->SetMarkerColor(kBlue);
		h_qs_ql->Draw(h2Style);
	}		
	
	if (fIA == true){
		c1->cd(cIA);
		h_int_ampl->SetMarkerStyle(21);
		h_int_ampl->SetMarkerSize(0.4);
		h_int_ampl->SetMarkerColor(kBlue);
		h_int_ampl->Draw(h2Style);
	}	
	
	
	c1->Update( );
	
		
}

*/
/*

void MainFrame::ReadoutLoop( ){	
		
	PrevRateTime = get_time();
	Double_t ampl[N_CH];
	const char* xlabel[5] = {"5", "6", "7", "8", "9"};
	const char* ylabel[5] = {"0", "1", "2", "3", "4"};
	
	for (Int_t i=0; i<5; i++)
		h_rubik->Fill(xlabel[i], ylabel[i], 0);
	
	while(loop == 1) {
		
			   
	  // Calculate throughput and trigger rate (every second) 			
			
        	
        	CurrentTime = get_time();
        	ElapsedTime = CurrentTime - PrevRateTime;

        	
        	//if (ElapsedTime > (fNumericEntries[3]->GetNumber()*1000)) { // 1000
			if (ElapsedTime > 1000) { // 1000
				sprintf(CName,"T: %li s",  (CurrentTime - StartTime) / 1000 );
				fTLabel->SetText(CName);
				gSystem->ProcessEvents(); 
            	if (Nb != 0){
					sprintf(CName,"Read. %.2f MB/s ", (float)Nb/((float)ElapsedTime*1048.576f) );
					fStatusBar->SetText(CName, 0);
					
					for (Int_t ch=0; ch<N_CH; ch++) { //8
						if (TrgCnt[ch] != 0){
							sprintf(CName, "CH[%i]: %.2f Hz ", ch, (float)TrgCnt[ch]*1000.0f/(float)ElapsedTime);
							if (ch < 15)
								fStatusBar->SetText(CName, ch+1);
						}
						else{
							sprintf(CName, "No data...");
							if (ch < 15)
								fStatusBar->SetText(CName, ch+1);
						}
						TrgCnt[ch] = 0;
					}
					//printf("No data...\n");
                	//if (ret == CAEN_DGTZ_Timeout) printf ("Timeout...\n"); else printf("No data...\n");
				}
									
            	else{
						//printf("No data...\n");
						sprintf(CName, "No data...");
						fStatusBar->SetText(CName, 0);
				}
								
            Nb = 0;
            		
            PrevRateTime = CurrentTime;
			gSystem->ProcessEvents(); 
        	}
		// Calculate throughput and trigger rate (every second) 
	   
		ret = CAEN_DGTZ_ReadData(handle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize); // Read the buffer from the digitizer 
		if (ret) {
			printf("ERR_READ_DATA \n");
			sprintf(CName, "ERR_READ_DATA");
			new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
			ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
        }
		
		//printf("BufferSize: %d\n", BufferSize);	
        
		
        if (BufferSize == 0) 
			continue;
		
		
		
		Nb += BufferSize;
		
		ret = CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)&Events, NumEvents);
        if (ret) {
            sprintf(CName, "GET_DPPEVENTS");
			new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
            ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
        }
        
		if ( fPrint == true)
			printf(" ---------------------------------------- \n");			
		
		int Nev = 0;
			
		for (Int_t ch=0; ch<N_CH; ch++) { 
			//ampl[ch] = 0;
			h_trace[ch]->Reset("ICESM");
			if (Dcfg.ChannelMask & (1<<ch) ){
				Nev +=(int)NumEvents[ch];
				for (uint32_t ev=0; ev<(int)NumEvents[ch]; ev++) {
				
					CAEN_DGTZ_DecodeDPPWaveforms(handle, (void**)&Events[ch][ev], Waveforms);
					TrgCnt[ch]++;
					
					Double_t a_val;
					FillHisto(ch, a_val, Events[ch][ev].TimeTag); // all data performance
					//printf(" FillHisto CH[%i] Ev[%i] Nev %i \n", ch, ev, Nev );			
				   
					//if (ev == 0){
					//	ampl[ch] = a_val;
					//	
					//}	
					gSystem->ProcessEvents(); 
				} // events loop
	


	
			} // check enabled channels
			
		}// channels loop
		
		bool fdT  = fC[4]->GetState() == kButtonDown ? true : false;
		bool fRubik  = fC[5]->GetState() == kButtonDown ? true : false;
		
		if (fdT == true){
			double dt = (Events[1][0].TimeTag > Events[0][0].TimeTag) ? (double)(Events[1][0].TimeTag - Events[0][0].TimeTag) : (double)(Events[0][0].TimeTag - Events[1][0].TimeTag);
			printf(" [1] %d [0] %d dt %f ns  \n", Events[1][0].TimeTag, Events[0][0].TimeTag, dt/1000);
			h_dt->Fill(dt/1000);
		}
		
				
		if (fRubik == true && Nev!=0){
			h_rubik->Reset("ICESM" );
			
			for (Int_t ch=0; ch<N_CH; ch++){
				if (ampl[ch] > 0 && ch < 5) {
					for (Int_t i=0; i<5; i++)
						h_rubik->Fill(xlabel[i], ylabel[ch], 1);
					
				}
				if (ampl[ch] > 0 && ch >= 5) {
					for (Int_t i=0; i<5; i++)
						h_rubik->Fill(xlabel[ch-5], ylabel[i], 1);
				}
			}
			
		}	
		
		if (ElapsedTime>=(fNumericEntries[2]->GetNumber()*1000) && Nev!=0)//&& i==0 ) // sec*1000 = ms // DrawTime = fNumericEntries[6]->GetNumber()
			DrawHisto( );	
		
		if ( fPrint == true)
			printf(" ---------------------------------------- \n");			
		
		
		gSystem->ProcessEvents(); 
		
    } // end of readout loop		
				
	
}

*/
void gui(){
	
	new MainFrame(gClient->GetRoot(), 1800, 800);
}

void daq(){
	
	ret = DataAcquisition(N_CH, &Histo);
}

//---- Main program ------------------------------------------------------------

int main(int argc, char **argv)
{
	ROOT::EnableImplicitMT(2);		
   TApplication theApp("App", &argc, argv);

   if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return 1;
   }
   
   memset(&Dcfg, 0, sizeof(Dcfg));
   memset(&Rcfg, 0, sizeof(Rcfg));
   memset(&Histo, 0, sizeof(Histo));
	
	//Configuration file routine
	
	FILE *f_ini = fopen(DEFAULT_CONFIG_FILE, "r");
	if (f_ini == NULL) {
		printf("Config file not found!\n");
		exit(0);
	}
	
	
	ParseConfigFile(f_ini, &Dcfg);
	fclose(f_ini);
	
		
	printf("Config's abtained successful TraceLength[0] %i  Polarity[0] %i Threshold[0] %i \n", Dcfg.RecordLength[0], Dcfg.PulsePolarity[0], Dcfg.thr[0]);
	//Configuration file routine
	
	InitReadoutConfig(&Rcfg, N_CH);	
		
   //GUI;
   handle = 0;
   //new MainFrame(gClient->GetRoot(), 1800, 800);
    
   // Create the task group and give work to it
   ROOT::Experimental::TTaskGroup tg;
   tg.Run(gui);
   tg.Run(daq);
      
   // Wait until all items are complete
   tg.Wait();
   
   //ret = DataAcquisition(N_CH, &Histo);
   

   theApp.Run();

   return 0;
}

