#include "MTCFrame.h"
#include "MTCLogic.h"
#include "MTCParams.h"
#include "MTCOpt.h"
#include "MTCFunc.h"

extern int N_CH, CH_2D;
extern uint32_t log_val[2][8];
extern int handle;
extern int loop;
extern uint64_t StartTime;
extern DigitizerConfig_t Dcfg;
extern CAEN_DGTZ_ErrorCode ret;

	extern char *buffer;
	
	extern CAEN_DGTZ_DPP_PSD_Event_t   *Events[MAX_CH];  // events buffer
	extern CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms;         // waveforms buffer


MainFrame::MainFrame(const TGWindow *p, UInt_t w, UInt_t h)
{
	handle = -1;
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
	//ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
		
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
		
	uint32_t AllocatedSize;
	
	/*	
	//Hist initialisation	
	for (int ch = 0; ch < N_CH; ch++){ //
		sprintf(CName, "h_ampl%i", ch);
		h_ampl[ch]= new TH1D(CName, CName, 1000, 0, 32000);
		sprintf(CName, "h_integral%i", ch);
		h_integral[ch]= new TH1D(CName, CName, 100000, 0, 100000);
	}
	*/
	
	
	ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB_A4818_V2718, Dcfg.PID, 0, 0, &handle); //15103
	
	
    if(ret != CAEN_DGTZ_Success) {
		sprintf(CName, "Can't open digitizer \n ret = %i", ret);
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
		ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
	}

		
		//GetInfo 
	CAEN_DGTZ_BoardInfo_t BoardInfo;
	ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
	sprintf(CName, "Connected to CAEN Digitizer Model %s \n ROC FPGA Release is %s\n AMC FPGA Release is %s\n", BoardInfo.ModelName, BoardInfo.ROC_FirmwareRel, BoardInfo.AMC_FirmwareRel);
	new TGMsgBox(gClient->GetRoot(), fMain, "Info", CName, kMBIconAsterisk, kMBOk);
	
	
	ret = ProgramDigitizer(handle, Dcfg);
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "ERR_PROGRAM_DIGITIZER_FAILURE \n", kMBIconStop, kMBOk);
        ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
    }	

	// Allocate memory for the readout buffer 
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
    // Allocate memory for the events 
    ret = CAEN_DGTZ_MallocDPPEvents(handle, (void**)&Events, &AllocatedSize); 
    // Allocate memory for the waveforms 
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
	
	printf("handle after init %i \n", handle);
	fInitButton->SetState (kButtonDisabled);
	fNumericEntries[0]->SetState(kFALSE);
	
	ret = DataAcquisition(handle, N_CH);
}



void MainFrame::ClearHisto()
{
/*
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
*/
			
//	ec_out = 0;
	
	//ret = CAEN_DGTZ_ClearData(handle); // WHAT DOES IT DO? - only crashes
	
	printf("ClearHisto \n");
	
	
}

void MainFrame::StartButton()
{	
 	printf("Start button \n");
	//uint64_t StartTime;
	
	StartTime = get_time( );
	printf("Start time %ld \n", StartTime);
	
	/*
	//Store traces if choosen
	if (fSTCheck->GetState() == kButtonDown){
		printf(" Traces will be saved in  : %s \n", fSTTextBuffer->GetString());
		
		ff = TFile::Open(fSTTextBuffer->GetString(),"RECREATE");		
		
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
	*/
	
	loop = 1;
	
	
	
	//ret = CAEN_DGTZ_SWStartAcquisition(handle);
	
	//ReadoutLoop( );
}

void MainFrame::StopButton()
{	
	printf("Stop button \n");
	
	
	loop = 0;
	
	//ret = CAEN_DGTZ_SWStopAcquisition(handle);
	/*
	if (fSTCheck->GetState() == kButtonDown){
		tree->Write();
		ff->Write();
		printf(" Data saved as \"%s\" \n", ff->GetName() );
	}
	*/
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
			/*
			ctree->SetBranchAddress("Dcfg.AcqMode", &Dcfg.AcqMode);
			ctree->SetBranchAddress("Dcfg.IOlev", &Dcfg.IOlev);
			ctree->SetBranchAddress("Dcfg.ChannelMask", &Dcfg.ChannelMask);
			ctree->SetBranchAddress("Dcfg.EventAggr", &Dcfg.EventAggr);
			ctree->SetBranchAddress("Dcfg.RecordLength", Dcfg.RecordLength);
			ctree->SetBranchAddress("Dcfg.DCOffset", Dcfg.DCOffset);
			ctree->SetBranchAddress("Dcfg.PreTrigger", Dcfg.PreTrigger);
			ctree->SetBranchAddress("Dcfg.PulsePolarity", Dcfg.PulsePolarity);
			ctree->SetBranchAddress("DPPParams.thr", DPPParams.thr);
			*/
			
			//printf("entries %lli\n", ctree->GetEntries());
			ctree->GetEntry(0);
			
			for (int i = 0; i < 2; i++)
				for (int ch = 0; ch < 8; ch++){
					printf("log_val[%i][%i] %04X\n", i, ch, log_val_cfg[i][ch]);
					log_val[i][ch] = log_val_cfg[i][ch]; 
				}
			
			/*		
			printf("ChannelMask_cfg %i\n", Dcfg.ChannelMask);
			printf("RecordLength_cfg %i\n", Dcfg.RecordLength[0]);
			printf("DCOffset_cfg[0] %i\n", Dcfg.DCOffset[0]);
			printf("PreTrigger_cfg %i\n", Dcfg.PreTrigger[0]);
			printf("PulsePolarity_cfg[0] %i\n", Dcfg.PulsePolarity[0]);
			printf("thr[0] %i\n", DPPParams.thr[0]);
			*/
			
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
			/*			
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
			*/
						
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
			/*
			for (int i = 0; i<N_CH; i++){
				h_ampl[i]->Write(h_ampl[i]->GetTitle());
				h_integral[i]->Write(h_integral[i]->GetTitle());
			}	
			h_psd_ampl->Write(h_psd_ampl->GetTitle());
			h_dt->Write(h_dt->GetTitle());
			*/
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
