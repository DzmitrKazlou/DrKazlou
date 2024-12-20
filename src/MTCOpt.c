#include "MTCOpt.h"
#include "MTCFunc.h"


//extern DigitizerConfig_t Dcfg;
extern Histograms_t Histo[2];
extern ReadoutConfig_t Rcfg;

OptMenu::OptMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, int N_CH, UInt_t options)
{
	Nch = N_CH;
	char str[20];
	fMain = new TGTransientFrame(p, main, w, h, options);
	fMain->Connect("CloseWindow()", "OptMenu", this, "CloseWindow()");
	fMain->DontCallClose(); // to avoid double deletions.

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);


   fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame);

   f1 = new TGCompositeFrame(fMain, 300, 200, kVerticalFrame); //| kFixedWidth
	
	const char *titlelabel[] = {"Amplitude range search [ns]", "Integral range calc. [ns]", "PSD", "Waveforms"};		
	
	const char *numlabel[3][2] = { {"LBound","RBound"}, {"LBound","RBound"}, {"PSD_BIN", " $ " }  };	
	const char *numlabel2[] = { "Xmin","Xmax","Ymin", "Ymax" };	
	
	//const Int_t numinit[4][4] = {
	//	{Histo.ALBound, Histo.ARBound, 2, 3},
	//	{Histo.ILBound, Histo.IRBound, 2, 3},
	//	{Histo.PSD_BIN, 500, -1000, 50},
	//	{Histo.WF_XMIN, Histo.WF_XMAX, Histo.WF_YMIN, Histo.WF_YMAX}  
	//};	
	
	Int_t numinit[16][4][4];
	
	for (int i = 0; i < Nch; i++){
		numinit[i][0][0] = Histo[0].ALBound[i]; numinit[i][0][1] = Histo[0].ARBound[i]; numinit[i][0][2] = 2; numinit[i][0][3] = 3;
		numinit[i][1][0] = Histo[0].ILBound[i]; numinit[i][1][1] = Histo[0].IRBound[i]; numinit[i][1][2] = 2; numinit[i][1][3] = 3;
		numinit[i][2][0] = Histo[0].PSD_BIN[i]; numinit[i][2][1] = 500; numinit[i][2][2] = -1000; numinit[i][2][3] = 50;
		numinit[i][3][0] = Histo[0].WF_XMIN[i]; numinit[i][3][1] = Histo[0].WF_XMAX[i]; numinit[i][3][2] = Histo[0].WF_YMIN[i]; numinit[i][3][3] = Histo[0].WF_YMAX[i];
	}
	

	int iStyle[]	= {0, 2}; 
	double Lmin[]	= {-40000, -40000, -40000,-40000}; 	
	double Lmax[]	= {40000, 40000, 40000, 40000}; 	
	
	fTab = new TGTab(f1, 300, 300);
			
	for (int n = 0; n < Nch; n++){
		sprintf(str, "[%i]", n);
		fCF = fTab->AddTab(str);
		for (int i = 0;i<4;i++){
		
			fGF[n][i] = new TGGroupFrame(fCF, titlelabel[i], kVerticalFrame);
			fGF[n][i]->SetTitlePos(TGGroupFrame::kLeft); 
			fCF->AddFrame(fGF[n][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//

   			// 2 column, n rows
   			fGF[n][i]->SetLayoutManager(new TGMatrixLayout(fGF[n][i], 0, 1, 10));
		
			if (i<3) {
				fF[n][i][0] = new TGHorizontalFrame(fGF[n][i], 220, 30);
			   	fGF[n][i]->AddFrame(fF[n][i][0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));	
	
	 			for (int j = 0; j < 2; j++) { 
     				fNEOpt[n][i][j] = new TGNumberEntry(fF[n][i][0], numinit[n][i][j], 6, 2*i + j  + 30,(TGNumberFormat::EStyle)  iStyle[0]); //
     				fNEOpt[n][i][j]->SetLimits(TGNumberFormat::kNELLimitMinMax, Lmin[0], Lmax[0]);
					fNEOpt[n][i][j]->Connect("ValueSet(Long_t)", "OptMenu", this, "DoSetVal()");
				
	   
     				fF[n][i][0]->AddFrame(fNEOpt[n][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
     				fLabel[n][i][j] = new TGLabel(fF[n][i][0], numlabel[i][j]);
     				fF[n][i][0]->AddFrame(fLabel[n][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
				}
			}
			else{
				for (int k=0;k<2;k++){
 	  				fF[n][i][k] = new TGHorizontalFrame(fGF[n][i], 220, 30);
				   	fGF[n][i]->AddFrame(fF[n][i][k], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
	
	 				for (int j = 0; j < 2; j++) { 
     					fNEOpt[n][i][j+2*k] = new TGNumberEntry(fF[n][i][k], numinit[n][i][j+2*k], 6, 2*i + j +2*k + 30,(TGNumberFormat::EStyle)  iStyle[0]); //
     					fNEOpt[n][i][j+2*k]->SetLimits(TGNumberFormat::kNELLimitMinMax, Lmin[0], Lmax[0]);
						fNEOpt[n][i][j+2*k]->Connect("ValueSet(Long_t)", "OptMenu", this, "DoSetVal()");
							   
     					fF[n][i][k]->AddFrame(fNEOpt[n][i][j+2*k], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
     					fLabel[n][i][j+2*k] = new TGLabel(fF[n][i][k], numlabel2[j+2*k]);
     					fF[n][i][k]->AddFrame(fLabel[n][i][j+2*k], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
					}			
			
			
				}
		
			}
			
			fGF[n][i]->Resize();	
		}
	
		fCF->Resize(); 
	}
		
	f1->AddFrame(fTab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 5, 5));//
	
	const char *opt_label[] = { "OptStat","OptFit","Print", "TH2D AP", "TH2D COLZ" };	
	
	TGHorizontalFrame *fHFOpt = new TGHorizontalFrame(f1, 200, 60);
	
	fVF[0] = new TGVerticalFrame(fHFOpt, 40, 30);
	fVF[1] = new TGVerticalFrame(fHFOpt, 40, 30);
	
	for (int i=0; i<5; i++){
		if (i<3){
			fC[i] = new TGCheckButton(fVF[0], new TGHotString(opt_label[i]), 10 + i);	
			fVF[0]->AddFrame(fC[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
		}	
		else{
			fC[i] = new TGCheckButton(fVF[1], new TGHotString(opt_label[i]), 10 + i);	
			fVF[1]->AddFrame(fC[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
		}	
		fC[i]->Connect("Clicked()", "OptMenu", this, "DoCheckBox()");
	}
		
	fC[3]->SetState(kButtonDown);
	
	fVF[0]->Resize( );
	fVF[1]->Resize( );
	fHFOpt->AddFrame(fVF[0], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
	fHFOpt->AddFrame(fVF[1], new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
	
	f1->AddFrame(fHFOpt, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) ); //(kLHintsTop | kLHintsExpandX, 2, 2, 3, 0)
	
	f1->Resize();

	fMain->AddFrame(f1, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 5, 5, 0) );

  


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
    	
   CloseWindow();
}

void OptMenu::DoSetVal()
{	for (int b = 0; b< Rcfg.N_B; b++){
		for (int ch = 0; ch < Nch; ch++){
			Histo[b].ALBound[ch] = fNEOpt[ch][0][0]->GetNumber();
			Histo[b].ARBound[ch] = fNEOpt[ch][0][1]->GetNumber();
			Histo[b].ILBound[ch] = fNEOpt[ch][1][0]->GetNumber();
			Histo[b].IRBound[ch] = fNEOpt[ch][1][1]->GetNumber();
			Histo[b].PSD_BIN[ch] = fNEOpt[ch][2][0]->GetNumber();	
	
			Histo[b].WF_XMIN[ch] = fNEOpt[ch][3][0]->GetNumber();	
			Histo[b].WF_XMAX[ch] = fNEOpt[ch][3][1]->GetNumber();
			Histo[b].WF_YMIN[ch] = fNEOpt[ch][3][2]->GetNumber();	
			Histo[b].WF_YMAX[ch] = fNEOpt[ch][3][3]->GetNumber();
		}
		Histo[b].trace[Histo[0].FirstToDraw]->GetXaxis()->SetRangeUser(Histo[0].WF_XMIN[Histo[0].FirstToDraw], Histo[0].WF_XMAX[Histo[0].FirstToDraw]);
		Histo[b].trace[Histo[0].FirstToDraw]->GetXaxis()->SetTitle(" Time, ns");
		Histo[b].trace[Histo[0].FirstToDraw]->GetYaxis()->SetRangeUser(Histo[0].WF_YMIN[Histo[0].FirstToDraw], Histo[0].WF_YMAX[Histo[0].FirstToDraw]);
		Histo[b].trace[Histo[0].FirstToDraw]->GetYaxis()->SetTitleOffset(1.1);
		Histo[b].trace[Histo[0].FirstToDraw]->GetYaxis()->SetTitle(" Channels, lbs"); 

		Histo[b].fTraceAxisModified = true;
	}
}

void OptMenu::DoCheckBox(){
	
	TGButton *btn = (TGButton *) gTQSender;
	Int_t id = btn->WidgetId();
	
	//OptStat
	if (id == 10 ) 
		fC[0]->GetState() == kButtonDown ? gStyle->SetOptStat(1111) : gStyle->SetOptStat(0);
	
	//OptFit     	
	if (id == 11) 
		fC[1]->GetState() == kButtonDown ? gStyle->SetOptFit(1111) : gStyle->SetOptFit(0);
	
	//fPrint	
	if (id == 12) 
		fC[2]->GetState() == kButtonDown ? Rcfg.fPrint = true : Rcfg.fPrint = false;
		
	if (id == 13) { // DrawOption  with marker
		if (fC[3]->GetState() == kButtonDown){
			sprintf(Histo[0].h2Style, "%s", "");
			fC[4]->SetState(kButtonUp);
		}	
		else{
			fC[4]->SetState(kButtonDown);
			sprintf(Histo[0].h2Style, "COLZ");
		}	
	}	
	
	if (id == 14) { // DrawOption  COLZ
		if (fC[4]->GetState() == kButtonDown){
			sprintf(Histo[0].h2Style, "COLZ");
			fC[3]->SetState(kButtonUp);
		}	
		else{
			fC[3]->SetState(kButtonDown);
			sprintf(Histo[0].h2Style, "%s", "");
		}	
	}	
  
	
	
}

void OptMenu::TryToClose()
{
   // The user try to close the main window,
   //  while a message dialog box is still open.
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}