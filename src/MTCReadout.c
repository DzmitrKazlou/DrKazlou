#include "MTCReadout.h"
#include "TThread.h"


	extern DigitizerConfig_t Dcfg[2];
	extern ReadoutConfig_t Rcfg;
	extern Histograms_t Histo[2];

	//extern int handle[2];
	extern char *buffer[2];
		
	extern CAEN_DGTZ_DPP_PSD_Event_t   *Events[2][MAX_CH];  // events buffer
	extern CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms[2];         // waveforms buffer
		

/////////////////////////////////////////////
////READOUT FUNCTIONS
/////////////////////////////////////////////

using namespace std;

void InitReadoutConfig(ReadoutConfig_t *Rcfg, int N_CH){
	
	Rcfg->Nch = N_CH; // current hack in code
	Rcfg->fPrint = false; // common print flag for debug
	Rcfg->fStoreTraces = false; // flag to store traces
	Rcfg->fTimer = false; // flag for timer usage
	Rcfg->fInit = false; // flag for Digitizer/s initialisation
	
	Rcfg->loop = -1; // ReadoutLoop flag
	Rcfg->DrawTime = 0.5; // time between histograms drawing in sec

	for (int b = 1; b>=0; b--)
		for (int i = 0; i<Rcfg->Nch; i++)
			Rcfg->TrgCnt[b][i] = 0;
	
}

void InitHisto(Histograms_t *Histo, int RecordLength[MAX_CH], int N_CH, int b, int handle){
	char str[100];
	Color_t color[16] = {kBlue, kRed, kViolet, kGreen+1, kPink-9, kOrange, kMagenta, kCyan-7, kGray, kBlack, kBlue, kRed, kGreen, kOrange-2, kBlack, kOrange+2}; 	
		
	Histo->b = b;
	Histo->handle = handle;

	for (int i=0; i<N_CH; i++){
		Histo->WF_XMIN[i] = 0, Histo->WF_XMAX[i] = RecordLength[0] * b_width;
		Histo->WF_YMIN[i] = -10000, Histo->WF_YMAX[i] = 200;
		Histo->ALBound[i] = 0, Histo->ARBound[i] = RecordLength[0] * b_width;
		Histo->ILBound[i] = 0, Histo->IRBound[i] = RecordLength[0] * b_width;
		Histo->PSD_BIN[i] = 20;
	}
	
	Histo->fBL = true;
	Histo->fTrace = true;
	Histo->fTraceAxisModified = true;
	Histo->fXYPadModified = false;
	
	Histo->fDChannels = true;
	Histo->fDTypes = false;
	Histo->fTTrace = true;
	
	for (int i=0; i<N_CH; i++){
		sprintf(str,"h[%i]_trace%i", b, i);
		Histo->trace[i] = new TH1D(str, str, RecordLength[0], 0, RecordLength[0] * b_width);
		sprintf(str, "h[%i]_ampl%i", b, i);
		Histo->ampl[i]= new TH1D(str, str, 16384, 0, 16384);
		sprintf(str, "h[%i]_integral%i", b, i);
		Histo->integral[i]= new TH1D(str, str, 10000, 0, 200000);
		sprintf(str, "h[%i]_charge%i", b, i);
		Histo->charge[i]= new TH1D(str, str, 1000, 0, 65536/2);
		
		Histo->trace[i]->SetLineColor(color[i]);
		Histo->ampl[i]->SetLineColor(color[i]);
		Histo->integral[i]->SetLineColor(color[i]);
		Histo->charge[i]->SetLineColor(color[i]);
		
		Histo->fDraw[i] = true;
		Histo->cTPos[i] = i+1;
	}
	
	Histo->FirstToDraw = 0;
	Histo->NPad = 1;
	Histo->NPadT = N_CH;
	Histo->CH_2D = 0;
		
	Histo->trace[0]->GetXaxis( )->SetRangeUser(Histo->WF_XMIN[0], Histo->WF_XMAX[0]);
	Histo->trace[0]->GetXaxis( )->SetTitle(" Time, ns");
	Histo->trace[0]->GetYaxis( )->SetRangeUser(Histo->WF_YMIN[0], Histo->WF_YMAX[0]);
	Histo->trace[0]->GetYaxis( )->SetTitleOffset(1.1);
	Histo->trace[0]->GetYaxis( )->SetTitle(" Channels, lbs"); 
	
	sprintf(str,"h[%i]_dt", b);
	Histo->dt = new TH1D(str, str, 3000, -500, 1000);
	Histo->dt->SetLineColor(kBlue);

	sprintf(str,"h[%i]_counts", b);
	Histo->counts = new TH1D(str, str, 16, 0, 16);
	sprintf(str,"h[%i]_layers", b);
	Histo->layers = new TH1D(str, str, 16, 0, 16);
	
	///TH2D	
	for (int i=0; i<N_CH; i++){
		sprintf(str,"h[%i]_int_ampl%i", b, i);
		Histo->int_ampl[i] = new TH2D(str, str, 1000, 0, 16384, 1000, 0, 200000);
		sprintf(str,"h[%i]_psd_ampl%i", b, i);
		Histo->psd_ampl[i] = new TH2D(str, str, 1000, 0, 16384, 1000, 0, 1);
		sprintf(str,"h[%i]_psd_int%i", b, i);
		Histo->psd_int[i] = new TH2D(str, str, 1000, 0, 200000, 1000, 0, 1);
	}	
	
	sprintf(str,"h[%i]_qs_ql", b);
	Histo->qs_ql = new TH2D(str, str, 1000, 0, 50000, 1000, 0, 100000);
	sprintf(str,"h[%i]_xy", b);
	Histo->xy = new TH2D(str, str, 16, 0, 16, 16, 0, 16);
	Histo->xy->SetFillColor(kRed);
	sprintf(str,"h[%i]_rubik", b);
		
}

void DrawTH2D(bool flag, TCanvas *can, TH2D *h, int cPos, char *opt){

	if (flag){
		can->cd(cPos);
		h->SetMarkerStyle(21);
		h->SetMarkerSize(0.4);
		h->SetMarkerColor(kBlue);
		h->Draw(opt);
	}		
	
}

void DrawHistoCh(Histograms_t *Histo, int N_CH, TCanvas *can){
			
	if (Histo->fTrace){
		can->cd(1);
		for (int ch = Histo->FirstToDraw; ch<N_CH; ch++){
			if ( Histo->fDraw[ch] ){
				Histo->trace[ch]->Draw(ch == Histo->FirstToDraw ? "HIST" : "HIST SAME");
				if (ch == Histo->FirstToDraw)
					Histo->trace[ch]->GetYaxis()->SetRangeUser(Histo->WF_YMIN[ch], Histo->WF_YMAX[ch]);
			}
		}
	}	
		
	if (Histo->fAmpl){		
		can->cd(Histo->cAmpl);
		for (int ch = Histo->FirstToDraw; ch<N_CH; ch++)
			if (Histo->fDraw[ch])
				Histo->ampl[ch]->Draw(ch == Histo->FirstToDraw ? "HIST" : "HIST SAME");
	}
	
	if (Histo->fInt){	
		can->cd(Histo->cInt);
		for (int ch = Histo->FirstToDraw; ch<N_CH; ch++)
			if (Histo->fDraw[ch])
				Histo->integral[ch]->Draw(ch == Histo->FirstToDraw ? "HIST" : "HIST SAME");
	}
	
	if (Histo->fCharge){	
		can->cd(Histo->cCharge);
		for (int ch = Histo->FirstToDraw; ch<N_CH; ch++)
			if (Histo->fDraw[ch])
				Histo->charge[ch]->Draw(ch == Histo->FirstToDraw ? "HIST" : "HIST SAME");
	}
	
	if (Histo->fdT){
		can->cd(Histo->cdT);
		Histo->dt->Draw("HIST");
		Histo->dt->GetXaxis()->SetTitle(" Time, ns");
	}
	
	if (Histo->fIA){
		can->cd(Histo->cIA);
		Histo->int_ampl[Histo->CH_2D]->SetMarkerStyle(21);
		Histo->int_ampl[Histo->CH_2D]->SetMarkerSize(0.4);
		Histo->int_ampl[Histo->CH_2D]->SetMarkerColor(kBlue);
		Histo->int_ampl[Histo->CH_2D]->Draw("COLZ");
	}	

	//DrawTH2D(Histo->fIA, can, Histo->int_ampl[Histo->CH_2D], Histo->cIA, Histo->h2Style);//Histo->h2Style
	DrawTH2D(Histo->fPSD_ampl, can, Histo->psd_ampl[Histo->CH_2D], Histo->cPSD_ampl, Histo->h2Style);
	DrawTH2D(Histo->fPSD_int, can, Histo->psd_int[Histo->CH_2D], Histo->cPSD_int, Histo->h2Style);
	DrawTH2D(Histo->fQsl, can, Histo->qs_ql, Histo->cQsl, Histo->h2Style);
	
	if (Histo->fLayers){
		can->cd(Histo->cLayers);
		Histo->layers->Reset("ICESM");
		for (int i = Histo->FirstToDraw; i<N_CH; i++)
			Histo->layers->Fill( i, Histo->fLayersCoeff == false ?  Histo->ampl[i]->GetMean( ) : Histo->ampl[i]->GetMean( ) / Histo->layers_coeff[i] );
		
		Histo->layers->SetLineColor(kMagenta+3);
		Histo->layers->SetFillColor(kMagenta+3);
		Histo->layers->Draw("bar hist");
		Histo->layers->SetBarWidth(0.95);
	}
	
	if (Histo->fCounts){
		can->cd(Histo->cCounts);
		Histo->counts->Reset("ICESM");
		for (int i = Histo->FirstToDraw; i<N_CH; i++)
			Histo->counts->Fill(i, Histo->ampl[i]->GetEntries( ) );
		
		Histo->counts->SetLineColor(kMagenta+3);
		Histo->counts->SetFillColor(kMagenta+3);
		Histo->counts->Draw("bar hist");
		Histo->counts->SetBarWidth(0.95);
	}	
	
	if (Histo->fXY){
		can->cd(Histo->cXY);
		if (!Histo->fXYPadModified){
			char str[20];
			sprintf(str,"Ecanvas1_%i", Histo->cXY);
			TPad *pad = (TPad*)can->GetPrimitive(str);
			//printf("%i x1 %0.2f y1 %0.2f x2 %0.2f y2 %0.2f \n", Histo->fXYPadModified, pad->GetXlowNDC( ), pad->GetYlowNDC( ), pad->GetHNDC( ), pad->GetHNDC( ) );
			pad->SetGrid( );
			pad->SetPad( pad->GetXlowNDC( ), pad->GetYlowNDC( ), pad->GetHNDC( ), pad->GetHNDC( ) ); // to make pad rectangular pad->GetWNDC( ) * pad->GetWh( ) / pad->GetWw( /
		}
		//Histo->xy->Draw("BOX TEXT");
		Histo->xy->Draw("COLZ");	

		if (!Histo->fXYPadModified){
			Histo->xy->GetXaxis( )->SetLabelSize(0.03);
			Histo->xy->GetXaxis( )->SetNdivisions(16, kTRUE);
			Histo->xy->GetYaxis( )->SetLabelSize(0.03);
			Histo->xy->GetYaxis( )->SetNdivisions(16, kTRUE);
			Histo->fXYPadModified = true;
			//printf("%i x1 %0.2f y1 %0.2f x2 %0.2f y2 %0.2f \n", Histo->fXYPadModified, pad->GetXlowNDC( ), pad->GetYlowNDC( ), pad->GetHNDC( ), pad->GetHNDC( ) );
		}

		printf("Histo XY was drawn \n");
	}
		
	can->Modified( );
}

void DrawHistoT(Histograms_t Histo, int N_CH, TCanvas *can){
			
	if (Histo.fTTrace){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
					
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.trace[ch]->Draw("HIST");
				Histo.trace[ch]->GetYaxis()->SetRangeUser(Histo.WF_YMIN[ch], Histo.WF_YMAX[ch]);
				Histo.trace[ch]->GetXaxis()->SetRangeUser(Histo.WF_XMIN[ch], Histo.WF_XMAX[ch]);
			}
		}
	}
	
	if (Histo.fTAmpl){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.ampl[ch]->Draw("HIST");
			}
		}
	}
	
	if (Histo.fTCharge){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.charge[ch]->Draw("HIST");
			}
		}
	}
	
	if (Histo.fTInt){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.integral[ch]->Draw("HIST");
			}
		}
	}
	
	if (Histo.fTIA){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.int_ampl[ch]->Draw("COLZ");
			}
		}
	}
	
	if (Histo.fTPSD_ampl){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.psd_ampl[ch]->Draw("COLZ");
			}
		}
	}
	
	if (Histo.fTPSD_int){
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				can->cd(Histo.cTPos[ch]);
				Histo.psd_int[ch]->Draw("COLZ");
			}
		}
	}
		
}

void FillTraces(int b, int ch, TH1D *hist, bool fBL, int BL_CUT){
	
	//int BL_CUT = 20; 
			
	Double_t BL_mean = 0,  ampl = 0, integral = 0;
				
	Int_t p = Dcfg[b].PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive ? 1: -1; //POLARITY
		
	std::vector <double> vec, vec_bl; 
	uint16_t *WaveLine;
	WaveLine = Waveforms[b]->Trace1;
		
	//hist->Reset("ICESM");	
	
			
		for (int j=0; j<(int)Waveforms[b]->Ns; j++)
			vec_bl.push_back((double)WaveLine[j]);
	
		for ( int j=0; j<BL_CUT; j++)
			BL_mean = BL_mean + vec_bl[j];	
		BL_mean /= BL_CUT;	
		
		for ( int j=0; j<vec_bl.size( ); j++){
			vec.push_back(vec_bl[j] - BL_mean);		
			if (vec[j] * p > ampl)
				ampl = vec[j] * p;
		}

		if (fBL){
			for ( int j=0; j<vec.size( ); j++)
				hist->Fill(j * b_width, vec[j]);
		}
		else{
			for ( int j=0; j<vec_bl.size( ); j++)
				hist->Fill(j * b_width, vec_bl[j]);
		}	

	//printf("FillTraces b[%i] ch[%i] A[%0.2f] %s\n", b, ch, ampl, hist->GetTitle( ));

	vec.clear( );
	vec_bl.clear( );
}

void FillHisto(int b, int ch,  uint32_t ev, Histograms_t Histo, double &ampl, int BL_CUT){ 
			
	//int BL_CUT = 20;
		
	Double_t BL_mean = 0,  integral = 0;
	ampl = 0;
	Int_t m_stamp;
	Double_t psd_val =0, Qs = 0, Ql = 0;
		
	Int_t p = Dcfg[b].PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive ? 1: -1; //POLARITY
		
	//vector <double> vec, vec_bl; 
	uint16_t *WaveLine;
	WaveLine = Waveforms[b]->Trace1;
		
	Histo.trace[ch]->Reset("ICESM");	
			
		for (int j=0; j<(int)Waveforms[b]->Ns; j++)
			Rcfg.Data.vec_bl.push_back((double)WaveLine[j]);
		
		for ( int j=0; j<BL_CUT; j++)
			BL_mean = BL_mean + Rcfg.Data.vec_bl[j];	
		BL_mean /= BL_CUT;	
		
		
		for ( int j=0; j<Rcfg.Data.vec_bl.size( ); j++){
			Rcfg.Data.vec.push_back(Rcfg.Data.vec_bl[j] - BL_mean);
						
			if (Rcfg.Data.vec[j] * p > ampl && j * b_width > Histo.ALBound[ch] && j * b_width < Histo.ARBound[ch]){
				ampl = Rcfg.Data.vec[j] * p;
				m_stamp = j;
		}	
			
			if (j * b_width > Histo.ILBound[ch] && j * b_width < Histo.IRBound[ch])
				integral += Rcfg.Data.vec[j] * p;
		}
		
		if (Histo.fBL){
				for ( int j=0; j<Rcfg.Data.vec.size( ); j++)
					Histo.trace[ch]->Fill(j * b_width, Rcfg.Data.vec[j]);
			}
			else{
				for ( int j=0; j<Rcfg.Data.vec_bl.size( ); j++)
					Histo.trace[ch]->Fill(j * b_width, Rcfg.Data.vec_bl[j]);
			}	
				
		Histo.integral[ch]->Fill(integral);
		Histo.ampl[ch]->Fill(ampl);
		
		//PSD
		
		if (Histo.fDChannels){
			if ( ( (Histo.fPSD_ampl) || (Histo.fPSD_int) || (Histo.fQsl)  )  && (ch == Histo.CH_2D) ){
				for (int j=m_stamp-10; j<Rcfg.Data.vec.size( ); j++){
					if (j<(m_stamp + Histo.PSD_BIN[ch] - 10))
						Qs = Qs + p * Rcfg.Data.vec[j];
					Ql = Ql + p * Rcfg.Data.vec[j];
				}
				psd_val = 1 - ( Qs/Ql );
			
				if (Histo.fPSD_ampl) 
					Histo.psd_ampl[ch]->Fill(ampl, psd_val);
				if (Histo.fPSD_int) 
					Histo.psd_int[ch]->Fill(integral, psd_val);	
				if (Histo.fQsl) 
					Histo.qs_ql->Fill(Ql, Qs);	
			}	
		}
		if (Histo.fDTypes){
			if (Histo.fTPSD_ampl || Histo.fTPSD_int){
				for (int j=m_stamp; j<Rcfg.Data.vec.size( ); j++){
					if (j<(m_stamp + Histo.PSD_BIN[ch] - 10) )
						Qs = Qs + p * Rcfg.Data.vec[j];
					Ql = Ql + p * Rcfg.Data.vec[j];
				}
				psd_val = 1 - ( Qs/Ql );
			
				if (Histo.fPSD_ampl) 
					Histo.psd_ampl[ch]->Fill(ampl, psd_val);
				if (Histo.fPSD_int) 
					Histo.psd_int[ch]->Fill(integral, psd_val);	
			}
		}	
		//PSD
		
		if (Histo.fCharge)
			Histo.charge[ch]->Fill(Events[b][ch][ev].ChargeLong);
	
		if (Histo.fDChannels)
			if (Histo.fIA && ch == Histo.CH_2D)
				Histo.int_ampl[ch]->Fill(ampl, integral);
				
		if (Histo.fDTypes)
			Histo.int_ampl[ch]->Fill(ampl, integral);
		
		//if(Rcfg.fPrint)
		//	printf(" ev[%i]  ch[%i] ampl %f Vec.size %li \n", ev, ch, ampl, Rcfg.Data.vec.size());
	
		if ( Rcfg.fStoreTraces)
			Rcfg.tree->Fill( );
			
			
		
	Rcfg.Data.vec.clear( );
	Rcfg.Data.vec_bl.clear( );
}

/////////////////////////////////////////////////////////////////////
/////END OF HISTOGRAMS FUNCTIONS
/////////////////////////////////////////////////////////////////////

void ReadoutLoop(Histograms_t Histo[2]){	 //Histograms_t *Histo 
	
	char CName[100];
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;	
	CAEN_DGTZ_BoardInfo_t BoardInfo;
	
	uint32_t BufferSize[2], NumEvents[2][MAX_CH];	
	uint64_t PrevRateTime = get_time(), PrevDrawTime;
	double ampl[Rcfg.Nch];
	uint32_t read_ind[2] = {0, 0};
	
	vector <TimeStruct_t> TimeBuffer{};
	TimeStruct_t El_time{};

	int NEvtsPerRead[2]{};
	
	//const char* xlabel[5] = {"5", "6", "7", "8", "9"};
	//const char* ylabel[5] = {"0", "1", "2", "3", "4"};
		
	int handle[2]	= {Histo[0].handle, Histo[1].handle};
	
	
	int CleaningSize = 50000;

	int t_window = 50; // in TriggerTimeTag units - samples, require to multiply by 2ns (seems to be)
	//int t_window = 50000; // for ps mode

	gSystem->Sleep(10); // to get time for cleaup and startacq properly
			
	while(Rcfg.loop == 1 && !ret) {

		
		TThread::Lock( );			
						
		for (int b = Rcfg.N_B-1; b>=0; b-- ){ // read data 2 functions
			
			El_time.b = b;
			Rcfg.Data.b = b;
			ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer[b], &BufferSize[b]); // Read the buffer from the digitizer 
			ReturnErrorHandler(ret, (char*) "ERR_READ_DATA", handle[b], buffer[b], (void **)&Events[b], Waveforms[b]);
		
			read_ind[b]++;
			El_time.read_num = read_ind[b];
			Rcfg.Data.read_num = read_ind[b];
			
			//printf("B[%i] BufferSize: %d\n", b, BufferSize[b]);		
	
			if (BufferSize[b] == 0) 
				continue;

			Rcfg.Nb[b] += BufferSize[b];

			
			ret = CAEN_DGTZ_GetDPPEvents(handle[b], buffer[b], BufferSize[b], (void**)&Events[b], NumEvents[b]);
			ReturnErrorHandler(ret, (char*) "GET_DPPEVENTS", handle[b], buffer[b], (void **)&Events[b], Waveforms[b]);			

			NEvtsPerRead[b] = 0;
				
			for (int ch=0; ch<Rcfg.Nch; ch++){ 
				if (Dcfg[b].AcqMode != CAEN_DGTZ_DPP_ACQ_MODE_List)
					Histo[b].trace[ch]->Reset("ICESM");

				El_time.ch = ch;
				NEvtsPerRead[b] += NumEvents[b][ch];

				for (int ev=0; ev<NumEvents[b][ch]; ev++) {
					if (Rcfg.fStoreTraces){
						Rcfg.Data.ch = ch;
						Rcfg.Data.extra = Events[b][ch][ev].Extras;
						Rcfg.Data.timetag = Events[b][ch][ev].TimeTag;
					}
					
					if (Dcfg[b].AcqMode != CAEN_DGTZ_DPP_ACQ_MODE_List){
						ret = CAEN_DGTZ_DecodeDPPWaveforms(handle[b], (void**)&Events[b][ch][ev], Waveforms[b]);
						//printf("B[%i] Ch[%i] Evt(%i) time %i ret = %i\n ", b , ch, ev, Events[b][ch][ev].TimeTag, ret);
						FillHisto(b, ch, ev, Histo[b], ampl[ch], Dcfg[b].BL_CUT); // all data performance		
					}
					else{
						if ( Rcfg.fStoreTraces)
							Rcfg.tree->Fill( );
					}
					Rcfg.TrgCnt[b][ch]++;						
					El_time.timetag = Events[b][ch][ev].TimeTag;
					//El_time.timestamp = ( ( (uint64_t)(Events[b][ch][ev].Extras & 0xFFFF0000)<<16) + Events[b][ch][ev].TimeTag * 1000 * 2 + 2 * (Events[b][ch][ev].Extras &~ 0xFFFFFC00) );
					////El_time.timestamp = ( ( (uint64_t)(Events[b][ch][ev].Extras & 0xFFFF0000)<<16) + Events[b][ch][ev].TimeTag * 1000 * 2 );
					if (Histo[0].fXY){
						//El_time.timestamp = ( ( (uint64_t)(Events[b][ch][ev].Extras & 0xFFFF0000)<<16) + Events[b][ch][ev].TimeTag * 1000 * 2 + 2 * (Events[b][ch][ev].Extras &~ 0xFFFFFC00) );
					//working with TimeStruct TimeBuffer
						//TimeBuffer.push_back(El_time);
						if (TimeBuffer.empty( ) )
							TimeBuffer.push_back(El_time);
						else	
							SortTime(TimeBuffer, El_time);
					}
					//working with TimeStruct TimeBuffer
				}
			}
			if (Rcfg.fPrint)
				printf("-------B[%i] ----- BufferSize: %d -------- read_ind %i ------NEvtsPerRead %i ---------- TB.size %li  [%i : %i]\n", b, BufferSize[b], read_ind[b], NEvtsPerRead[b], TimeBuffer.size( ), TimeBuffer.front( ).timetag, TimeBuffer.back( ).timetag );				
				//printf("-------B[%i] ----- BufferSize: %d -------- read_ind %i ------NEvtsPerRead %i ---------- TB.size %li  [%li : %li]\n", b, BufferSize[b], read_ind[b], NEvtsPerRead[b], TimeBuffer.size( ), TimeBuffer.front( ).timestamp, TimeBuffer.back( ).timestamp );				
				//printf("-------B[%i] ----- BufferSize: %d -------- read_ind %i ------Nb %i ---------NEvents[0] %i ---- NEvents[1] %i\n", b, BufferSize[b], read_ind[b], Rcfg.Nb[b], NumEvents[b][0], NumEvents[b][1]);					
				//printf("-------B[%i] ----- BufferSize: %d -------- read_ind %i ------Nb %i ----------\n", b, BufferSize[b], read_ind[b], Rcfg.Nb[b]);				
				//printf("-------B[%i] ----- BufferSize: %d -------- read_ind %i ------NEvtsPerRead %i ----------\n", b, BufferSize[b], read_ind[b], NEvtsPerRead[b]);				
				
		}// read data 2 functions b cicle
		
		

		
		if (NEvtsPerRead[0] + NEvtsPerRead[1] > CleaningSize) //check that reading rate less than cleaning size
			printf(" NEvents per read B0 [%i]+ B1 [%i] higher then CleaningSize %i\n", NEvtsPerRead[0], NEvtsPerRead[1], CleaningSize);
						
		//	
		if (Histo[0].fXY && TimeBuffer.size( ) > 100000){ // rework this part in function
			///////////////////////	
			///part of code to get elements of vector <TimeStruct_t> in certain time interval
			/////////////////////
			

			int start = 0;
			int end = 0;
			bool found = false;
			unordered_set<int> unique_b_values{};
			int interval_count = 0;
			int interval_size = 0;

			while (start < CleaningSize) {
			// Clear the set for the new interval
				unique_b_values.clear( );
				end = start;

			// Expand the window until it covers the timewindow
				while (end < CleaningSize && (TimeBuffer[end].timetag - TimeBuffer[start].timetag)<= t_window) {
					//printf(" end %i start %i  B[%i] Ch(%i) TTT end %i B[%i] Ch(%i) TTT start %i dt %i \n", end, start, 
					//			TimeBuffer[end].b, TimeBuffer[end].ch, TimeBuffer[end].timetag, 
					//			TimeBuffer[start].b, TimeBuffer[start].ch, TimeBuffer[start].timetag, TimeBuffer[end].timetag - TimeBuffer[start].timetag);
					unique_b_values.insert(TimeBuffer[end].b);
					if (unique_b_values.size() >= 2 ) { //&& (TimeBuffer[end].timetag - TimeBuffer[start].timetag)> t_window
						found = true;
						break;
					}
					end++;
				}

				if (found) {
					++interval_count;
					interval_size = end-start+1;
					int pos = start;
					printf("\nInterval_count %i interval_size %i \n ", interval_count, interval_size);
					printf("Start position: %i B[%i] Ch(%i) TTT %i Size %i\n", start, TimeBuffer[start].b, TimeBuffer[start].ch, TimeBuffer[start].timetag, end-start+1 );
					printf("End position: %i B[%i] Ch(%i) TTT %i Size %i\n", end,TimeBuffer[end].b, TimeBuffer[end].ch, TimeBuffer[end].timetag, end-start+1 );
					if ( interval_size >= 2){ // >= 3
						vector<int>b0_pos{};
						for (int ind = pos; ind<pos+interval_size; ind++){
							printf("%i B[%i] Ch(%i) TTT %i interval_size %i\n", ind, TimeBuffer[ind].b, TimeBuffer[ind].ch, TimeBuffer[ind].timetag, interval_size);
							if (TimeBuffer[ind].b == 0) b0_pos.push_back(ind);
						}
						//XY Histo handling	
						for ( auto num: b0_pos ){
							for (int ind = pos; ind<pos+interval_size; ind++){
								if (TimeBuffer[ind].b != 0){
									printf("B[%i] Ch(%i) TTT %i B[%i] Ch(%i) TTT %i interval_size %i\n", TimeBuffer[num].b, TimeBuffer[num].ch, TimeBuffer[num].timetag, TimeBuffer[ind].b, TimeBuffer[ind].ch, TimeBuffer[ind].timetag, interval_size);
									Histo[0].xy->Fill( TimeBuffer[num].ch, TimeBuffer[ind].ch); 
								}	
							}	
						}
						//XY Histo handling	
					}
					// easiest case to draw intersection, case of 2 fibers belong to different lines
					if (interval_size == 2){
						Histo[0].xy->Fill( TimeBuffer[pos].b == 0 ? TimeBuffer[pos].ch : TimeBuffer[pos+1].ch, TimeBuffer[pos+1].b == 1 ? TimeBuffer[pos+1].ch : TimeBuffer[pos].ch );
						if ( TimeBuffer[pos].b != 0 && TimeBuffer[pos+1].b != 0)
							printf("\tWhere is an intersection?!?!  \nB[%i] Ch(%i) TTT %i B[%i] Ch(%i) TTT %i\n", TimeBuffer[pos].b, TimeBuffer[pos].ch, TimeBuffer[pos].timetag, TimeBuffer[pos+1].b, TimeBuffer[pos+1].ch, TimeBuffer[pos+1].timetag);	
					}
						

					found = false;
				// Move start to the "end" next position to find other intervals
					start = end;
				} else {
				// No valid interval found starting at 'start', move to the next position
					start++;
				}
			}

			if (interval_count == 0) {
				printf("\n No intervals found with timetag window %i and at least 2 different b values.\n", t_window);
			}
			
			///////////////////////	
			///part of code to get elements of vector <TimeStruct_t> in certain time interval
			/////////////////////
			if (Rcfg.fPrint)
				printf(" ---Cleaning---B[%i]-------[R_ind %i]----------NEvtsPerRead (%i)------ TB.size (%li) \n", 0, read_ind[0], NEvtsPerRead[0], TimeBuffer.size( ) );	
			TimeBuffer.erase(TimeBuffer.begin( ), TimeBuffer.begin( ) + CleaningSize);// cleaning the first 1000 vector elements to avoid expanding the vector to infinity	
		}

		

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
	

		gSystem->ProcessEvents(); 
		TThread::UnLock( );	
		//gSystem->Sleep(50); 
    } // end of readout loop		
				

	
}

CAEN_DGTZ_ErrorCode DataAcquisition( ){ //Histograms_t *Histo	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
	uint64_t start_time = get_time( ), ElapsedTime;
	bool flag = false; 
	
	printf("inside of DataAcquisition\n" );
	
	while(1) { 
				
		if ( (get_time( ) - start_time) % 1000 == 0){
			if (ElapsedTime < (get_time( ) - start_time) / 1000)
				flag = false;
			ElapsedTime = (get_time( ) - start_time) / 1000;
			if (!flag){
				//printf("DAQ T: %li \n", (get_time( ) - s_time)/1000);
				flag = true;
			}
			
		}
		
	
		if (Rcfg.loop == 1){
									
			if (ret) {
				printf("ERR_START_ACQUISITION %i \n", ret);
				//new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
				//ret = QuitMain(handle, buffer[b], (void**)&Events[b], Waveforms[b]);
				for (int b = 0; b<Rcfg.N_B; b++)
					ret = QuitMain(Histo[b].handle, buffer[b], (void**)&Events[b], Waveforms[b]);	
				//ret = QuitMain(Histo[1].handle, buffer[1], (void**)&Events[1], Waveforms[1]);	
			}
			
			ReadoutLoop(Histo); 
		}
		
		if (Rcfg.loop == 0){
			printf("Acquisition stopped\n");

			if (Rcfg.fStoreTraces)
			{	
				Rcfg.ff->cd( );
				Rcfg.tree->Write( );
				Rcfg.ff->Close( );
				printf(" Data saved as \"%s\" \n", Rcfg.ff->GetName( ) );
			}
			Rcfg.loop = -1;
		}
	
	} // infinite loop

	return ret;
}


void* daq_thread(void* ptr){
		
	//int h = (long)ptr;
	//printf("daq thr: %i \n", h);

	Histograms_t  *Histo_ptr, Histo[2];
	Histo_ptr = (Histograms_t*)ptr;

	Histo[0] = *Histo_ptr;
	Histo[1] = *(Histo_ptr+1);
	
	printf("daq thr[%d] handle: %p \n", Histo[0].b, &Histo[0].handle);
	printf("daq thr[%d] handle: %p \n", Histo[1].b, &Histo[1].handle);

	//uint64_t s_time = get_time( );
	
	DataAcquisition( );	
			
	return 0;
}