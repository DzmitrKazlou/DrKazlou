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
#include <TGSlider.h>
#include <TGStatusBar.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <RQ_OBJECT.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>
#include <TLatex.h>

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
   
   //M_TEST_MSGBOX,
   //M_TEST_SLIDER,
   //M_TEST_SHUTTER,
   
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


	

   TGHorizontalFrame    *fFStore, *fF[8], *fFTimer;
   TGLabel              *fSTLabel, *fLabel[8], *fTLabel;
   TGNumberEntry        *fNumericEntries[8];
   
   TGTextEntry *fSTTextEntry;
   TGTextBuffer *fSTTextBuffer;
   
	
	TGCheckButton *fSTCheck, *fCTime, *fC[15], *fCsub[2], *fCa[16];	
	
	TGTextButton 	* fInitButton, *fClearButton, *fStartButton, *fStopButton;	
	
	TGStatusBar *fStatusBar;
	
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuOpt, *fMenuHelp;
  
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;
      
   
	TPad *pad; // used only for Rubik 5x5  th2d plot to SetGrid
		
	
public:
	
   MainFrame(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainFrame();
	
	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";
	
	uint64_t StartTime;
	int handle;
	//const int MAX_CH = 16;
	
	char CName[300];
	
   // slots
      
   void CloseWindow();
   void InitButton();
   void ClearHisto();	
   void StartButton();	
   void StopButton();
   void ShowStats();
   
   void DoSetVal();
   void DoCheckBox();
  
   void HandleMenu(Int_t id);
   void HandlePopup() { printf("menu popped up\n"); }
   void HandlePopdown() { printf("menu popped down\n"); }

   void Created() { Emit("Created()"); } //SIGNAL
   void Welcome() { printf("MTRoot. Welcome on board!\n"); }
};