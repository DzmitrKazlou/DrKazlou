#include "MTCFrame.h"
#include "MTCLogic.h"
#include "MTCParams.h"
#include "MTCOpt.h"
//#include "MTCFunc.h"
#include "MTCReadout.h"
#include <pthread.h>

//extern int N_CH;
extern uint32_t log_val[2][2][8];

extern int handle[2];

extern DigitizerConfig_t Dcfg[2];
extern ReadoutConfig_t Rcfg;
extern Histograms_t Histo[2];


	extern char *buffer[2];
	
	extern CAEN_DGTZ_DPP_PSD_Event_t   *Events[2][MAX_CH];  // events buffer
	extern CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms[2];         // waveforms buffer
	
	


MainFrame::MainFrame(const TGWindow *p, UInt_t w, UInt_t h)
{
   fMain = new TGMainFrame(p, w, h);
	Rcfg.main = fMain;

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
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("E&xit", M_FILE_EXIT);
	
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


	TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fMain, 200, 40);
	TGVerticalFrame *vframe1 = new TGVerticalFrame(hframe1, 200, 40);		
    hframe1->AddFrame(vframe1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 2, 2));//
	
	TGGroupFrame *gframe_store = new TGGroupFrame(hframe1, "Data output", kVerticalFrame);
	gframe_store->SetTitlePos(TGGroupFrame::kRight); // right aligned
	vframe1->AddFrame(gframe_store, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 1, 1));//
	
	fSTCheck  = new TGCheckButton(gframe_store, new TGHotString("STORE TRACES"), 40);	
	fSTCheck->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
	gframe_store->AddFrame(fSTCheck, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
	
	fFStore = new TGHorizontalFrame(gframe_store, 200, 40);
	gframe_store->AddFrame(fFStore, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
	fSTLabel = new TGLabel(fFStore, "File name");
	fFStore->AddFrame(fSTLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
	
	//fSTTextEntry = new TGTextEntry(fFStore, fSTTextBuffer = new TGTextBuffer(0)) ;
	fSTTextEntry = new TGTextEntry(gframe_store, fSTTextBuffer = new TGTextBuffer(0)) ;
	fSTTextBuffer->AddText(0, "output.root");
	fSTTextEntry->SetEnabled(0);
	fSTTextEntry->Resize(100, fSTTextEntry->GetDefaultHeight());
	gframe_store->AddFrame(fSTTextEntry, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 0, 2, 1, 1));
	
	gframe_store->Resize();
	
	TGGroupFrame *gframe_opt = new TGGroupFrame(vframe1, "Options", kVerticalFrame);
	gframe_opt->SetTitlePos(TGGroupFrame::kRight); // right aligned
	vframe1->AddFrame(gframe_opt, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 1, 1));//

   // 2 column, n rows
   gframe_opt->SetLayoutManager(new TGMatrixLayout(gframe_opt, 0, 1, 10));
	

	const char *numlabel[] = {"N_CH", "DrawTime", "Board", "CH_2D", "Timer"};	
	//AcqMode combobox
	for (int b = 0; b<Rcfg.N_B; b++){
		fHFAcqMode[b] = new TGHorizontalFrame(gframe_opt, 150, 30);
		gframe_opt->AddFrame(fHFAcqMode[b], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
		fCAcqMode[b] = new TGComboBox(fHFAcqMode[b], b);  // single combobox on mainframe
		fCAcqMode[b]->AddEntry("List", 0);
		fCAcqMode[b]->AddEntry("Waveforms", 1);
		fCAcqMode[b]->Select(1);
		fCAcqMode[b]->Resize(80, 20);
		fCAcqMode[b]->Connect("Selected(Int_t)", "MainFrame", this, "DoComboBox()");
		fHFAcqMode[b]->AddFrame(fCAcqMode[b], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
		fAcqModeLabel[b] = new TGLabel(fHFAcqMode[b], Form("Mode B[%i]", b) );
		fHFAcqMode[b]->AddFrame(fAcqModeLabel[b], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
	}
	
	const Double_t numinit[] = {  (double)Rcfg.Nch, 0.5, 0, 0, 300};	

	int iStyle[]	= {0, 2, 0, 0, 0}; 	
	
   for (int i = 0; i < 5; i++) {
      fHF[i] = new TGHorizontalFrame(gframe_opt, 150, 30);
      gframe_opt->AddFrame(fHF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
      fNumericEntries[i] = new TGNumberEntry(fHF[i], numinit[i], 8, i + 20, (TGNumberFormat::EStyle) iStyle[i]); 
	  fNumericEntries[i]->Connect("ValueSet(Long_t)", "MainFrame", this, "DoSetVal()");
      fHF[i]->AddFrame(fNumericEntries[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
      fLabel[i] = new TGLabel(fHF[i], numlabel[i]);
      fHF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
	   if ( i ==0 || (i == 2 && Rcfg.N_B==1) ) 
		  fNumericEntries[i]->SetState(kFALSE);
	  if ( i ==4) {
		  fNumericEntries[i]->SetState(kFALSE);
		  fCTime = new TGCheckButton(fHF[i], new TGHotString(""), 20);	
		  fCTime->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
		  fHF[i]->AddFrame(fCTime, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 0));
	  }
   }
         
   gframe_opt->Resize();
   
   TGGroupFrame *gframe_ch = new TGGroupFrame(vframe1, "Channels", kVerticalFrame);
	gframe_ch->SetTitlePos(TGGroupFrame::kRight); 
	vframe1->AddFrame(gframe_ch, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 1, 1));//
   
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
	 
	gframe_ch->SetLayoutManager(new TGMatrixLayout(gframe_ch, 0, 2, 3));
	gframe_ch->Resize(); 
   	
	fTab = new TGTab(vframe1, 100, 300);
	fTab->Connect("Selected(Int_t)", "MainFrame", this, "DoTab()");
	TGCompositeFrame *fCF = fTab->AddTab("Channels");
		

	const char *cblabel[] = {"BL_CUT", "TRACES", "AMPL_HIST", "ChargeL", "INTEGRAL", "dT", "PSD_ampl", "PSD_int", "Qsl", "Int vs Ampl", "Layers", "Counts", "XY"};
		
	for (int i = 0; i < 13; i++) {
		if ( i!=10 ){
			fC[i] = new TGCheckButton(fCF, new TGHotString(cblabel[i]), 41+i);	
			fC[i]->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
			fCF->AddFrame(fC[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
		}
		else{ 
			TGHorizontalFrame *fHFsub = new TGHorizontalFrame(fCF, 60, 30);
			fC[i] = new TGCheckButton(fHFsub, new TGHotString(cblabel[i]), 41+i);	
			fC[i]->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
			fHFsub->AddFrame(fC[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
			fCsub[0] = new TGCheckButton(fHFsub, new TGHotString("coeff"), 57);	 // be careful with that number
			fCsub[0]->Connect("Clicked()", "MainFrame", this, "DoCheckBox()");
			fHFsub->AddFrame(fCsub[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
			fCF->AddFrame(fHFsub, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 2, 2, 2));
		}
	}	
	
	fC[0]->SetState(kButtonDown); //BL_CUT ON
	fC[1]->SetState(kButtonDown); //TRACES ON

	for (int b = 0; b<=Rcfg.N_B-1; b++)
		Histo[b].NPad = 1;
	//Histo[1].NPad = 1;
		
	//gframe_hist->Resize( );
	fCF->Resize( );
	
	//separate histograms for every channel
	fCF = fTab->AddTab("Types");
	
	const char *rblabel[] = {"TRACES", "AMPL_HIST", "ChargeL", "INTEGRAL", "PSD_ampl", "PSD_int", "Int vs Ampl"};	
	
	for (int i = 0; i < 7; i++) {
		fRb[i] = new TGRadioButton(fCF, new TGHotString(rblabel[i]), i);	
		fRb[i]->Connect("Clicked()", "MainFrame", this, "DoRadioButton()");
		fCF->AddFrame(fRb[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 12, 2, 3, 3));
	}
	
	fRb[0]->SetState(kButtonDown); //Traces ON
	fCF->Resize( );
	
	vframe1->AddFrame(fTab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 5, 5));//
	 
	TGHorizontalFrame *hf_initb = new TGHorizontalFrame(vframe1, 60, 30);
    fInitButton = new TGTextButton(hf_initb, " In&it ", 1);
    fInitButton->SetFont(sFont); 
    fInitButton->Resize(60, 30);
    fInitButton->Connect("Clicked()","MainFrame",this,"InitButton()");
    hf_initb->AddFrame( fInitButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));	
	
	printf("N_BOARDS %i \n", Rcfg.N_B);
	if (Rcfg.N_B==2){
		fInitSlaveButton = new TGTextButton(hf_initb, " Init2 ", 1);
		fInitSlaveButton->SetFont(sFont); 
		fInitSlaveButton->Resize(60, 30);
		fInitSlaveButton->Connect("Clicked()","MainFrame",this,"InitSlaveButton()");
		hf_initb->AddFrame( fInitSlaveButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));	
	}

	vframe1->AddFrame(hf_initb, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));	

   fClearButton = new TGTextButton(vframe1, " Cle&ar ", 1);
   fClearButton->SetFont(sFont);
   fClearButton->Resize(60, 30);
   fClearButton->SetState (kButtonDisabled);
   fClearButton->Connect("Clicked()","MainFrame",this,"ClearHisto()");
   vframe1->AddFrame(fClearButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));	
	
	Rcfg.TLabel = new TGLabel(vframe1, "          Timer          ");
	Rcfg.TLabel->SetTextFont(sFont);
	Rcfg.TLabel->Resize(200, 30);
	
	vframe1->AddFrame(Rcfg.TLabel, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   	
	
	vframe1->Resize();
	
	fEcanvas1 = new TRootEmbeddedCanvas("Ecanvas1", hframe1, w-280 , h-80);  // CANVAS VALUES  w-280 , h-180
	hframe1->AddFrame(fEcanvas1, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 2, 5, 2,0)); 
	hframe1->Resize();

	
   fMain->AddFrame(hframe1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) );
   
	can = fEcanvas1->GetCanvas( );
		
	TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fMain, 200, 35);
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

	fSensorsButton = new TGTextButton(hframe2,"  T.C&heck  ", 1);
    fSensorsButton->SetFont(sFont);
    fSensorsButton->Resize(60, 30);
	fSensorsButton->SetState (kButtonDisabled);
	fSensorsButton->Connect("Clicked()","MainFrame",this,"SensorsButton( )");	
	hframe2->AddFrame(fSensorsButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));	
    
	hframe2->Resize();
	fMain->AddFrame(hframe2, new TGLayoutHints(kLHintsCenterX,       2, 2, 10, 2));
	
	TGHorizontalFrame *hf_status = new TGHorizontalFrame(fMain, 1200, 35);

	TGVerticalFrame *vf_countrate = new TGVerticalFrame(hf_status, 1000, 30);
	char empty_line[400];
	for (int i=0; i<400-1; i++) 
		empty_line[i] = ' ';
	empty_line[399] = '\0';

	for (int i = 0; i < Rcfg.N_B; i++){
		Rcfg.StatusLabel[i] = new TGLabel(vf_countrate, (char*)empty_line);
		//Rcfg.StatusLabel[i] = new TGLabel(vf_countrate, "          -----------------------------------------------------------------------------------------------------------------------------------------------------------          ");
		Rcfg.StatusLabel[i]->SetTextFont(StatusFont);
		Rcfg.StatusLabel[i]->Resize(800, 20);
		vf_countrate->AddFrame(Rcfg.StatusLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	}
	hf_status->AddFrame(vf_countrate, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));	

	hf_status->Resize();
	fMain->AddFrame(hf_status, new TGLayoutHints(kLHintsLeft,  2, 2, 2, 2) );
      
   //to add icon and name for the application
   fMain->SetWindowName("MTCRoot");
   //fMain->SetIconName("MTCRoot");
   //fMain->SetIconPixmap("/home/valera/Programs/CAEN/MTCRoot/rose512.jpg");
   fMain->MapSubwindows( );

	fMain->Resize( );
	fMain->MapWindow( );

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
  
	//ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
		
	gApplication->Terminate();
}

void MainFrame::DoTab( ){
	
	//printf("Active Tab #%i\n", fTab->GetCurrent( ) );// 0 - Channels, 1 - Types
	//TThread::Lock( );
	can->Clear( );
	//can->SetGrid( );
	int b = DrawBoard;
	
	if (fTab->GetCurrent() == 0){
		Histo[b].fDChannels = true;
		Histo[b].fDTypes = false;
		
		if (Histo[b].NPad == 1)
			can->Divide(1, 1, 0.001, 0.001);
		if (Histo[b].NPad == 2)
			can->Divide(2, 1, 0.001, 0.001);
		if (Histo[b].NPad > 2 && Histo[b].NPad < 5)
			can->Divide(2, 2, 0.001, 0.001);
		if (Histo[b].NPad > 4 && Histo[b].NPad < 7)
			can->Divide(3, 2, 0.001, 0.001);
		if (Histo[b].NPad > 6 && Histo[b].NPad < 10)
			can->Divide(3, 3, 0.001, 0.001);
		if (Histo[b].NPad > 9 && Histo[b].NPad < 13)
			can->Divide(4, 3, 0.001, 0.001);
		if (Histo[b].NPad > 12)
			can->Divide(4, 4, 0.001, 0.001);
	}
	else{
		Histo[b].fDChannels = false;
		Histo[b].fDTypes = true;	
				
		if (Histo[b].NPadT == 1)
			can->Divide(1, 1, 0.001, 0.001);
		if (Histo[b].NPadT == 2)
			can->Divide(2, 1, 0.001, 0.001);
		if (Histo[b].NPadT > 2 && Histo[0].NPadT < 5)
			can->Divide(2, 2, 0.001, 0.001);
		if (Histo[b].NPadT > 4 && Histo[0].NPadT < 7)
			can->Divide(3, 2, 0.001, 0.001);
		if (Histo[b].NPadT > 6 && Histo[0].NPadT < 10)
			can->Divide(3, 3, 0.001, 0.001);
		if (Histo[b].NPadT > 9 && Histo[0].NPadT < 13)
			can->Divide(4, 3, 0.001, 0.001);
		if (Histo[b].NPadT > 12)
			can->Divide(4, 4, 0.001, 0.001);
	}
	
	//can->Modified( );
	//TThread::UnLock( );
}


void MainFrame::DoComboBox( ){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	TGComboBox *cb = (TGComboBox *) gTQSender;
	Int_t id = cb->WidgetId();
	printf("combobox changed %i  Select %i\n", id, cb->GetSelected( ) );
	
	Dcfg[id].AcqMode = cb->GetSelected( ) == 0 ? CAEN_DGTZ_DPP_ACQ_MODE_List : CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
	//Dcfg[1].AcqMode = Dcfg[0].AcqMode;
	
	for (int b = 0; b<Rcfg.N_B; b++){
		ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle[b], Dcfg[b].AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
		//ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle[b], Dcfg.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly);
		printf("Board[%i] AcqMode changed to %s ret = %i \n", b, Dcfg[b].AcqMode == CAEN_DGTZ_DPP_ACQ_MODE_List ? "List Mode" : "Waveforms mode", ret);
	
		uint32_t reg_data = 0;
		ret = CAEN_DGTZ_ReadRegister(handle[b], 0x8000, &reg_data);
		printf(" After SetDPPAcquisitionMode in  0x%04X: %08X \n", 0x8000, reg_data); 	
		reg_data |= (1<<17); //b17 responsible for Extra settings
		//reg_data &= ~(1<<19); //b19 responsible for charge recording [this line will switch off Charge]
		ret = CAEN_DGTZ_WriteRegister(handle[b], 0x8000, reg_data); //should be 0xE0910
		ret = CAEN_DGTZ_ReadRegister(handle[b], 0x8000, &reg_data);
		printf(" Corrected Extra bit in  0x%04X: %08X \n", 0x8000, reg_data); 	
	}
}

void MainFrame::DoRadioButton( ){
	
	TGRadioButton *rbtn = (TGRadioButton *) gTQSender;
	Int_t id = rbtn->WidgetId();
	//printf("radiobutton changed %i  \n", id);
	int b = DrawBoard;

	for (int i = 0; i<7; i++)
		fRb[i]->SetState( i!=id ? kButtonUp : kButtonDown);
	
	Histo[b].fTTrace = false; 
	Histo[b].fTAmpl = false; 
	Histo[b].fTCharge = false; 
	Histo[b].fTInt = false; 
	Histo[b].fTPSD_ampl = false; 
	Histo[b].fTPSD_int = false;  
	Histo[b].fTIA = false;
		 
	if (id == 0)
		Histo[b].fTTrace = true;
	if (id == 1)
		Histo[b].fTAmpl = true;
	if (id == 2)
		Histo[b].fTCharge = true;
	if (id == 3)
		Histo[b].fTInt = true;
	if (id == 4)
		Histo[b].fTPSD_ampl = true;
	if (id == 5)
		Histo[b].fTPSD_int = true;
	if (id == 6)
		Histo[b].fTIA = true;
	
}	

void MainFrame::DoCheckBox(){
	
	TGButton *btn = (TGButton *) gTQSender;
	Int_t id = btn->WidgetId();
	
	//Store traces checkbox
	if (id == 40 ) {
	   fSTTextEntry->SetEnabled( fSTCheck->GetState() == kButtonDown ? 1 : 0 );
	   Rcfg.fStoreTraces = fSTCheck->GetState( ) == kButtonDown ? true : false;
	}  
	
   //Timer checkbox
	if (id == 20 ) {
	   fNumericEntries[4]->SetState( fCTime->GetState() == kButtonDown ? kTRUE : kFALSE );
	   Rcfg.fTimer = fCTime->GetState( ) == kButtonDown ? true : false;
	}  
	
	int b = DrawBoard;	
	for (int b = 0; b<Rcfg.N_B; b++){
		ManageCanvas(can, &Histo[b], Dcfg[0].Nch, fCa, fC, fCsub, id);
		printf("B[%i] fCharge %i fIA %i \n", b, Histo[b].fCharge, Histo[b].fIA );	
	}
	//ManageCanvas(can, &Histo[1], N_CH, fCa, fC, fCsub, id);
	
	
}

void MainFrame::DoSetVal(){
		
	Rcfg.DrawTime = fNumericEntries[1]->GetNumber( );
	DrawBoard = fNumericEntries[2]->GetNumber( );
	for (int b = 0; b<Rcfg.N_B; b++)
		Histo[b].CH_2D = fNumericEntries[3]->GetNumber();
	//Histo[1].CH_2D = fNumericEntries[3]->GetNumber();
	Rcfg.timer_val = fNumericEntries[4]->GetNumber( );
	
}


void MainFrame::InitSlaveButton()
{
	int b = 1;
	uint32_t AllocatedSize;

	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB_A4818_V2718, Dcfg[b].PID_S, 1, 0, &handle[b]); //15103
	
	
    if(ret) {
		sprintf(CName, "Can't open slave digitizer \n ret = %i", ret);
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
	}

	//GetInfo 
	CAEN_DGTZ_BoardInfo_t BoardInfo;
	ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
	sprintf(CName, "Connected to CAEN Digitizer \n Model %s SN: %i \n ROC FPGA Release is %s\n AMC FPGA Release is %s\n", 
				BoardInfo.ModelName, BoardInfo.SerialNumber, BoardInfo.ROC_FirmwareRel, BoardInfo.AMC_FirmwareRel);
	new TGMsgBox(gClient->GetRoot(), fMain, "Info", CName, kMBIconAsterisk, kMBOk);
	Dcfg[b].SerialNumber = BoardInfo.SerialNumber;
		
	Dcfg[b].ChannelMask &= ~0xFFFF;
	for (int i = 0; i<Dcfg[0].Nch; i++)
		Dcfg[b].ChannelMask |= (1<<i);

	ret = ProgramDigitizer(handle[b], Dcfg[b]);
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "ERR_PROGRAM_DIGITIZER_FAILURE \n", kMBIconStop, kMBOk);
        ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
    }	

	// Allocate memory for the readout buffer 
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle[b], &buffer[b], &AllocatedSize);
    // Allocate memory for the events 
    ret = CAEN_DGTZ_MallocDPPEvents(handle[b], (void**)&Events[b], &AllocatedSize); 
    // Allocate memory for the waveforms 
    ret = CAEN_DGTZ_MallocDPPWaveforms(handle[b], (void**)&Waveforms[b], &AllocatedSize); 
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Can't allocate memory buffers\n", kMBIconStop, kMBOk);
        ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
    }
	
	printf("SN:%i Programming and Memory Allocation Finished [%d] handle = %p \n", BoardInfo.SerialNumber, b, &handle[b]);
	uint32_t reg_data = 0;
	
	ret = CAEN_DGTZ_ReadRegister(handle[b], 0x8000, &reg_data);
	printf(" Previously in  0x%04X: %08X \n", 0x8000, reg_data); 	

	// settings of TriggerHold-Off
	//uint32_t TrgHoldOffAddress[MAX_CH] = { 0x1074, 0x1174, 0x1274, 0x1374, 0x1474, 0x1574, 0x1674, 0x1774,
	//									   0x1874, 0x1974, 0x1A74, 0x1B74, 0x1C74, 0x1D74, 0x1E74, 0x1F74};
	for (int ch=0; ch<Dcfg[0].Nch; ch++){
	  ret = CAEN_DGTZ_ReadRegister(handle[b], TrgHoldOffAddress[ch], &reg_data);
	  //printf(" Previously [TrgHoldOff] 0x%04X: %08X \n", TrgHoldOffAddress[ch], reg_data);
	  reg_data = (uint32_t)(Dcfg[b].RecordLength[ch] / 4); // TrgHold-Off in steps of 8 ns, Dcfg.RecordLength in bins | 1 bin = 2 ns
	  ret = CAEN_DGTZ_WriteRegister(handle[b], TrgHoldOffAddress[ch], reg_data); 
	  ret = CAEN_DGTZ_ReadRegister(handle[b], TrgHoldOffAddress[ch], &reg_data);
	  //printf(" Now [TrgHoldOff] 0x%04X: %08X \n", TrgHoldOffAddress[ch], reg_data);
	}
		
	// settings of Extra data configuration
	for (int ch=0; ch<Dcfg[0].Nch; ch++){
	  ret = CAEN_DGTZ_ReadRegister(handle[b], ExtraConfigAddress[ch], &reg_data);
	  //printf(" Previously [ExtraConfig] 0x%04X: %08X \n", ExtraConfigAddress[ch], reg_data);
	  reg_data = (reg_data &~(0x700)) | (1<<9); // clean up [b10:8] and set that value to 010 - Extras = extended time stamps | flags | fine time stamps
	  ret = CAEN_DGTZ_WriteRegister(handle[b], ExtraConfigAddress[ch], reg_data); 
	  //ret = CAEN_DGTZ_ReadRegister(handle[b], ExtraConfigAddress[ch], &reg_data);
	  //printf(" Now [ExtraConfig] 0x%04X: %08X \n", ExtraConfigAddress[ch], reg_data);
	}
	
	//
	//
	//slave
	//
	ret = CAEN_DGTZ_SetAcquisitionMode(handle[b], CAEN_DGTZ_FIRST_TRG_CONTROLLED); 
	ret = CAEN_DGTZ_SetSWTriggerMode(handle[b], CAEN_DGTZ_TRGMODE_DISABLED); //CAEN_DGTZ_TRGMODE_ACQ_ONLY
	ret = CAEN_DGTZ_SetChannelSelfTrigger(handle[b], CAEN_DGTZ_TRGMODE_DISABLED, 0x2);
	ret = CAEN_DGTZ_SetExtTriggerInputMode(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY);

	ret = CAEN_DGTZ_SetRunSynchronizationMode(handle[b], CAEN_DGTZ_RUN_SYNC_TrgOutTrgInDaisyChain);
	
	fInitSlaveButton->SetState (kButtonDisabled);

	InitHisto(&Histo[b], Dcfg[b].RecordLength, Dcfg[0].Nch, b, handle[b]);	

	///////////////////////	
	//second thread for 2nd board
	///////////////////////
	
	if (Rcfg.N_B == 2){ // in case of synchronization
		pthread_t threadB;
		pthread_create(&threadB, NULL, &daq_thread, &Histo);
		gSystem->Sleep(4);
		Rcfg.fInit = true;
	}
}

void MainFrame::InitButton()
{
	int b = 0;	
	uint32_t AllocatedSize;
		
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB_A4818_V2718, Dcfg[b].PID_S, 0, 0, &handle[b]); //15103
	
    if(ret) {
		sprintf(CName, "Can't open main digitizer \n ret = %i", ret);
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
		ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
	}

		
	//GetInfo 
	CAEN_DGTZ_BoardInfo_t BoardInfo;
	ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
	sprintf(CName, "Connected to CAEN Digitizer \n Model %s SN: %i \n ROC FPGA Release is %s\n AMC FPGA Release is %s\n", 
				BoardInfo.ModelName, BoardInfo.SerialNumber, BoardInfo.ROC_FirmwareRel, BoardInfo.AMC_FirmwareRel);
	new TGMsgBox(gClient->GetRoot(), fMain, "Info", CName, kMBIconAsterisk, kMBOk);
	Dcfg[b].SerialNumber = BoardInfo.SerialNumber;

	//Dcfg[b].Nch = N_CH;	
	Dcfg[b].ChannelMask &= ~0xFFFF;
	for (int i = 0; i<Dcfg[0].Nch; i++)
		Dcfg[b].ChannelMask |= (1<<i);
	//printf("Nch %i ChannelMask %08X \n", Dcfg.Nch, Dcfg.ChannelMask);
	
	ret = ProgramDigitizer(handle[b], Dcfg[b]);
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "ERR_PROGRAM_DIGITIZER_FAILURE \n", kMBIconStop, kMBOk);
        ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
    }	

	// Allocate memory for the readout buffer 
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle[b], &buffer[b], &AllocatedSize);
    // Allocate memory for the events 
    ret = CAEN_DGTZ_MallocDPPEvents(handle[b], (void**)&Events[b], &AllocatedSize); 
    // Allocate memory for the waveforms 
    ret = CAEN_DGTZ_MallocDPPWaveforms(handle[b], (void**)&Waveforms[b], &AllocatedSize); 
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Can't allocate memory buffers\n", kMBIconStop, kMBOk);
        ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
    }
		
	printf("SN:%i Programming and Memory Allocation Finished [%d] handle = %p\n", BoardInfo.SerialNumber, b, &handle[b]);
		
	//enable buttons after DIGI initialisation
	fClearButton->SetState (kButtonUp);
	fStartButton->SetState (kButtonUp);
	fStopButton->SetState (kButtonUp);
	fSensorsButton->SetState (kButtonUp);
	
	
	for (int i = 0; i<MAX_CH; i++){
		if (i < Dcfg[0].Nch)
			Dcfg[b].ChannelMask & (1<<i) ? fCa[i]->SetState(kButtonDown) : fCa[i]->SetState(kButtonUp); 
		else
			fCa[i]->SetState(kButtonDisabled);
	}	
	
	//printf("handle after init %i \n", handle);
	fInitButton->SetState(kButtonDisabled);
	fNumericEntries[0]->SetState(kFALSE);

	if (Rcfg.N_B == 1){ // in case of single board
		pthread_t threadB;
		pthread_create(&threadB, NULL, &daq_thread, &Histo);
		gSystem->Sleep(4);
		Rcfg.fInit = true;
	}
		
    InitHisto(&Histo[0], Dcfg[b].RecordLength, Dcfg[0].Nch, 0, handle[0]);	
	
	
	//check EventAggr
	
	uint32_t reg_data = 0;
	
	// ret = CAEN_DGTZ_ReadRegister(handle, 0x800C, &reg_data);
	// printf(" Previously in  0x%08X: %08X \n", 0x800C, reg_data); //this register defines how many aggregates can be contained in the memory.
	// ret = CAEN_DGTZ_WriteRegister(handle, 0x800C, 0xA); // 0x2 - 2 (lowest value); 0xA - 1024 (highest value)  // auto: 0xA
	// ret = CAEN_DGTZ_ReadRegister(handle, 0x800C, &reg_data);
	// printf(" Now in  0x%04X: %08X \n", 0x800C, reg_data);
	
	// //Events per Aggregate
	// ret = CAEN_DGTZ_ReadRegister(handle, 0x8034, &reg_data);
	// printf(" Previously in  0x%04X: %08X \n", 0x8034, reg_data); 
	// ret = CAEN_DGTZ_WriteRegister(handle, 0x8034, 0xA); // 0x3FF - 1024 - dec (highest value)  0x10 - for kinetic meaurements // auto: 0xA
	// ret = CAEN_DGTZ_ReadRegister(handle, 0x8034, &reg_data);
	// printf(" Now in  0x%04X: %08X \n", 0x8034, reg_data);
	
	// uint32_t EventsPerAggregateAddress[MAX_CH] = { 0x1034, 0x1134, 0x1234, 0x1334, 0x1434, 0x1534, 0x1634, 0x1734,
	// 									   0x1834, 0x1934, 0x1A34, 0x1B34, 0x1C34, 0x1D34, 0x1E34, 0x1F34};
	// for (int ch=0; ch<Dcfg[0].Nch; ch++){
	//   ret = CAEN_DGTZ_ReadRegister(handle, EventsPerAggregateAddress[ch], &reg_data);
	//   printf(" Previously [EventsPerAggregateConfig] 0x%04X: %08X \n", EventsPerAggregateAddress[ch], reg_data);
	//   ret = CAEN_DGTZ_WriteRegister(handle, EventsPerAggregateAddress[ch], 0x20); 
	//   ret = CAEN_DGTZ_ReadRegister(handle, EventsPerAggregateAddress[ch], &reg_data);
	//   printf(" Now [EventsPerAggregateConfig] 0x%04X: %08X \n", EventsPerAggregateAddress[ch], reg_data);
	// }

	
	ret = CAEN_DGTZ_ReadRegister(handle[b], 0x8000, &reg_data);
	printf(" Previously in  0x%04X: %08X \n", 0x8000, reg_data); 
	
	// settings of TriggerHold-Off
	//uint32_t TrgHoldOffAddress[MAX_CH] = { 0x1074, 0x1174, 0x1274, 0x1374, 0x1474, 0x1574, 0x1674, 0x1774,
	//									   0x1874, 0x1974, 0x1A74, 0x1B74, 0x1C74, 0x1D74, 0x1E74, 0x1F74};
	for (int ch=0; ch<Dcfg[0].Nch; ch++){
	  ret = CAEN_DGTZ_ReadRegister(handle[b], TrgHoldOffAddress[ch], &reg_data);
	  //printf(" Previously [TrgHoldOff] 0x%04X: %08X \n", TrgHoldOffAddress[ch], reg_data);
	  reg_data = (uint32_t)(Dcfg[b].RecordLength[ch] / 4); // TrgHold-Off in steps of 8 ns, Dcfg.RecordLength in bins | 1 bin = 2 ns
	  ret = CAEN_DGTZ_WriteRegister(handle[b], TrgHoldOffAddress[ch], reg_data); 
	  ret = CAEN_DGTZ_ReadRegister(handle[b], TrgHoldOffAddress[ch], &reg_data);
	  //printf(" Now [TrgHoldOff] 0x%04X: %08X \n", TrgHoldOffAddress[ch], reg_data);
	}
		
	// settings of Extra data configuration
	for (int ch=0; ch<Dcfg[0].Nch; ch++){
	  ret = CAEN_DGTZ_ReadRegister(handle[b], ExtraConfigAddress[ch], &reg_data);
	  //printf(" Previously [ExtraConfig] 0x%04X: %08X \n", ExtraConfigAddress[ch], reg_data);
	  //reg_data = (reg_data &~(0x700)) | (1<<9); // clean up [b10:8] and set that value to 010 - Extras = extended time stamps | flags | fine time stamps
	  reg_data = (reg_data &~(0x700)) | (1<<8); // clean up [b10:8] and set that value to 001 - Extras = extended time stamps | flags 
	  ret = CAEN_DGTZ_WriteRegister(handle[b], ExtraConfigAddress[ch], reg_data); 
	  ret = CAEN_DGTZ_ReadRegister(handle[b], ExtraConfigAddress[ch], &reg_data);
	  printf(" Now [ExtraConfig] 0x%04X: %08X \n", ExtraConfigAddress[ch], reg_data);
	}

	if (Rcfg.N_B == 2){ // in case of synchronization
		ret = CAEN_DGTZ_SetRunSynchronizationMode(handle[b], CAEN_DGTZ_RUN_SYNC_TrgOutTrgInDaisyChain);
		ret = CAEN_DGTZ_SetAcquisitionMode(handle[b], CAEN_DGTZ_SW_CONTROLLED); // CAEN_DGTZ_SW_CONTROLLED
		ret = CAEN_DGTZ_SetSWTriggerMode(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY); //CAEN_DGTZ_TRGMODE_ACQ_ONLY
		ret = CAEN_DGTZ_SetChannelSelfTrigger(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0x2); // Dcfg.ChannelMask
		ret = CAEN_DGTZ_SetExtTriggerInputMode(handle[b], CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT); 
	}
		
	
	//pthread_t threadB;
	//pthread_create(&threadB, NULL, &daq_thread, &Histo[0]);
	//gSystem->Sleep(4);

	
		
}



void MainFrame::ClearHisto()
{ 
	for (int b = Rcfg.N_B-1; b>=0; b--){
		for (int ch = 0; ch < Dcfg[0].Nch; ch++){		
			Histo[b].ampl[ch]->Reset("ICESM");
			Histo[b].integral[ch]->Reset("ICESM");
			Histo[b].charge[ch]->Reset("ICESM");
		
			Histo[b].psd_ampl[ch]->Reset("ICESM");
			Histo[b].psd_int[ch]->Reset("ICESM");
			Histo[b].int_ampl[ch]->Reset("ICESM");
		}
	
		Histo[b].qs_ql->Reset("ICESM");
	
		Histo[b].dt->Reset("ICESM");		
		Histo[b].layers->Reset("ICESM");		
		Histo[b].counts->Reset("ICESM");		
		Histo[b].xy->Reset("ICESM");		
			
		Histo[b].evt = 0;
	}
			
	Rcfg.StartTime = get_time( );
	printf("ClearHisto \n");

}

void MainFrame::ShowGraphics( ){
	bool flag = false; 
	uint64_t start_time = get_time( ), PrevDrawTime = get_time(), ElapsedTime;
	
	while(Rcfg.loop == 1) {
				
		CalcRate(start_time);
				
		if ( (get_time() - PrevDrawTime) >= Rcfg.DrawTime*1000 && Rcfg.loop == 1 ){	
			printf("GUI DrTime[%0.2f] T: %li \n", Rcfg.DrawTime, (get_time( ) - Rcfg.StartTime) );
			//
			
			//DrawRndmHisto(h_rndm, can);
						
			TThread::Lock( );
	
			if (Histo[DrawBoard].fDChannels)
				DrawHistoCh(&Histo[DrawBoard], Rcfg.Nch, can);	
			else
				DrawHistoT(Histo[DrawBoard], Rcfg.Nch, can);		
						
			
			TThread::UnLock( );
			
			//can->Modified( );
			can->Update( );
		

			PrevDrawTime = get_time( );
			
		}	
		gSystem->ProcessEvents(); 	
	}


}

void MainFrame::SensorsButton( ){
		
	CAEN_DGTZ_ErrorCode ret  = CAEN_DGTZ_Success;
	char TempText[2][500], CName[20];
 	printf("Sensors button \n");
	
	uint32_t temp;
	for (int b = 0; b<Rcfg.N_B; b++){
		sprintf(TempText[b], "B[%i] Temp: ", b);
		for (int ch = 0; ch < MAX_CH; ch++) {
			ret = CAEN_DGTZ_ReadTemperature(handle[b], ch, &temp);
			//printf("B[%i] Ch %d  ADC temperature: %d C\n", b, ch, temp);
			sprintf(CName, " |%d|", temp);
			strcat(TempText[b], CName);
		}
		Rcfg.StatusLabel[b]->SetText(TempText[b]);		
	}
	
}

void MainFrame::StartButton()
{	
	CAEN_DGTZ_ErrorCode ret  = CAEN_DGTZ_Success;
	char CName[100];
 	printf("Start button \n");
	fSensorsButton->SetState (kButtonDisabled);

	bool fStart = true;	
	Rcfg.StartTime = get_time( );
	
	for (int b = 0; b<Rcfg.N_B; b++)
		fCAcqMode[b]->SetEnabled(0); 

	//Store traces if choosen
	if (Rcfg.fStoreTraces){
				
		int retval;
		// check if such file exist
		
		if( !gSystem->AccessPathName( fSTTextBuffer->GetString( ) ) ){ //such file exist
			sprintf(CName, "File  %s exist. \n It will be overwritten. \n Continue?", fSTTextBuffer->GetString( ));
			new TGMsgBox(gClient->GetRoot(), fMain, "Warning", CName, kMBIconExclamation, kMBNo | kMBYes, &retval); //1 - Yes, No -2 // strange logic
			printf("retval %d \n", retval);
			retval == 2 ? fStart = false: fStart = true; 
			
			if (fStart){
				sprintf(CName, "Traces will be saved in  \n %s", fSTTextBuffer->GetString( ));
				new TGMsgBox(gClient->GetRoot(), fMain, "Info", CName, kMBIconAsterisk, kMBOk);
				Rcfg.ff = TFile::Open(fSTTextBuffer->GetString(),"RECREATE");		
				Rcfg.ff->cd( );
				Rcfg.tree = new TTree("vtree", "vtree");		
				Rcfg.tree->Branch("ReadNum", &Rcfg.Data.read_num);
				Rcfg.tree->Branch("Board", &Rcfg.Data.b);
				Rcfg.tree->Branch("Channel", &Rcfg.Data.ch);
				Rcfg.tree->Branch("Extras", &Rcfg.Data.extra);
				Rcfg.tree->Branch("TimeTag", &Rcfg.Data.timetag);
				Rcfg.tree->Branch("Trace", &Rcfg.Data.vec);	 
			}
		} 
		else{
			sprintf(CName, "Traces will be saved in  \n %s", fSTTextBuffer->GetString( ));
			new TGMsgBox(gClient->GetRoot(), fMain, "Info", CName, kMBIconAsterisk, kMBOk);
			Rcfg.ff = TFile::Open(fSTTextBuffer->GetString(),"RECREATE");		
			Rcfg.ff->cd( );			
			Rcfg.tree = new TTree("vtree", "vtree");		
			Rcfg.tree->Branch("ReadNum", &Rcfg.Data.read_num);
			Rcfg.tree->Branch("Board", &Rcfg.Data.b);
			Rcfg.tree->Branch("Channel", &Rcfg.Data.ch);
			Rcfg.tree->Branch("Extras", &Rcfg.Data.extra);
			Rcfg.tree->Branch("TimeTag", &Rcfg.Data.timetag);
			Rcfg.tree->Branch("Trace", &Rcfg.Data.vec);	 
		}
	}
	
	Rcfg.loop = 1;
	CAEN_DGTZ_BoardInfo_t BoardInfo;

	for (int b = Rcfg.N_B-1; b>=0; b--)
		ret = CAEN_DGTZ_ClearData(handle[b]);
	//ret = CAEN_DGTZ_ClearData(handle[1]);

	printf("ClearData %s ret = %i\n", !ret ? "OK" : "Problems",  ret);

	//ret = ForceClockSync(handle[0]);
	//printf("ForceClockSync %p ret = %i \n", &handle[0], ret);
	//ret = CAEN_DGTZ_SendSWtrigger(handle[0]);
	//printf("SendSWTrigger %p  ret = %i\n", &handle[0], ret);
	
	for (int b = Rcfg.N_B-1; b>=0; b--){
		ret = CAEN_DGTZ_SWStartAcquisition(handle[b]);
		ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
		printf("Start B[%i] %p SN: %i \n", b, &handle[b], BoardInfo.SerialNumber);
	}
	

	ShowGraphics( );		//all histograms drawing

	if (ret){
		sprintf(CName, "Problems in StartButton \n ret =  %i \n", ret);
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
		for (int b = Rcfg.N_B-1; b>=0; b--)
			ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
	}	
}

void MainFrame::StopButton()
{	
	Rcfg.loop = 0;
	CAEN_DGTZ_ErrorCode ret  = CAEN_DGTZ_Success;
	CAEN_DGTZ_BoardInfo_t BoardInfo;
	printf("Stop button \n");
	fSensorsButton->SetState (kButtonUp);
			
	for ( int b = Rcfg.N_B-1; b>=0; b--){
		fCAcqMode[b]->SetEnabled(1); 
		ret = CAEN_DGTZ_SWStopAcquisition(handle[b]);
		ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);		
		printf("SN: [%i] Stop :  %p\n", BoardInfo.SerialNumber, &handle[b]);
	}
		
	if (ret){
		sprintf(CName, "Problems in StopButton \n ret =  %i \n", ret);
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
		for (int b = Rcfg.N_B-1; b>=0; b--)
			ret = QuitMain(handle[b], buffer[b], (void**)&Events[b], Waveforms[b]);
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
			
			uint32_t log_val_cfg[2][2][8];
			
			
			TFile *cfile = new TFile(fi.fFilename,"READ");
			
			TTree *ctree = (TTree*)cfile->Get("config"); 

			CAEN_DGTZ_InputRange_t InputRange[MAX_CH];
			CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_CH];
			CAEN_DGTZ_DPP_AcqMode_t AcqMode;
			CAEN_DGTZ_IOLevel_t IOlev;
			uint32_t ChannelMask; 	
			int RecordLength[MAX_CH], PreTrigger[MAX_CH], DCOffset[MAX_CH], EventAggr; 
			int thr[MAX_CH], nsbl[MAX_CH], lgate[MAX_CH], sgate[MAX_CH], pgate[MAX_CH], selft[MAX_CH], discr[MAX_CH], cfdd[MAX_CH], cfdf[MAX_CH], tvaw[MAX_CH], csens[MAX_CH];
			
			ctree->SetBranchAddress("AcqMode", &AcqMode);
			ctree->SetBranchAddress("IOlev", &IOlev);
			ctree->SetBranchAddress("EventAggr", &EventAggr);
		
			ctree->SetBranchAddress("ChannelMask", &ChannelMask);
			ctree->SetBranchAddress("InputRange", InputRange); // not shure
			ctree->SetBranchAddress("PulsePolarity", PulsePolarity);
			ctree->SetBranchAddress("RecordLength", RecordLength);
			ctree->SetBranchAddress("PreTrigger", PreTrigger);
			ctree->SetBranchAddress("DCOffset", DCOffset); //, 
			ctree->SetBranchAddress("thr", thr);

			ctree->SetBranchAddress("nsbl", nsbl); // not necessary property
			ctree->SetBranchAddress("lgate", lgate);
			ctree->SetBranchAddress("sgate", sgate);
			ctree->SetBranchAddress("pgate", pgate);	
			ctree->SetBranchAddress("selft", selft);	
			ctree->SetBranchAddress("discr", discr);	
			ctree->SetBranchAddress("cfdd", cfdd);	
			ctree->SetBranchAddress("cfdf", cfdf);	
			ctree->SetBranchAddress("tvaw", tvaw);	
			ctree->SetBranchAddress("csens", csens);		
	
			ctree->SetBranchAddress("log_val", log_val_cfg);

			for (int b = 0; b < Rcfg.N_B; b++){
				ctree->GetEntry(b);
			
				Dcfg[b].AcqMode = AcqMode;
				Dcfg[b].IOlev = IOlev;
				Dcfg[b].EventAggr = EventAggr;			
				Dcfg[b].ChannelMask = ChannelMask;

				for (int ch = 0; ch<MAX_CH; ch++){
					Dcfg[b].InputRange[ch] = InputRange[ch];
					Dcfg[b].PulsePolarity[ch] = PulsePolarity[ch];
					Dcfg[b].RecordLength[ch] = RecordLength[ch];
					Dcfg[b].PreTrigger[ch] = PreTrigger[ch];
					Dcfg[b].DCOffset[ch] = DCOffset[ch];
					Dcfg[b].thr[ch] = thr[ch];
					Dcfg[b].nsbl[ch] = nsbl[ch];
					Dcfg[b].lgate[ch] = lgate[ch];
					Dcfg[b].sgate[ch] = sgate[ch];
					Dcfg[b].pgate[ch] = pgate[ch];
					Dcfg[b].selft[ch] = selft[ch];
					Dcfg[b].discr[ch] = discr[ch];
					Dcfg[b].cfdd[ch] = cfdd[ch];
					Dcfg[b].cfdf[ch] = cfdf[ch];
					Dcfg[b].tvaw[ch] = tvaw[ch];
					Dcfg[b].csens[ch] = csens[ch];
				}
			
				for (int i = 0; i < 2; i++)	{
					for (int ch = 0; ch < 8; ch++){
						printf("B[%i] log_val[%i][%i] %04X\n", b, i, ch, log_val_cfg[b][i][ch]);
						log_val[b][i][ch] = log_val_cfg[b][i][ch]; 
					}
				}	
			}// board loop			
			printf("thr[0] %i\n", thr[0]);
			printf("Dcfg[0].thr[0] %i\n", Dcfg[0].thr[0]);	
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
			printf("Saving config in %s \n",fi.fFilename);
					
			TTree *ctree = new TTree("config", "config"); 
			
			CAEN_DGTZ_InputRange_t InputRange[MAX_CH];
			CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_CH];
			CAEN_DGTZ_DPP_AcqMode_t AcqMode;
			CAEN_DGTZ_IOLevel_t IOlev;
			uint32_t ChannelMask; 	
			int RecordLength[MAX_CH], PreTrigger[MAX_CH], DCOffset[MAX_CH], EventAggr; 
			int thr[MAX_CH], nsbl[MAX_CH], lgate[MAX_CH], sgate[MAX_CH], pgate[MAX_CH], selft[MAX_CH], discr[MAX_CH], cfdd[MAX_CH], cfdf[MAX_CH], tvaw[MAX_CH], csens[MAX_CH];
			
			ctree->Branch("AcqMode", &AcqMode, "AcqMode/I");
			ctree->Branch("IOlev", &IOlev, "IOlev/I");
			ctree->Branch("EventAggr", &EventAggr);
		
			ctree->Branch("ChannelMask", &ChannelMask);
			ctree->Branch("InputRange", InputRange, "InputRange[16]/I"); // not shure // shure shure
			ctree->Branch("PulsePolarity", PulsePolarity, "PulsePolarity[16]/I");
			ctree->Branch("RecordLength", RecordLength, "RecordLength[16]/I");
			ctree->Branch("PreTrigger", PreTrigger, "PreTrigger[16]/I");
			ctree->Branch("DCOffset", DCOffset, "DCOffset[16]/I"); //, 
			ctree->Branch("thr", thr, "thr[16]/I");

			ctree->Branch("nsbl", nsbl, "nsbl[16]/I"); // not necessary property
			ctree->Branch("lgate", lgate, "lgate[16]/I");
			ctree->Branch("sgate", sgate, "sgate[16]/I");
			ctree->Branch("pgate", pgate, "pgate[16]/I");	
			ctree->Branch("selft", selft, "selft[16]/I");	
			ctree->Branch("discr", discr, "discr[16]/I");	
			ctree->Branch("cfdd", cfdd, "cfdd[16]/I");	
			ctree->Branch("cfdf", cfdf, "cfdf[16]/I");	
			ctree->Branch("tvaw", tvaw, "tvaw[16]/I");	
			ctree->Branch("csens", csens, "csens[16]/I");	
	
			ctree->Branch("log_val", log_val, "log_val[2][2][8]/I");
	
			for (int b = 0; b<Rcfg.N_B; b++){
				AcqMode = Dcfg[b].AcqMode;
				IOlev = Dcfg[b].IOlev;
				EventAggr = Dcfg[b].EventAggr;			
				ChannelMask = Dcfg[b].ChannelMask;
				for (int ch = 0; ch<MAX_CH; ch++){
					InputRange[ch] = Dcfg[b].InputRange[ch];
					PulsePolarity[ch] = Dcfg[b].PulsePolarity[ch];
					RecordLength[ch] = Dcfg[b].RecordLength[ch];
					PreTrigger[ch] = Dcfg[b].PreTrigger[ch];
					DCOffset[ch] = Dcfg[b].DCOffset[ch];
					thr[ch] = Dcfg[b].thr[ch];
					nsbl[ch] = Dcfg[b].nsbl[ch];
					lgate[ch] = Dcfg[b].lgate[ch];
					sgate[ch] = Dcfg[b].sgate[ch];
					pgate[ch] = Dcfg[b].pgate[ch];
					selft[ch] = Dcfg[b].selft[ch];
					discr[ch] = Dcfg[b].discr[ch];
					cfdd[ch] = Dcfg[b].cfdd[ch];
					cfdf[ch] = Dcfg[b].cfdf[ch];
					tvaw[ch] = Dcfg[b].tvaw[ch];
					csens[ch] = Dcfg[b].csens[ch];
				}

				ctree->Fill( );
			}// board loop

			cfile->cd( );
			ctree->Write( );
			cfile->Close( ); 
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
			outfile->cd( ); 
			can->Write("can");

			for (int b = 0; b<Rcfg.N_B; b++){
				for (int i = 0; i<Dcfg[0].Nch; i++){
					Histo[b].ampl[i]->Write(Histo[b].ampl[i]->GetTitle( ) );
					Histo[b].integral[i]->Write(Histo[b].integral[i]->GetTitle( ) );
					if (Histo[0].fCharge)
						Histo[b].charge[i]->Write(Histo[b].charge[i]->GetTitle( ) );
					if (Histo[0].fIA)
						Histo[b].int_ampl[i]->Write(Histo[b].int_ampl[i]->GetTitle( ) );
					if (Histo[0].fPSD_ampl)
						Histo[b].psd_ampl[i]->Write(Histo[b].psd_ampl[i]->GetTitle( ) );
					if (Histo[0].fPSD_int)
						Histo[b].psd_int[i]->Write(Histo[b].psd_int[i]->GetTitle( ) );
				}	
			}

			if (Histo[0].fdT)
				Histo[0].dt->Write(Histo[0].dt->GetTitle( ) );
			if (Histo[0].fXY)
				Histo[0].xy->Write(Histo[0].xy->GetTitle( ) );
						
			outfile->Close(); 
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
        new OptMenu(gClient->GetRoot(), fMain, 400, 200, Dcfg[0].Nch);
		break; 	 
	
	case M_PARAMS_MENU:
        new ParamsMenu(gClient->GetRoot(), fMain, 400, 400, Dcfg[0].Nch);
		break; 	 
		
	case M_LOGIC_MENU:
		new LogicMenu(gClient->GetRoot(), fMain, 400, 400, Dcfg[0].Nch);
		break; 	 	
	
	case M_MANUAL:
		new TGMsgBox(gClient->GetRoot(), fMain, "Manual", "After a while it will be added \n but noone knows value of while \n because it's a loop", kMBIconAsterisk, kMBOk);
        break;
	
	case M_HELP_ABOUT:
		new TGMsgBox(gClient->GetRoot(), fMain, "About program", "Handmade spectra and waveform reader \n for CAEN V1730 \n v 2024-12", kMBIconAsterisk, kMBOk);
        break;
 
      default:
         printf("Menu item %d selected\n", id);
         break;
   }
}
