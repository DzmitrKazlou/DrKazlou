#include <stdlib.h>

//#include <../include/CAENDigitizerType.h>
//#include "CAENDigitizer.h"


#include <TROOT.h>
#include <TClass.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TGStatusBar.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGColorSelect.h>
#include <RQ_OBJECT.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TROOT.h"
#include "TStyle.h"

#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>



enum ETestCommandIdentifiers {
   M_LOAD_CONFIG,
   M_SAVE_CONFIG,
   M_FILE_SAVE_HISTO,
   M_FILE_SAVE_TRACES,
   M_FILE_EXIT,
      
	M_OPT_MENU,	
	M_PARAMS_MENU,	
	M_LOGIC_MENU,	

   M_MANUAL,
   M_HELP_ABOUT,
  
};


class MainFrame {

RQ_OBJECT("MainFrame")

private:
   TGMainFrame        *fMain;
   TGDockableFrame    *fMenuDock;
   TGCanvas           *fCanvasWindow;
   TRootEmbeddedCanvas	*fEcanvas1;
   TRootEmbeddedCanvas	*fEcanvas2;	


   TGHorizontalFrame    *fFStore, *fHFAcqMode[2], *fHF[8], *fFTimer;
   TGLabel              *fSTLabel, *fAcqModeLabel[2], *fLabel[8], *fTLabel, *fTempLabel[2]; 
	TGComboBox *fCAcqMode[2];
   TGNumberEntry        *fNumericEntries[8];
   
   TGTextEntry *fSTTextEntry;
   TGTextBuffer *fSTTextBuffer;
   TGTab *fTab;
	
	TGCheckButton *fSTCheck, *fCTime, *fC[15], *fCsub[2], *fCa[16];	
	TGRadioButton *fRb[8];
	
	TGTextButton 	*fInitButton, *fInitSlaveButton, *fClearButton, *fStartButton, *fStopButton, *fSensorsButton;	
	
	//TGStatusBar *fStatusBar;
	
	
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuOpt, *fMenuHelp;
  
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;
      
   
	TPad *pad; // used only for Rubik 5x5  th2d plot to SetGrid
	TCanvas *can;
	//TH1D *h_rndm;
	TH1D *h_rndm = new TH1D(" h_rndm", "h_rndm", 1000, -4, 4);	
	
public:
	
   MainFrame(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainFrame();
	
	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";

	const char *StatusFont = "-Ubuntu-bold-r-*-*-12-*-*-*-*-*-iso8859-1";
	
	uint64_t StartTime;
	
	int DrawBoard = 0;
	
	char CName[300];
	uint32_t TrgHoldOffAddress[16] = { 0x1074, 0x1174, 0x1274, 0x1374, 0x1474, 0x1574, 0x1674, 0x1774,
										   0x1874, 0x1974, 0x1A74, 0x1B74, 0x1C74, 0x1D74, 0x1E74, 0x1F74};
	uint32_t ExtraConfigAddress[16] = { 0x1084, 0x1184, 0x1284, 0x1384, 0x1484, 0x1584, 0x1684, 0x1784,
										   0x1884, 0x1984, 0x1A84, 0x1B84, 0x1C84, 0x1D84, 0x1E84, 0x1F84};
	
   // slots
      
	void CloseWindow();
	void InitButton();
	void InitSlaveButton();
	void ClearHisto();	
	void StartButton();	
	void StopButton();
	void SensorsButton();

	void ShowGraphics( );
    
	void DoSetVal();
	void DoTab();
	void DoComboBox();
	void DoCheckBox();
	void DoRadioButton();
  
   void HandleMenu(Int_t id);
   void HandlePopup() { printf("menu popped up\n"); }
   void HandlePopdown() { printf("menu popped down\n"); }

   void Created() { Emit("Created()"); } //SIGNAL
   void Welcome() { printf("MTRoot. Welcome on board!\n"); }
};