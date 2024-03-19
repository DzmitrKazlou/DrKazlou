#include <stdlib.h>

#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"


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
   M_FILE_PRINT,
   M_FILE_PRINTSETUP,
   M_FILE_EXIT,

   M_TEST_DLG,
   M_TEST_MSGBOX,
   M_TEST_SLIDER,
   M_TEST_SHUTTER,
   M_TEST_DIRLIST,
   M_TEST_FILELIST,
   M_TEST_PROGRESS,
   M_TEST_NUMBERENTRY,

	M_OPT_MENU,	
	M_PARAMS_MENU,	
	M_LOGIC_MENU,	

   M_MANUAL,
   M_HELP_ABOUT,

   M_CASCADE_1,
   M_CASCADE_2,
   M_CASCADE_3,

   M_NEW_REMOVEMENU,
  
};

/*
typedef struct
{
    //CAEN_DGTZ_ConnectionType LinkType;
	uint32_t PID;
	int Nch;
    //uint32_t VMEBaseAddress;
	uint32_t RecordLength[MAX_CH];
	uint32_t ChannelMask;
	uint32_t PreTrigger[MAX_CH];
	uint32_t DCOffset[MAX_CH];
    int EventAggr;
    CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_CH];
    CAEN_DGTZ_DPP_AcqMode_t AcqMode;
    CAEN_DGTZ_IOLevel_t IOlev;
} DigitizerConfig_t;

*/

class MainFrame {

RQ_OBJECT("MainFrame")

private:
   TGMainFrame        *fMain;
   TGDockableFrame    *fMenuDock;
   TGCanvas           *fCanvasWindow;
   TRootEmbeddedCanvas	*fEcanvas1;
   TRootEmbeddedCanvas	*fEcanvas2;	

   TGTextEntry        *fTestText;
   TGButton           *fTestButton1;
   TGButton           *fTestButton2;	
   TGColorSelect      *fColorSel;

	
	

   TGHorizontalFrame    *fFStore, *fF[8], *fFTimer;
   TGLabel              *fSTLabel, *fLabel[8], *fTLabel;
   TGNumberEntry        *fNumericEntries[8];
   
   TGTextEntry *fSTTextEntry;
   TGTextBuffer *fSTTextBuffer;
   
	//TGRadioButton *fR[4];
	TGCheckButton *fSTCheck, *fC[10], *fCa[16];	
	
	TGTextButton 	* fInitButton, *fClearButton, *fStartButton, *fStopButton;	
	
	TGStatusBar *fStatusBar;
	
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuOpt, *fMenuHelp;
  
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;

      
   TCanvas 			*c1;
	TPad *pad;
		
	
public:
	
   MainFrame(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainFrame();

   // slots
   
   void SaveTraces( ); 
   void FillHisto(Int_t ch, Double_t &ampl, uint32_t timestamp ); 
   void DrawHisto( ); 
   void ReadoutLoop();
   void CloseWindow();
   void InitButton();
   void ClearHisto();	
   void StartButton();	
   void StopButton();
   
   void DoSetVal();
   void DoCheckBox();
  
   void HandleMenu(Int_t id);
   void HandlePopup() { printf("menu popped up\n"); }
   void HandlePopdown() { printf("menu popped down\n"); }

   void Created() { Emit("Created()"); } //*SIGNAL*
   void Welcome() { printf("MTRoot. Welcome on board!\n"); }
};

class OptMenu {
	
RQ_OBJECT("OptMenu")

private:
   TGTransientFrame     *fMain;
   TGCompositeFrame     *f1, *f2, *f3;
   TGCheckButton *fC[2];	
   TGTextButton      *fCloseButton;
   
	
   TGGroupFrame *fGF[4];	
	TGHorizontalFrame    *fF[4][2];
   TGLabel              *fLabel[4][4];
   //TGNumberEntry        *fNumericEntries[4][4];
   TGNumberEntry        *fNumericEntriesOpt[4][4];
	
   	

public:
   OptMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
              UInt_t options = kVerticalFrame);

   virtual ~OptMenu();

	
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
 
   void DoCheckBox();
   void DoSetVal();
};


class ParamsMenu {
	
RQ_OBJECT("ParamsMenu")

private:
   TGTransientFrame     *fMain;
   TGCompositeFrame     *f1;
   
   //TGTextButton      *fSetButton;
   
	
   TGGroupFrame *fGF[16];	
	
	TGVerticalFrame    *fVF0, *fVF[16];
	
	TGCheckButton *fC[16], *fCselft[16];	
	TGComboBox *fCPol[16], *fCnsbl[16], *fCtrgc[16], *fCdiscr[16], *fCcfdf[16], *fCcsens[16];
	
   TGLabel              *fLabel[20];
   
   TGNumberEntry        *fNumericEntries[16][16];
   
	TGTextEntry        *fTEntries[16][16];
	TGTextBuffer *tbuf[16][16];
   	

public:
   ParamsMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
              UInt_t options = kHorizontalFrame);

   virtual ~ParamsMenu();

	int th_add[16] = {0x1060, 0x1160, 0x1260, 0x1360, 0x1460, 0x1560, 0x1660, 0x1760, 0x1860, 0x1960, 0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60};
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
   void DoSetVal();
   void DoCheckBox();
   void DoComboBox();
   
};



class LogicMenu {
	
RQ_OBJECT("LogicMenu")

private:
   TGTransientFrame     *fMain;
   TGCompositeFrame     *f1, *f2;
   
   TGTextButton      *fSetButton, *fSwitchOffButton;
   
	
   TGGroupFrame *fGF[2][8];	
	TGHorizontalFrame    *fHF0couple, *fHF0board, *fHFcouple[8], *fHFboard[8];
	
	
	TGCheckButton *fCcouple[8][8], *fCboard[8][12];	
		
   TGLabel              *fLabel[2][12];
   
   TGNumberEntry        *fNumericEntries[16][16];
   
	TGTextEntry        *fTEntries[16][16];
	TGTextBuffer *tbuf[16][16];
   	

public:
   LogicMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
              UInt_t options = kHorizontalFrame);

   virtual ~LogicMenu();

	
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
   void DoSetVal();
   void DoCheckBox();
   void SetButton();
   void SwitchOffButton();
      
};