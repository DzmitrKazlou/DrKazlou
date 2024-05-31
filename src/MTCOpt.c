#include "MTCOpt.h"
#include "MTCFunc.h"

extern Double_t WF_XMIN, WF_XMAX, WF_YMIN, WF_YMAX;
extern Double_t LBound, RBound;
extern Int_t PSD_BIN;
extern DigitizerConfig_t Dcfg;

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
		{WF_XMIN, WF_XMAX, WF_YMIN, WF_YMAX}  
		
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