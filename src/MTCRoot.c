

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <iterator>

#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
#include "CAENComm.h"
#include "CAENVMElib.h"


#include "MTCconfig.h"
#include "MTCFunc.h"
#include "MTCRoot.h"

#include "TROOT.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TROOT.h"
#include "TStyle.h"

#define CAEN_USE_DIGITIZERS

// asdlkfadfh

using namespace std;


	Color_t color[16] = {kBlue, kRed, kViolet, kGreen+1, kPink-9, kOrange, kMagenta, kCyan-7, kGray, kBlack, kBlue, kRed, kGreen, kOrange-2, kBlack, kOrange+2}; 			
	
	char CName[300];
	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";
	
	
	
	Double_t WF_XMIN, WF_XMAX, WF_YMIN, WF_YMAX;
	Int_t PSD_BIN = 2;
	Double_t LBound, RBound;
	
	DigitizerConfig_t   Dcfg;
	CAEN_DGTZ_DPP_PSD_Params_t DPPParams;	
	
	FILE *f_ini;
	  
	CAEN_DGTZ_ErrorCode ret;
	CAENComm_ErrorCode com_err;
	
	bool fInit = false;
	bool fPrint = true; 
	int handle = -1;
	int loop = 0; // Main readout loop flag
	//const int MAX_CH = 16;
	Int_t N_CH;
	uint32_t log_val[2][8] = {0};
	
	
    CAEN_DGTZ_BoardInfo_t BoardInfo;
	CAEN_DGTZ_EventInfo_t EventInfo;
	CAEN_DGTZ_UINT16_EVENT_t *Event16 = NULL;
	char *buffer = NULL;
	
	CAEN_DGTZ_DPP_PSD_Event_t   *Events[MAX_CH];  // events buffer
    CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms=NULL;         // waveforms buffer
	
	
	int MajorNumber;
	uint32_t Nb=0;
	int TrgCnt[MAX_CH];
	
	char * EventPtr = NULL;
	uint32_t AllocatedSize, BufferSize, NumEvents[MAX_CH];
	
	//int DrawTime = 1000; //ms
	
	Int_t b_width = 2; // 2 ns bin width for 500MS/s V1730S
	//Int_t p = -1; //NEGATIVE POLARITY
		
	uint64_t StartTime, CurrentTime, PrevRateTime, ElapsedTime, time1, time2;
    
		
	TH1D *h_trace[MAX_CH];
	TH1D *h_ampl[MAX_CH], *h_integral[MAX_CH];
	TH1D *h_dt = new TH1D("hist_dt","hist dt", 400, -200, 200);
	//TH1D *h_dt_math = new TH1D("hist_dt_math","hist dt math", 400, -200, 200);
	
	//TH2D *h_psd_ampl[2], *h_psd_int[2], *h_int_ampl[2], *h_qs_ql[2];
		
	TH2D *h_psd_ampl = new TH2D("h_psd_ampl", "h_psd_ampl", 1000, 0, 20000, 1000, 0, 1);
	TH2D *h_psd_int = new TH2D("h_psd_int", "h_psd_int", 1000, 0, 1000000, 1000, 0, 1);
	TH2D *h_int_ampl = new TH2D("h_int_ampl", "h_int_ampl", 1000, 0, 20000, 1000, 0, 1000000);
	TH2D *h_qs_ql = new TH2D("h_qs_ql", "h_qs_ql", 1000, 0, 500000, 1000, 0, 100000);
	
		
		
	TH2I *h_rubik = new TH2I("h_rubik", "h_rubik", 5, 5, 10, 5, 0, 5);
	
	Int_t CH_2D = 0;
	
	TFile *ff;		
	TTree * tree;		
	Int_t ec_out = 0, ch_out = 0;
	uint32_t tst_out = 0;
	
	vector < vector <double>> v_out;


static long get_time()
{
    long time_ms;
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;

    return time_ms;
}

int DeltaT(TH1D *hist[2]){
	Double_t  min_val[2], min_bin[2];	

	for (int i =0; i<2; i++){
		min_val[i] = 5000;
		min_bin[i] = 0;
	
		for (int bin = 1; bin<hist[i]->GetNbinsX(); bin++){
			if (hist[i]->GetBinContent(bin)<min_val[i]){
				min_bin[i] = bin;
				min_val[i] = hist[i]->GetBinContent(bin);
			}	
			
		}
	}
	
	
	
	return (min_bin[1] - min_bin[0]) * b_width;
}

int DeltaTMath(TH1D *hist[2], Int_t TH1, Int_t TH2){
	
	Int_t p = -1; //POLARITY
	Double_t  pre_val[2] = {0.0, 0.0}, thr_val[2] = {0.0, 0.0}, t_stamp[2] = {0.0, 0.0};	
	Int_t thr_bin[2], thr[2] = {TH1, TH2};
	
	Double_t k, b;
	
	printf(" ---------------------------- \n");
	
	for (int i =0; i<2; i++){
				
		int bin =1;
		while ( bin<hist[i]->GetNbinsX() &&  (p * hist[i]->GetBinContent(bin) ) < thr[i]){
			
			//printf(" thr[%i]  bin %i Ampl %f \n", i, bin, hist[i]->GetBinContent(bin));	
			pre_val[i] = p * hist[i]->GetBinContent(bin-1);
			thr_val[i] = p * hist[i]->GetBinContent(bin);
			thr_bin[i]	= bin;
			bin++;
		}
		//printf(" thr[%i]  bin %i Ampl %0.2f \n", i, bin, p * hist[i]->GetBinContent(bin));	
		
		k = ( thr_val[i] - pre_val[i] ) / b_width;
		b = thr_val[i] - k * thr_bin[i] * b_width; 
		t_stamp[i] = ( thr[i] - b ) / k;
		//printf(" t_stamp[%i], %0.2f \n", i, t_stamp[i]);
	}
	//printf(" dt %0.2f \n", t_stamp[1] - t_stamp[0] );
	
	
	return (t_stamp[1] - t_stamp[0]);
}

void MainFrame::SaveTraces( ){
	
	printf("save traces \n");
	
}	

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


MainFrame::MainFrame(const TGWindow *p, UInt_t w, UInt_t h)
{
	
   fMain = new TGMainFrame(p, w, h);

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   fMain->Connect("CloseWindow()", "MainFrame", this, "CloseWindow()");

   // Create menubar and popup menus. The hint objects are used to place
   // and group the different menu widgets with respect to eachother.
   fMenuDock = new TGDockableFrame(fMain);
   fMain->AddFrame(fMenuDock, new TGLayoutHints(kLHintsExpandX, 0, 0, 1, 0));
   fMenuDock->SetWindowName("MTCRoot Menu");

   fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
   fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
   fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

   fMenuFile = new TGPopupMenu(gClient->GetRoot());
   fMenuFile->AddEntry("L&oad config", M_LOAD_CONFIG);
   fMenuFile->AddEntry("Save &config", M_SAVE_CONFIG);
   fMenuFile->AddEntry("Save &histo", M_FILE_SAVE_HISTO);
   fMenuFile->AddEntry("Sa&ve traces", M_FILE_SAVE_TRACES);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("E&xit", M_FILE_EXIT);
	
	fMenuFile->DisableEntry(M_FILE_SAVE_TRACES);
   
   fMenuOpt = new TGPopupMenu(gClient->GetRoot());
   fMenuOpt->AddEntry("&Hist options", M_OPT_MENU);
   fMenuOpt->AddEntry("&Digi parameters", M_PARAMS_MENU);
   fMenuOpt->AddEntry("&Logic scheme", M_LOGIC_MENU);


   fMenuHelp = new TGPopupMenu(gClient->GetRoot());
   fMenuHelp->AddEntry("&Manual", M_MANUAL);
   fMenuHelp->AddEntry("&About", M_HELP_ABOUT);

   // Menu button messages are handled by the main frame (i.e. "this")
   // HandleMenu() method.
	fMenuFile->Connect("Activated(Int_t)", "MainFrame", this, "HandleMenu(Int_t)");
	fMenuOpt->Connect("Activated(Int_t)", "MainFrame", this, "HandleMenu(Int_t)");
	fMenuHelp->Connect("Activated(Int_t)", "MainFrame", this, "HandleMenu(Int_t)");

   fMenuBar = new TGMenuBar(fMenuDock, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Options", fMenuOpt, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

   fMenuDock->AddFrame(fMenuBar, fMenuBarLayout);


	TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fMain,200,40);
	TGVerticalFrame *vframe1 = new TGVerticalFrame(hframe1,200,40);		
    hframe1->AddFrame(vframe1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	TGGroupFrame *gframe_store = new TGGroupFrame(hframe1, " - ", kVerticalFrame);
	gframe_store->SetTitlePos(TGGroupFrame::kRight); // right aligned
	vframe1->AddFrame(gframe_store, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	fSTCheck  = new TGCheckButton(gframe_store, new TGHotString("STORE TRACES"), 40);	
	fSTCheck->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
	gframe_store->AddFrame(fSTCheck, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	
	fFStore = new TGHorizontalFrame(gframe_store, 200, 40);
	gframe_store->AddFrame(fFStore, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fSTLabel = new TGLabel(fFStore, "File name");
	fFStore->AddFrame(fSTLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	
	fSTTextEntry = new TGTextEntry(fFStore, fSTTextBuffer = new TGTextBuffer(0)) ;
	fSTTextBuffer->AddText(0, "output.root");
	fSTTextEntry->SetEnabled(0);
	fSTTextEntry->Resize(80, fSTTextEntry->GetDefaultHeight());
	fFStore->AddFrame(fSTTextEntry, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	
	gframe_store->Resize();
	
	TGGroupFrame *gframe_opt = new TGGroupFrame(hframe1, "Options", kVerticalFrame);
	gframe_opt->SetTitlePos(TGGroupFrame::kRight); // right aligned
	vframe1->AddFrame(gframe_opt, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//

   // 2 column, n rows
   gframe_opt->SetLayoutManager(new TGMatrixLayout(gframe_opt, 0, 1, 10));

const char *numlabel[] = {
   "N_CH",
   "BLine_cut",
   "DrawTime",
   "CH_2D"
    };	
	
const Double_t numinit[] = {
   10, 10, 0.5, 0
};	

int iStyle[]	= {0, 0, 2, 0}; 	
	
   for (int i = 0; i < 4; i++) {
      fF[i] = new TGHorizontalFrame(gframe_opt, 200, 30);
      gframe_opt->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
      fNumericEntries[i] = new TGNumberEntry(fF[i], numinit[i], 8, i + 20, (TGNumberFormat::EStyle) iStyle[i]); //numinit[i], 7, i + 20, (TGNumberFormat::EStyle) iStyle[i]
	  fNumericEntries[i]->Connect("ValueSet(Long_t)", "MainFrame", this, "DoSetVal()");
      fF[i]->AddFrame(fNumericEntries[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
      fLabel[i] = new TGLabel(fF[i], numlabel[i]);
      fF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   }
   
   N_CH = fNumericEntries[0]->GetNumber();
   CH_2D = fNumericEntries[3]->GetNumber();
   
   gframe_opt->Resize();
   
   TGGroupFrame *gframe_ch = new TGGroupFrame(hframe1, "Channels", kVerticalFrame);
	gframe_ch->SetTitlePos(TGGroupFrame::kRight); 
	vframe1->AddFrame(gframe_ch, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
   
   	Pixel_t p_color[16];
	gClient->GetColorByName("blue", p_color[0]);
	gClient->GetColorByName("red", p_color[1]);
	gClient->GetColorByName("violet", p_color[2]);
	//gClient->GetColorByName("teal", p_color[3]); // teal
	p_color[3] = 0x8000; // green; 
	gClient->GetColorByName("pink", p_color[4]);
	gClient->GetColorByName("orange", p_color[5]);
	gClient->GetColorByName("magenta", p_color[6]);
	p_color[7] = 0x7DF9FF; // electric blue
	//gClient->GetColorByName("green", p_color[7]);
	gClient->GetColorByName("gray", p_color[8]);
	gClient->GetColorByName("black", p_color[9]);
	gClient->GetColorByName("blue", p_color[10]);
	gClient->GetColorByName("red", p_color[11]);
	gClient->GetColorByName("green", p_color[12]);
	//gClient->GetColorByName("yellow", p_color[13]);
	p_color[13] = 0xFFD700; // gold
	gClient->GetColorByName("black", p_color[14]);
	gClient->GetColorByName("brown", p_color[15]);
	
	char cb_label[5];	

	for (int i = 0; i < MAX_CH; ++i) {
		sprintf(cb_label, "CH%i", i);
		fCa[i] = new TGCheckButton(gframe_ch, cb_label,	i);
		fCa[i]->SetTextColor(p_color[i]); //p_color[i]
		//printf("Color [%i] : %li \n", i, p_color[i]);
		fCa[i]->SetState(kButtonDisabled); 
		fCa[i]->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
		gframe_ch->AddFrame(fCa[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 5, 0));
	}
		
		//fCa[4]->SetState(kButtonDisabled); //Coincidence Initialy Disabled
 
	gframe_ch->SetLayoutManager(new TGMatrixLayout(gframe_ch, 0, 2, 3));
	gframe_ch->Resize(); 
   
	TGGroupFrame *gframe_hist = new TGGroupFrame(hframe1, "Hist", kVerticalFrame);
	gframe_hist->SetTitlePos(TGGroupFrame::kRight); 
	vframe1->AddFrame(gframe_hist, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//

	fC[0] = new TGCheckButton(gframe_hist, new TGHotString("BL_CUT"), 41);	
	fC[1] = new TGCheckButton(gframe_hist, new TGHotString("TRACES"), 42);	
	fC[2] = new TGCheckButton(gframe_hist, new TGHotString("AMPL_HIST"), 43);	
	fC[3] = new TGCheckButton(gframe_hist, new TGHotString("INTEGRAL"), 44);	
	fC[4] = new TGCheckButton(gframe_hist, new TGHotString("dT"), 45);	
	fC[5] = new TGCheckButton(gframe_hist, new TGHotString("Rubik"), 46);	
	fC[6] = new TGCheckButton(gframe_hist, new TGHotString("PSD_ampl"), 47);	
	fC[7] = new TGCheckButton(gframe_hist, new TGHotString("PSD_int"), 48);	
	fC[8] = new TGCheckButton(gframe_hist, new TGHotString("Qsl"), 49);	
	fC[9] = new TGCheckButton(gframe_hist, new TGHotString("Int vs Ampl"), 50);	
	
	fC[0]->SetState(kButtonDown); //BL_CUT ON
	fC[1]->SetState(kButtonDown); //TRACES ON
	
	
	for (int i = 0; i < 10; i++) {
		fC[i]->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
		gframe_hist->AddFrame(fC[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	}	
	
	//gframe_hist->SetLayoutManager(new TGMatrixLayout(gframe_hist, 0, 2, 5));
	gframe_hist->Resize();
	 
    fInitButton = new TGTextButton(vframe1, " In&it ", 1);
    fInitButton->SetFont(sFont); 
    fInitButton->Resize(60, 30);
    fInitButton->Connect("Clicked()","MainFrame",this,"InitButton()");
    vframe1->AddFrame( fInitButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));	
	
   fClearButton = new TGTextButton(vframe1, " Cle&ar ", 1);
   fClearButton->SetFont(sFont);
   fClearButton->Resize(60, 30);
   fClearButton->SetState (kButtonDisabled);
   fClearButton->Connect("Clicked()","MainFrame",this,"ClearHisto()");
   vframe1->AddFrame(fClearButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));	
	
	fTLabel = new TGLabel(vframe1, "Timer 000 000 000");
	fTLabel->SetTextFont(sFont);
	fTLabel->Resize(200, 30);
	
	vframe1->AddFrame(fTLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   	
	
	vframe1->Resize();
	
	fEcanvas1 = new TRootEmbeddedCanvas("Ecanvas1", hframe1, 1400, 800);
	hframe1->AddFrame(fEcanvas1, new TGLayoutHints(kLHintsCenterX, 10,5,25,0));//kLHintsExpandX |   kLHintsExpandY
	hframe1->Resize();

	
   fMain->AddFrame(hframe1, new TGLayoutHints(kLHintsCenterX, 2, 2 , 2, 2) );
	
		
	// status bar
	//Int_t parts[] = {13, 13, 13, 22, 39};
	Int_t parts[] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 2};
	fStatusBar = new TGStatusBar(fMain, 100, 20, kHorizontalFrame); //kHorizontalFrame //kSunkenFrame
	fStatusBar->SetParts(parts, 15);
	fMain->AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 0, 0, 2, 0));
	
	
	TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fMain, 200, 40);
 	fStartButton = new TGTextButton(hframe2," Sta&rt ", 1);
	fStartButton->SetFont(sFont);
    fStartButton->Resize(60, 30);
	fStartButton->SetState (kButtonDisabled);
  	fStartButton->Connect("Clicked()","MainFrame", this, "StartButton()");
  	hframe2->AddFrame(fStartButton, new TGLayoutHints(kLHintsCenterY |  kLHintsExpandX, 4, 4, 4, 4));

	fStopButton = new TGTextButton(hframe2,"  S&top  ", 1);
    fStopButton->SetFont(sFont);
    fStopButton->Resize(60, 30);
	fStopButton->SetState (kButtonDisabled);
	fStopButton->Connect("Clicked()","MainFrame",this,"StopButton()");	
	
    hframe2->AddFrame(fStopButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));
	hframe2->Resize();
	fMain->AddFrame(hframe2, new TGLayoutHints(kLHintsCenterX,       2, 2, 20, 2));
      
   
   fMain->SetWindowName("MTCRoot");
   fMain->SetIconName("MTCRoot");
   fMain->SetIconPixmap("/home/valera/Programs/CAEN/MTCRoot/rose512.jpg");
   fMain->MapSubwindows( );

	fMain->Resize( );
	fMain->MapWindow( );
   //fMain->Print();
   Connect("Created()", "MainFrame", this, "Welcome()");
   Created( );
}

MainFrame::~MainFrame()
{
   // Delete all created widgets.

   delete fMenuFile;

   delete fMain;
}

void MainFrame::CloseWindow()
{
   // Got close message for this MainFrame. Terminates the application.
	ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
		
	gApplication->Terminate();
}

void MainFrame::DoCheckBox(){
	
	TGButton *btn = (TGButton *) gTQSender;
	Int_t id = btn->WidgetId();
	
	//Store traces checkbox
	if (id == 40 ) {
	   if (fSTCheck->GetState() == kButtonDown)
            fSTTextEntry->SetEnabled(1);
		else
			fSTTextEntry->SetEnabled(0);
	}
   
  	// To avoid simultaneous PSD and dT 
	if (id >= 45 && id <= 46) {
		for (int i = 4; i < 6; i++)
			if (fC[i]->WidgetId() != id)
				fC[i]->SetState(kButtonUp);
	}	
			
	
}

void MainFrame::DoSetVal(){
	
	N_CH = fNumericEntries[0]->GetNumber();

	
}


void MainFrame::InitButton()
{
	/*
	memset(&Dcfg, 0, sizeof(Dcfg));
	
	//Configuration file routine
	f_ini = fopen(DEFAULT_CONFIG_FILE, "r");
	if (f_ini == NULL) {
		printf("Config file not found!\n");
		ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
	}
	ParseConfigFile(f_ini, &Dcfg, &DPPParams);
	fclose(f_ini);
	
	printf("Config's abtained successful TraceLength %i  Polarity %i \n", Dcfg.RecordLength, Dcfg.PulsePolarity);
	//Configuration file routine
	*/
	
	WF_XMIN = 0, WF_XMAX = Dcfg.RecordLength[0] * b_width;
	WF_YMIN = -500, WF_YMAX = 1000;
	
	LBound = 0, RBound = Dcfg.RecordLength[0] * b_width;
	
	ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB_A4818_V2718, Dcfg.PID, 0, 0, &handle); //15103
	
	
    if(ret != CAEN_DGTZ_Success) {
		sprintf(CName, "Can't open digitizer \n ret = %i", ret);
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
		ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
	}

	//GetInfo 
	ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
	sprintf(CName, "Connected to CAEN Digitizer Model %s \n ROC FPGA Release is %s\n AMC FPGA Release is %s\n", BoardInfo.ModelName, BoardInfo.ROC_FirmwareRel, BoardInfo.AMC_FirmwareRel);
	new TGMsgBox(gClient->GetRoot(), fMain, "Info", CName, kMBIconAsterisk, kMBOk);
	
		
	//Hist initialisation	
	for (int ch = 0; ch < N_CH; ch++){ //
		sprintf(CName, "h_ampl%i", ch);
		h_ampl[ch]= new TH1D(CName, CName, 1000, 0, 32000);
		sprintf(CName, "h_integral%i", ch);
		h_integral[ch]= new TH1D(CName, CName, 100000, 0, 100000);
	}
		
	ret = ProgramDigitizer(handle, Dcfg, DPPParams);
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "ERR_PROGRAM_DIGITIZER_FAILURE \n", kMBIconStop, kMBOk);
        ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
    }	

	/* Allocate memory for the readout buffer */
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
    /* Allocate memory for the events */
    ret = CAEN_DGTZ_MallocDPPEvents(handle, (void**)&Events, &AllocatedSize); 
    /* Allocate memory for the waveforms */
    ret = CAEN_DGTZ_MallocDPPWaveforms(handle, (void**)&Waveforms, &AllocatedSize); 
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Can't allocate memory buffers\n", kMBIconStop, kMBOk);
        ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
    }
	
	c1 = fEcanvas1->GetCanvas( );
		
	//enable buttons after DIGI initialisation
	fClearButton->SetState (kButtonUp);
	fStartButton->SetState (kButtonUp);
	fStopButton->SetState (kButtonUp);
	
	for (int i = 0; i<MAX_CH; i++){
		if (i < N_CH)
			Dcfg.ChannelMask & (1<<i) ? fCa[i]->SetState(kButtonDown) : fCa[i]->SetState(kButtonUp); 
		else
			fCa[i]->SetState(kButtonDisabled);
	}	
	
		
	
	fInitButton->SetState (kButtonDisabled);
	fNumericEntries[0]->SetState(kFALSE);
		
}

void MainFrame::ClearHisto()
{

	for (int ch = 0; ch < N_CH; ch++){		 //(int)BoardInfo.Channels
		h_ampl[ch]->Reset("ICESM");
		h_integral[ch]->Reset("ICESM");
		
		if ( ch < 2){
			h_psd_ampl->Reset("ICESM");
			h_psd_int->Reset("ICESM");
			h_int_ampl->Reset("ICESM");
			h_qs_ql->Reset("ICESM");
		}	
	}		
	h_dt->Reset("ICESM");
			
	ec_out = 0;
	
	//ret = CAEN_DGTZ_ClearData(handle); // WHAT DOES IT DO? - only crashes
	
	printf("ClearHisto \n");
	
	
}

void MainFrame::StartButton()
{	
 	printf("Start button \n");
	StartTime = get_time( );
	
	//Store traces if choosen
	if (fSTCheck->GetState() == kButtonDown){
		printf(" Traces will be saved in  : %s \n", fSTTextBuffer->GetString());
		
		ff = TFile::Open(fSTTextBuffer->GetString(),"RECREATE");		
		//ff = new TFile(fSTTextBuffer->GetString(),"WRITE");		
		tree = new TTree("vtree", "vtree");		
		tree->Branch("EventCounter", &ec_out);
		tree->Branch("Channel", &ch_out);
		tree->Branch("TimeStamp", &tst_out); 
		tree->Branch("Trace", &v_out);	 
	}
	
	for (int ch = 0; ch < N_CH; ch++){
		if (h_trace[ch])
			delete h_trace[ch];
		
		sprintf(CName, "h_trace%i", ch);
		h_trace[ch]= new TH1D(CName, CName, Dcfg.RecordLength[ch], 0, Dcfg.RecordLength[ch] * b_width);
		
	}
		
	loop = 1;
	ret = CAEN_DGTZ_SWStartAcquisition(handle);
	
	ReadoutLoop( );
}

void MainFrame::StopButton()
{	
	printf("Stop button \n");
	
	loop = 0;
	ret = CAEN_DGTZ_SWStopAcquisition(handle);
	
	if (fSTCheck->GetState() == kButtonDown){
		tree->Write();
		ff->Write();
		printf(" Data saved as \"%s\" \n", ff->GetName() );
		//delete tree;
		//delete ff;
	}
}




void MainFrame::HandleMenu(Int_t id)
{
   // Handle menu items.
	const char *filetypes[] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            "Text files",    "*.[tT][xX][tT]",
                            0,               0 };	

   switch (id) {

    case M_LOAD_CONFIG:
         {
            static TString dir(".");
            TGFileInfo fi;
            fi.fFileTypes = filetypes;
            fi.fIniDir    = StrDup(dir);
            //printf("fIniDir = %s\n", fi.fIniDir);
            new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
            printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
            dir = fi.fIniDir;
			
			uint32_t log_val_cfg[2][8];
			
			
			TFile *cfile = new TFile(fi.fFilename,"READ");
			
			TTree *ctree = (TTree*)cfile->Get("config"); 
			
			ctree->SetBranchAddress("log_val", log_val_cfg);
			
			ctree->SetBranchAddress("Dcfg.AcqMode", &Dcfg.AcqMode);
			ctree->SetBranchAddress("Dcfg.IOlev", &Dcfg.IOlev);
			ctree->SetBranchAddress("Dcfg.ChannelMask", &Dcfg.ChannelMask);
			ctree->SetBranchAddress("Dcfg.EventAggr", &Dcfg.EventAggr);
			ctree->SetBranchAddress("Dcfg.RecordLength", Dcfg.RecordLength);
			ctree->SetBranchAddress("Dcfg.DCOffset", Dcfg.DCOffset);
			ctree->SetBranchAddress("Dcfg.PreTrigger", Dcfg.PreTrigger);
			ctree->SetBranchAddress("Dcfg.PulsePolarity", Dcfg.PulsePolarity);
			
			ctree->SetBranchAddress("DPPParams.thr", DPPParams.thr);
			
			//printf("entries %lli\n", ctree->GetEntries());
			ctree->GetEntry(0);
			
			for (int i = 0; i < 2; i++)
				for (int ch = 0; ch < 8; ch++){
					printf("log_val[%i][%i] %04X\n", i, ch, log_val_cfg[i][ch]);
					log_val[i][ch] = log_val_cfg[i][ch]; 
				}
			
					
			printf("ChannelMask_cfg %i\n", Dcfg.ChannelMask);
			printf("RecordLength_cfg %i\n", Dcfg.RecordLength[0]);
			printf("DCOffset_cfg[0] %i\n", Dcfg.DCOffset[0]);
			printf("PreTrigger_cfg %i\n", Dcfg.PreTrigger[0]);
			printf("PulsePolarity_cfg[0] %i\n", Dcfg.PulsePolarity[0]);
			printf("thr[0] %i\n", DPPParams.thr[0]);
			
         }
         break;
	
	case M_SAVE_CONFIG:
         {
            const char *filetypes[] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            "Text files",    "*.[tT][xX][tT]",
                            0,               0 }; 
			 
		 	static TString dir(".");
            TGFileInfo fi;
            fi.fFileTypes = filetypes;
            fi.fIniDir    = StrDup(dir);
            new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fi);
                      
		 	TFile *cfile = new TFile(fi.fFilename,"RECREATE");
			//printf("Dcfg.RecordLength[0] %i \n", Dcfg.RecordLength[0]);		
			//printf("Dcfg.thr[0] %i \n", DPPParams.thr[0]);		
			//printf("log_val[0][1] %i \n", log_val[0][1]);		
			
			TTree *ctree = new TTree("config", "config"); 
			//ctree->Branch("Dcfg", &Dcfg);
			ctree->Branch("Dcfg.AcqMode", &Dcfg.AcqMode, "AcqMode/I");
			ctree->Branch("Dcfg.IOlev", &Dcfg.IOlev, "IOlev/I");
			ctree->Branch("Dcfg.ChannelMask", &Dcfg.ChannelMask);
			ctree->Branch("Dcfg.EventAggr", &Dcfg.EventAggr);
			ctree->Branch("Dcfg.RecordLength", &Dcfg.RecordLength, "RecordLength[16]/I");
			ctree->Branch("Dcfg.DCOffset", &Dcfg.DCOffset, "DCOffset[16]/I"); //, 
			ctree->Branch("Dcfg.PreTrigger", &Dcfg.PreTrigger, "PreTrigger[16]/I");
			ctree->Branch("Dcfg.PulsePolarity", &Dcfg.PulsePolarity, "PulsePolarity[16]/I");
			
			ctree->Branch("DPPParams.thr", &DPPParams.thr, "thr[16]/I");
			ctree->Branch("log_val", log_val, "log_val[2][8]/I");
						
			ctree->Fill();
			ctree->Write();
			cfile->Write(); 
         	printf("Config saved - %s \n",fi.fFilename);		
         }
         break;

    case M_FILE_SAVE_HISTO:
		{
			const char *filetypes[] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            "Text files",    "*.[tT][xX][tT]",
                            0,               0 }; 
			 
		 	static TString dir(".");
            TGFileInfo fi;
            fi.fFileTypes = filetypes;
            fi.fIniDir    = StrDup(dir);
            new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fi);
                      
		 	TFile *outfile = new TFile(fi.fFilename,"RECREATE");
			c1->Write("c1");
			for (int i = 0; i<N_CH; i++){
				h_ampl[i]->Write(h_ampl[i]->GetTitle());
				h_integral[i]->Write(h_integral[i]->GetTitle());
			}	
			h_psd_ampl->Write(h_psd_ampl->GetTitle());
			h_dt->Write(h_dt->GetTitle());
			
			outfile->Write(); 
         	printf("File saved - %s \n",fi.fFilename);			 
		 }
        break;

	case M_FILE_SAVE_TRACES:
		printf("M_FILE_SAVE_TRACES\n");
        break;

    case M_FILE_EXIT:
        CloseWindow();   
        break;
	 
	case M_OPT_MENU:
        new OptMenu(gClient->GetRoot(), fMain, 400, 200);
		break; 	 
	
	case M_PARAMS_MENU:
        new ParamsMenu(gClient->GetRoot(), fMain, 400, 400);
		break; 	 
		
	case M_LOGIC_MENU:
        new LogicMenu(gClient->GetRoot(), fMain, 400, 400);
		break; 	 	
	
	case M_MANUAL:
		new TGMsgBox(gClient->GetRoot(), fMain, "Manual", "After a while it will be added \n but noone knows value of while \n because it's a loop", kMBIconAsterisk, kMBOk);
        break;
	
	case M_HELP_ABOUT:
		new TGMsgBox(gClient->GetRoot(), fMain, "About program", "Handmade spectra and waveform reader \n for CAEN V1730", kMBIconAsterisk, kMBOk);
        break;
 
      default:
         printf("Menu item %d selected\n", id);
         break;
   }
}


OptMenu::OptMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, UInt_t options)
{


   fMain = new TGTransientFrame(p, main, w, h, options);
   fMain->Connect("CloseWindow()", "OptMenu", this, "CloseWindow()");
   fMain->DontCallClose(); // to avoid double deletions.

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);


   fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame);

   f1 = new TGCompositeFrame(fMain, 300, 200, kVerticalFrame); //| kFixedWidth
	
	const char *titlelabel[] = {"Amplitude", "Integral", "PSD", "Waveforms"};		
	
	const char *numlabel[3][2] = { {"Xmin","Xmax"}, {"LBound","RBound"}, {"PSD_BIN", " $ " }  };	
	const char *numlabel2[] = { "Xmin","Xmax","Ymin", "Ymax" };	
	
	const Double_t numinit[4][4] = {
		{0, (Double_t)Dcfg.RecordLength[0] * b_width, 2, 3},
		{0, (Double_t)Dcfg.RecordLength[0] * b_width, 2, 3},
		{2, 500, -1000, 50},
		{WF_XMIN, WF_XMAX, WF_YMIN, WF_YMAX}  //(Double_t)Dcfg.RecordLength[0] * b_width
		
	};	

	int iStyle[]	= {0, 2}; 
	double Lmin[]	= {-40000, -40000, -40000,-40000}; 	
	double Lmax[]	= {40000, 40000, 40000, 40000}; 	

	for (int i = 0;i<4;i++){
		
		fGF[i] = new TGGroupFrame(f1, titlelabel[i], kVerticalFrame);
		fGF[i]->SetTitlePos(TGGroupFrame::kLeft); 
		f1->AddFrame(fGF[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//

   		// 2 column, n rows
   		fGF[i]->SetLayoutManager(new TGMatrixLayout(fGF[i], 0, 1, 10));
		
		if (i<3) {
			
   			fF[i][0] = new TGHorizontalFrame(fGF[i], 220, 30);
		   	fGF[i]->AddFrame(fF[i][0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));	
	
	 		for (int j = 0; j < 2; j++) { 
     			fNumericEntriesOpt[i][j] = new TGNumberEntry(fF[i][0], numinit[i][j], 6, 2*i + j  + 30,(TGNumberFormat::EStyle)  iStyle[0]); //
     			fNumericEntriesOpt[i][j]->SetLimits(TGNumberFormat::kNELLimitMinMax, Lmin[0], Lmax[0]);
				fNumericEntriesOpt[i][j]->Connect("ValueSet(Long_t)", "OptMenu", this, "DoSetVal()");
				//fNumericEntriesOpt[i][j]->GetNumberEntry( )->Connect("ReturnPressed()", "OptMenu", this, "DoSetVal()");
	   
     			fF[i][0]->AddFrame(fNumericEntriesOpt[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
     			fLabel[i][j] = new TGLabel(fF[i][0], numlabel[i][j]);
     			fF[i][0]->AddFrame(fLabel[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
			}
		}
		else{
			for (int k=0;k<2;k++){
 	  			fF[i][k] = new TGHorizontalFrame(fGF[i], 220, 30);
			   	fGF[i]->AddFrame(fF[i][k], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
	
	 			for (int j = 0; j < 2; j++) { 
     				fNumericEntriesOpt[i][j+2*k] = new TGNumberEntry(fF[i][k], numinit[i][j+2*k], 6, 2*i + j +2*k + 30,(TGNumberFormat::EStyle)  iStyle[0]); //
     				fNumericEntriesOpt[i][j+2*k]->SetLimits(TGNumberFormat::kNELLimitMinMax, Lmin[0], Lmax[0]);
					fNumericEntriesOpt[i][j+2*k]->Connect("ValueSet(Long_t)", "OptMenu", this, "DoSetVal()");
					//fNumericEntriesOpt[i][j+2*k]->GetNumberEntry( )->Connect("ReturnPressed()", "OptMenu", this, "DoSetVal()");
	   
     				fF[i][k]->AddFrame(fNumericEntriesOpt[i][j+2*k], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
     				fLabel[i][j+2*k] = new TGLabel(fF[i][k], numlabel2[j+2*k]);
     				fF[i][k]->AddFrame(fLabel[i][j+2*k], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
				}			
			
			
			}
		
		}
			
		fGF[i]->Resize();	
	}
	
	fC[0] = new TGCheckButton(f1, new TGHotString("OptStat"), 10);	
	fC[1] = new TGCheckButton(f1, new TGHotString("OptFit"), 11);	
	
	for (int i=0; i<2; i++){
		fC[i]->Connect("Clicked()", "OptMenu", this, "DoCheckBox()");
		f1->AddFrame(fC[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 10, 2) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
	}

	fCloseButton = new TGTextButton(f1, "&Ok", 2);
	fCloseButton->SetFont(sFont);
	fCloseButton->Resize(60, 30);
	fCloseButton->Connect("Clicked()", "OptMenu", this, "DoClose()");

   

	
	f1->AddFrame(fCloseButton, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 10, 10, 3, 3) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
	f1->Resize();

	fMain->AddFrame(f1, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 5, 10, 0) );

  


   fMain->MapSubwindows();
   fMain->Resize();

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Hist options");

   fMain->MapWindow();
   
   //gSystem->ProcessEvents(); 
   //gClient->WaitFor(fMain);
}

// Order is important when deleting frames. Delete children first,
// parents last.

OptMenu::~OptMenu()
{

   fMain->DeleteWindow();  // deletes fMain
}

void OptMenu::CloseWindow()
{
   // Close dialog in response to window manager close.

   delete this;
}

void OptMenu::DoClose()
{
    //Handle Close button.
	
   CloseWindow();
}

void OptMenu::DoSetVal()
{
	
	LBound = fNumericEntriesOpt[1][0]->GetNumber();
	RBound = fNumericEntriesOpt[1][1]->GetNumber();
    PSD_BIN = fNumericEntriesOpt[2][0]->GetNumber();	
	
	WF_XMIN = fNumericEntriesOpt[3][0]->GetNumber();	
	WF_XMAX = fNumericEntriesOpt[3][1]->GetNumber();
	WF_YMIN = fNumericEntriesOpt[3][2]->GetNumber();	
	WF_YMAX = fNumericEntriesOpt[3][3]->GetNumber();
	
   //CloseWindow();
}

void OptMenu::DoCheckBox(){
	
	TGButton *btn = (TGButton *) gTQSender;
	Int_t id = btn->WidgetId();
	
	//Store traces checkbox
	if (id == 10 ) {
		if (fC[0]->GetState() == kButtonDown)
			gStyle->SetOptStat(1111);
		else
			gStyle->SetOptStat(0);
	}
   
  	// To avoid simultaneous PSD and dT 
	if (id == 11) {
		if (fC[1]->GetState() == kButtonDown)
			gStyle->SetOptFit(1111);
		else
			gStyle->SetOptFit(0);
	}	
  
	
	
}

void OptMenu::TryToClose()
{
   // The user try to close the main window,
   //  while a message dialog box is still open.
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}

//////////////////////////////////
////PARAMS_MENU
//////////////////////////////////


ParamsMenu::ParamsMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, UInt_t options)
{
	char titlelabel[10], str[10];
	int j;

   fMain = new TGTransientFrame(p, main, w, h, options);
   fMain->Connect("CloseWindow()", "ParamsMenu", this, "CloseWindow()");
   fMain->DontCallClose(); // to avoid double deletions.

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);


   fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame);

   f1 = new TGCompositeFrame(fMain, 400, 200, kHorizontalFrame); //| kFixedWidth
	
	const char *paramlabel[] = {"Enable", "Polarity", "RecordLength", "Pretrigger", "DCOffset", "thr", "nsbl", "lgate", "sgate", "pgate","selft", "trgc", "discr", "cfdd", "cfdf", "tvaw", "csens"};		
	
	int iStyle[]	= {0, 2}; 
	double Lmin[]	= {-10000, -50, -50,-50}; 	
	double Lmax[]	= {10000, 10000, 10000,10000}; 	
	
	fVF0 = new TGVerticalFrame(f1, 200, 300);
	f1->AddFrame(fVF0, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	for (j = 0; j < 17; j++) { 
	
		fLabel[j] = new TGLabel(fVF0, paramlabel[j]);
		fLabel[j]->SetTextFont(labelFont);
		if (j == 0)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 14, 2)); // left right top bottom
		else
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 4, 4));
	}				
	
	fVF0->Resize();
	
	for (int i = 0; i<N_CH; i++){
		sprintf(titlelabel, "CH%i", i);	
		fGF[i] = new TGGroupFrame(f1, titlelabel, kVerticalFrame);
		fGF[i]->SetTitlePos(TGGroupFrame::kCenter); 
		f1->AddFrame(fGF[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
		
			
   		fVF[i] = new TGVerticalFrame(fGF[i], 8, 30);
		fGF[i]->AddFrame(fVF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));	
			
		// enable channels checkboxies	
		fC[i] = new TGCheckButton(fVF[i], "", i);	
		if (Dcfg.ChannelMask & (1<<i))
			fC[i]->SetState(kButtonDown); 
		else
			fC[i]->SetState(kButtonUp); 
		fC[i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
		fVF[i]->AddFrame(fC[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

		// polarity comboboxies
		fCPol[i] = new TGComboBox(fVF[i], i);	
		fCPol[i]->AddEntry("Negative", 0);
		fCPol[i]->AddEntry("Positive", 1);
		Dcfg.PulsePolarity[i] == CAEN_DGTZ_PulsePolarityNegative? fCPol[i]->Select(0) : fCPol[i]->Select(1);
		fCPol[i]->Resize(50, 20);
		fCPol[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCPol[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
			
		// RecordLength, pretrigger, DCOffset textentries	
		for (j = 0; j < 4; j++) { 
			
			tbuf[i][j] = new TGTextBuffer(10);
			switch (j){
				case 0 :
					sprintf(str, "%i", Dcfg.RecordLength[i]);	
					break;
				case 1 :
					sprintf(str, "%i", Dcfg.PreTrigger[i]);	
					break;	
				case 2 :
					sprintf(str, "%i", Dcfg.DCOffset[i]);	
					break;
				case 3 :
					sprintf(str, "%i", DPPParams.thr[i]);	
					break;	
			}
			
			tbuf[i][j]->AddText(0, str);
				
			fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
			fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
			fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
			fTEntries[i][j]->SetFont(paramFont);
			fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		}
		
		// nsbl comboboxies
		fCnsbl[i] = new TGComboBox(fVF[i], i+16);	
		fCnsbl[i]->AddEntry("0", 0);
		fCnsbl[i]->AddEntry("16", 1);
		fCnsbl[i]->AddEntry("64", 2);
		fCnsbl[i]->AddEntry("256", 3);
		fCnsbl[i]->AddEntry("1024", 4);
		fCnsbl[i]->Select(DPPParams.nsbl[i]);
		fCnsbl[i]->Resize(50, 20);
		fCnsbl[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCnsbl[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		// lgate, sgate, pgate textentries	
		for (j = 4; j < 7; j++) { 
			tbuf[i][j] = new TGTextBuffer(10);
			
			switch (j){
				case 4 :
					sprintf(str, "%i", DPPParams.lgate[i]);	
					break;
				case 5 :
					sprintf(str, "%i", DPPParams.sgate[i]);	
					break;	
				case 6 :
					sprintf(str, "%i", DPPParams.pgate[i]);	
					break;
			}
			
			tbuf[i][j]->AddText(0, str);
				
			fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
			fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
			fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
			fTEntries[i][j]->SetFont(paramFont);
			fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		}
		
		// sel triggers checkboxies	
		fCselft[i] = new TGCheckButton(fVF[i], "", i+16);	
		DPPParams.selft[i] ==0 ? fCselft[i]->SetState(kButtonUp) : fCselft[i]->SetState(kButtonDown); 
		fCselft[i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
		fVF[i]->AddFrame(fCselft[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 4, 4));
		
		//trgc Trigger Configuration
		fCtrgc[i] = new TGComboBox(fVF[i], i+32);	
		fCtrgc[i]->AddEntry("Peak", 0);
		fCtrgc[i]->AddEntry("Threshold", 1);
		fCtrgc[i]->Resize(50, 20);
		fCtrgc[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fCtrgc[i]->SetEnabled(0); // disabled for better times
		fVF[i]->AddFrame(fCtrgc[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//discr LED/CFD
		fCdiscr[i] = new TGComboBox(fVF[i], i+48);	
		fCdiscr[i]->AddEntry("LED", 0);
		fCdiscr[i]->AddEntry("CFD", 1);
		fCdiscr[i]->Select(DPPParams.discr[i]==CAEN_DGTZ_DPP_DISCR_MODE_LED ? 0 : 1);
		fCdiscr[i]->Resize(50, 20);
		fCdiscr[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCdiscr[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//cfdd textentry
		j =7;
		tbuf[i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", DPPParams.cfdd[i]);	
		tbuf[i][j]->AddText(0, str);
				
		fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
		fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
		fTEntries[i][j]->SetFont(paramFont);
		fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		//cfdf fraction 0 - 25%, 1 - 50%, 2 - 75%, 3 - 100%
		fCcfdf[i] = new TGComboBox(fVF[i], i+64);	
		fCcfdf[i]->AddEntry("25%", 0);
		fCcfdf[i]->AddEntry("50%", 1);
		fCcfdf[i]->AddEntry("75%", 2);
		fCcfdf[i]->AddEntry("100%", 3);
		fCcfdf[i]->Select(DPPParams.cfdf[i]);
		fCcfdf[i]->Resize(50, 20);
		fCcfdf[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCcfdf[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//tvaw textentry
		j =8;
		tbuf[i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", DPPParams.tvaw[i]);	
		tbuf[i][j]->AddText(0, str);
				
		fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
		fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
		fTEntries[i][j]->SetFont(paramFont);
		fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		//csens charge sensibility
		fCcsens[i] = new TGComboBox(fVF[i], i+80);	
		fCcsens[i]->AddEntry("5fC/LSB", 0);
		fCcsens[i]->AddEntry("20fC/LSB", 1);
		fCcsens[i]->AddEntry("80fC/LSB", 2);
		fCcsens[i]->AddEntry("320fC/LSB", 3);
		fCcsens[i]->AddEntry("1.28pC/LSB", 4);
		fCcsens[i]->AddEntry("5.12pC/LSB", 5);
		fCcsens[i]->Select(DPPParams.csens[i]);
		fCcsens[i]->Resize(50, 20);
		fCcsens[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCcsens[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		fVF[i]->Resize();
		fGF[i]->Resize();	
	}
	
	fMain->AddFrame(f1, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 5, 10, 0) );


	fMain->MapSubwindows();
	fMain->Resize();

	fMain->CenterOnParent();

	fMain->SetWindowName("Digi params");

	fMain->MapWindow();
}



ParamsMenu::~ParamsMenu()
{

   fMain->DeleteWindow();  // deletes fMain
}

void ParamsMenu::CloseWindow()
{
   

   delete this;
}

void ParamsMenu::DoClose()
{
    //Handle Close button.
	
   CloseWindow();
}

void ParamsMenu::DoSetVal()
{
	//int th_add[16] = {0x1060, 0x1160, 0x1260, 0x1360, 0x1460, 0x1560, 0x1660, 0x1760, 0x1860, 0x1960, 0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60};
	TGTextEntry *te = (TGTextEntry *) gTQSender;
	Int_t id = te->WidgetId();
	Int_t i = id%16;
	Int_t j = id  / 16;
	
	printf("smth changed in %i i = %i j = %i, new val = %s num : %i \n", id, i, j, tbuf[i][j]->GetString(), atoi( tbuf[i][j]->GetString() ) );
	
	if (id <16){
		Dcfg.RecordLength[i] = atoi( tbuf[i][j]->GetString() );
		if (id == 0)
			ret = CAEN_DGTZ_SetRecordLength(handle, Dcfg.RecordLength[id], id);
	}	
	if (id >=16 && id <32)
		Dcfg.PreTrigger[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=32 && id <48){
		Dcfg.DCOffset[id-32] = atoi( tbuf[i][j]->GetString() );
		ret = CAEN_DGTZ_SetChannelDCOffset(handle, id-32, Dcfg.DCOffset[id-32]); //0x8000
	}	
	if (id >=48 && id <64){
		DPPParams.thr[i] = atoi( tbuf[i][j]->GetString() );
		printf(" thr[%i] set to : %i \n", i, DPPParams.thr[i]);	
		uint32_t reg_data;
		//ret = CAEN_DGTZ_ReadRegister(handle, 0x1080, &reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, th_add[id-48], DPPParams.thr[i]);
		ret = CAEN_DGTZ_ReadRegister(handle, th_add[id-48], &reg_data);
		printf(" In  0x%04X 0x%04X \n", th_add[id-48], reg_data);	
		
	}	
	
	if (id >=64 && id <80)
		DPPParams.lgate[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=80 && id <96)
		DPPParams.sgate[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=96 && id <112)
		DPPParams.pgate[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=112 && id <128)
		DPPParams.cfdd[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=128 && id <144)
		DPPParams.tvaw[i] = atoi( tbuf[i][j]->GetString() );

	ret = CAEN_DGTZ_SetDPPParameters(handle, Dcfg.ChannelMask, &DPPParams);	
	for (int i=0;  i <N_CH; i++)
		if (DPPParams.thr[i] > 4000 )
			ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], DPPParams.thr[i]);
	
}

void ParamsMenu::DoCheckBox()
{
	//int th_add[16] = {0x1060, 0x1160, 0x1260, 0x1360, 0x1460, 0x1560, 0x1660, 0x1760, 0x1860, 0x1960, 0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60};
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	printf("checkbox changed %i \n", id);
	
	if (id <16){
		fC[id]->GetState()== kButtonDown ? Dcfg.ChannelMask  |=  (1<<id) : Dcfg.ChannelMask  &= ~ (1<<id);
		printf("channelmask: %i \n", Dcfg.ChannelMask);
	}	
	
	if (id >=16 && id <32 )
		fCselft[id-16]->GetState()== kButtonUp ? DPPParams.selft[id-16] = 0 : DPPParams.selft[id-16] = 1;
	
	ret = CAEN_DGTZ_SetDPPParameters(handle, Dcfg.ChannelMask, &DPPParams);
	for (int i=0;  i <N_CH; i++)
		if (DPPParams.thr[i] > 4000 )
			ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], DPPParams.thr[i]);
}

void ParamsMenu::DoComboBox()
{
	TGComboBox *cb = (TGComboBox *) gTQSender;
	Int_t id = cb->WidgetId();
		
	printf("combobox changed %i  \n", id);
	
	if (id <16 && fCPol[id]->GetSelected() == 0 ){
		Dcfg.PulsePolarity[id]  = CAEN_DGTZ_PulsePolarityNegative;
		printf("Ch[%i] : %s  \n", id, "Negative");
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, id, Dcfg.PulsePolarity[id]);
	}
	
	if (id <16 && fCPol[id]->GetSelected() == 1 ){
		Dcfg.PulsePolarity[id]  = CAEN_DGTZ_PulsePolarityPositive;
		printf("Ch[%i] : %s  \n", id, "Positive");
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, id, Dcfg.PulsePolarity[id]);
	}
	
	if (id >=16 && id <32){
		DPPParams.nsbl[id-16] = fCnsbl[id-16]->GetSelected();
		printf("Ch[%i] nsbl : %i  \n", id-16, fCnsbl[id-16]->GetSelected());
	}	
	
	if (id >=48 && id <64){
		fCdiscr[id-48]->GetSelected() == 0 ? DPPParams.discr[id-48] = CAEN_DGTZ_DPP_DISCR_MODE_LED : DPPParams.discr[id-48] = CAEN_DGTZ_DPP_DISCR_MODE_CFD;
		printf("Ch[%i] discr : %s \n", id-48, fCdiscr[id-48]->GetSelected() == 0 ? "LED" : "CFD" );
	}	
	
	if (id >=64 && id <80){
		DPPParams.cfdf[id-64] = fCcfdf[id-64]->GetSelected();
		printf("Ch[%i] cfdf : %i  \n", id-64, fCcfdf[id-64]->GetSelected()*25 + 25 );
	}	
	
	if (id >=80 && id <96){
		DPPParams.csens[id-80] = fCcsens[id-80]->GetSelected();
		printf("Ch[%i] csens : %i \n", id-80, fCcsens[id-80]->GetSelected());
	}	
	
	ret = CAEN_DGTZ_SetDPPParameters(handle, Dcfg.ChannelMask, &DPPParams);
	for (int i=0;  i <N_CH; i++)
		if (DPPParams.thr[i] > 4000 )
			ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], DPPParams.thr[i]);
}

void ParamsMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}



//////////////////////////////////
////LOGIC_MENU
//////////////////////////////////


LogicMenu::LogicMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, UInt_t options)
{
	char titlelabel[10], str[20];
	int b[2] = {8, 12}; // bit for coinc logic  in couple | board
	
   fMain = new TGTransientFrame(p, main, w, h, options);
   fMain->Connect("CloseWindow()", "LogicMenu", this, "CloseWindow()");
   fMain->DontCallClose( ); // to avoid double deletions.

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);


   fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame); //
	
	TGVerticalFrame *vframe1 = new TGVerticalFrame(fMain, 200, 40);
	TGHorizontalFrame *hframe1 = new TGHorizontalFrame(vframe1, 200, 40);
	
    f1 = new TGCompositeFrame(hframe1, 200, 60, kVerticalFrame); //| kFixedWidth
	fHF0couple = new TGHorizontalFrame(f1, 100, 30);
	f1->AddFrame(fHF0couple, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	f2 = new TGCompositeFrame(hframe1, 200, 60, kVerticalFrame); //| kFixedWidth
	fHF0board = new TGHorizontalFrame(f2, 100, 30);
	f2->AddFrame(fHF0board, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	
	for (int i = 0; i < b[0]; i++) { 
		sprintf(str, "b%i", i);	
		fLabel[0][i] = new TGLabel(fHF0couple, str);
		fLabel[0][i]->SetTextFont(labelFont);
		if (i == 0)
			fHF0couple->AddFrame(fLabel[0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 0, 2, 2)); // left right top bottom
		else
			fHF0couple->AddFrame(fLabel[0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 1, 1, 2, 2));
	}				
	
	fHF0couple->Resize();
		
	for (int i = 0; i < b[1]; i++) { 
		sprintf(str, "b%i", i);	
		fLabel[1][i] = new TGLabel(fHF0board, str);
		fLabel[1][i]->SetTextFont(labelFont);
		if (i == 0)
			fHF0board->AddFrame(fLabel[1][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 0, 2, 2)); // left right top bottom
		else
			fHF0board->AddFrame(fLabel[1][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 2, 2, 2, 2));
	}				
	
	fHF0board->Resize();
	
	
		
	for (int i = 0; i<(int)(N_CH/2); i++){
		sprintf(str, "CH%i-CH%i", 2*i, 2*i+1);	
		fGF[0][i] = new TGGroupFrame(f1, str, kVerticalFrame);
		fGF[0][i]->SetTitlePos(TGGroupFrame::kLeft); 
		f1->AddFrame(fGF[0][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
		fGF[1][i] = new TGGroupFrame(f2, str, kVerticalFrame);
		fGF[1][i]->SetTitlePos(TGGroupFrame::kLeft); 
		f2->AddFrame(fGF[1][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
			
   		fHFcouple[i] = new TGHorizontalFrame(fGF[0][i], 20, 5);
		fHFboard[i] = new TGHorizontalFrame(fGF[1][i], 20, 5);
		fGF[0][i]->AddFrame(fHFcouple[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
		fGF[1][i]->AddFrame(fHFboard[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
			
		for (int j=b[0]-1; j>=0; j--){	
			fCcouple[i][j] = new TGCheckButton(fHFcouple[i], "", i * b[0] + j);	
			if (log_val[0][i] & (1<<j))
				fCcouple[i][j]->SetState(kButtonDown); 
			else
				fCcouple[i][j]->SetState(kButtonUp); 
			fCcouple[i][j]->GetState( ) == kButtonDown ? log_val[0][i] |= (1<<j) : log_val[0][i] &= ~(1<<j); 
			fCcouple[i][j]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
			fHFcouple[i]->AddFrame(fCcouple[i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 4, 4));
		}
				
		sprintf(str, "%02X", log_val[0][i]);	
						
		fTEntries[0][i] = new TGTextEntry(fHFcouple[i], str, i); 
		fTEntries[0][i]->Connect("ReturnPressed()", "LogicMenu", this, "DoSetVal()");	
		fTEntries[0][i]->Resize(40, fTEntries[0][i]->GetDefaultHeight());
		fTEntries[0][i]->SetFont(paramFont);
		fHFcouple[i]->AddFrame(fTEntries[0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY,  5, 0, 3, 3) ); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
				
		for (int j=b[1]-1; j>=0; j--){	
			fCboard[i][j] = new TGCheckButton(fHFboard[i], "", i * b[1] + j+100);	
			if (log_val[1][i] & (1<<j))
				fCboard[i][j]->SetState(kButtonDown); 
			else
				fCboard[i][j]->SetState(kButtonUp); 
			fCboard[i][j]->GetState( ) == kButtonDown ? log_val[1][i] |= (1<<j) : log_val[1][i] &= ~(1<<j); 
			
			fCboard[i][j]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
			if (j==11)
				fHFboard[i]->AddFrame(fCboard[i][j], new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 6, 4, 4));
			else if (j==10)
				fHFboard[i]->AddFrame(fCboard[i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 8, 4, 4));
			else 
				fHFboard[i]->AddFrame(fCboard[i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 3, 3, 4, 4));
		}
		
		sprintf(str, "%04X", log_val[1][i]);	
		fTEntries[1][i] = new TGTextEntry(fHFboard[i], str, i + 10); // tbuf[1][i]
		fTEntries[1][i]->Connect("ReturnPressed()", "LogicMenu", this, "DoSetVal()");	
		fTEntries[1][i]->Resize(50, fTEntries[1][i]->GetDefaultHeight());
		fTEntries[1][i]->SetFont(paramFont);
		fHFboard[i]->AddFrame(fTEntries[1][i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
				
		fHFcouple[i]->Resize( );
		fHFboard[i]->Resize( );
		fGF[0][i]->Resize( );	
		fGF[1][i]->Resize( );	
	}
	
	f1->Resize();
	f2->Resize();
	
	hframe1->AddFrame(f1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) );
	hframe1->AddFrame(f2, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2) );
	hframe1->Resize();
	vframe1->AddFrame(hframe1, new TGLayoutHints(kLHintsTop, 2, 2 , 2, 2) );
	
	TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fMain, 200, 40); 
 	fSetButton = new TGTextButton(hframe2," S&et ", 1);
	fSetButton->SetFont(sFont);
    fSetButton->Resize(60, 30);
	fSetButton->Connect("Clicked()","LogicMenu", this, "SetButton()");
  	hframe2->AddFrame(fSetButton, new TGLayoutHints(kLHintsCenterY |  kLHintsExpandX, 4, 4, 4, 4));

	fSwitchOffButton = new TGTextButton(hframe2,"  OFF  ", 1);
    fSwitchOffButton->SetFont(sFont);
    fSwitchOffButton->Resize(50, 30);
	fSwitchOffButton->Connect("Clicked()","LogicMenu",this,"SwitchOffButton()");	
    hframe2->AddFrame(fSwitchOffButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));
	
		
	hframe2->Resize();
	vframe1->AddFrame(hframe2, new TGLayoutHints(kLHintsBottom | kLHintsCenterX,       2, 2, 2, 2));
	vframe1->Resize();
	
    fMain->AddFrame(vframe1, new TGLayoutHints(kLHintsCenterX,       2, 2, 2, 2));
	

	fMain->MapSubwindows();
	fMain->Resize();

	fMain->CenterOnParent();

	fMain->SetWindowName("Logical scheme");

	fMain->MapWindow();
}



LogicMenu::~LogicMenu()
{

   fMain->DeleteWindow();  // deletes fMain
}

void LogicMenu::CloseWindow()
{
   

   delete this;
}

void LogicMenu::DoClose()
{
    //Handle Close button.
	
   CloseWindow();
}

void LogicMenu::DoSetVal()
{
	
	TGTextEntry *te = (TGTextEntry *) gTQSender;
	Int_t id = te->WidgetId();
	Int_t i, j;
	int b[2] = {8, 12};
		
	if (id < 10){
		i = 0;
		j = id;	
	}
	else{
		i = 1;
		j = id -10;	
	}
		
	log_val[i][j] = strtol( fTEntries[i][j]->GetText( ), 0, 16);
	
	if (log_val[0][j] > 0xFF ){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", "This value can`t be \n more then 0xFF \n", kMBIconStop, kMBOk);
		fTEntries[0][j]->SetText("0");
		log_val[0][j] = 0;
	}	
	if (log_val[1][j] > 0xFFF ){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", "This value can`t be \n more then 0xFFF \n", kMBIconStop, kMBOk);
		fTEntries[1][j]->SetText("0");
		log_val[1][j] = 0;
	}
		
	
	printf("smth changed in %i i = %i j = %i, new val = %s num : %04lX \n", id, i, j, fTEntries[i][j]->GetText( ), strtol( fTEntries[i][j]->GetText( ), 0, 16) ); 
	printf("in memory %04X \n", log_val[i][j] ); 
	
	if (i ==0)
		for (int k=b[i]-1; k>=0; k--){	
			if (log_val[i][j] & (1<<k))
				fCcouple[j][k]->SetState(kButtonDown); 
			else
				fCcouple[j][k]->SetState(kButtonUp); 
		}	
	else	
		for (int k=b[i]-1; k>=0; k--){
			if (log_val[i][j] & (1<<k))
				fCboard[j][k]->SetState(kButtonDown); 
			else
				fCboard[j][k]->SetState(kButtonUp); 
		}
	
}

void LogicMenu::DoCheckBox()
{
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	Int_t i, j, n, b[2] = {8, 12};
	char str[10];
			
	if (id < 100){
		i  = id / b[0];
		j = id  % b[0];
		
		fCcouple[i][j]->GetState( ) == kButtonDown ? log_val[0][i] |= (1<<j) : log_val[0][i] &= ~(1<<j); 		
		sprintf(str, "%02X", log_val[0][i]);	
		printf("%04X \n", log_val[0][i]);	
		fTEntries[0][i]->SetText(str);
	}
	else{
		i = (id - 100) / b[1];
		j = (id - 100)  % b[1];
				
		fCboard[i][j]->GetState( ) == kButtonDown ? log_val[1][i] |= (1<<j) : log_val[1][i] &= ~(1<<j); 		
		sprintf(str, "%04X", log_val[1][i]);	
		printf("%04X \n", log_val[1][i]);	
		fTEntries[1][i]->SetText(str);
	}
	
	printf("checkbox changed %i i = %i j = %i \n", id, i, j ); 
	
}

void LogicMenu::SetButton()
{
  int th_add[16] = {0x1060, 0x1160, 0x1260, 0x1360, 0x1460, 0x1560, 0x1660, 0x1760, 0x1860, 0x1960, 0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60};  
  
  ret = SetLogic(handle, log_val, N_CH);
  
  	
  printf("SetButton \n" ); 
}

void LogicMenu::SwitchOffButton()
{
    //switch off coincidence on board
	
	ret = SwitchOffLogic(handle, N_CH);
	
	printf("Coincidence OFF \n" ); 
}


void LogicMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}

void gui()
{
   new MainFrame(gClient->GetRoot(), 1800, 800);
}

//---- Main program ------------------------------------------------------------

int main(int argc, char **argv)
{
	
   TApplication theApp("App", &argc, argv);

   if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return 1;
   }
   
   memset(&Dcfg, 0, sizeof(Dcfg));
	
	//Configuration file routine
	
	f_ini = fopen(DEFAULT_CONFIG_FILE, "r");
	if (f_ini == NULL) {
		printf("Config file not found!\n");
		ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
	}
	ParseConfigFile(f_ini, &Dcfg, &DPPParams);
	fclose(f_ini);
	
		
	printf("Config's abtained successful TraceLength[0] %i  Polarity[0] %i \n", Dcfg.RecordLength[0], Dcfg.PulsePolarity[0]);
	//Configuration file routine
   
   gui();

   theApp.Run();

   return 0;
}

