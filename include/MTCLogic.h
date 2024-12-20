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

#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>

class LogicMenu {
	
RQ_OBJECT("LogicMenu")

private:
   TGTransientFrame     *fMain;
   TGCompositeFrame     *f1, *f2;
   
   TGTextButton      *fSetButton, *fSwitchOffButton;
   
	
   TGGroupFrame *fGF[2][8];	
	TGHorizontalFrame    *fHF0couple, *fHF0board, *fHFcouple[8], *fHFboard[8];
	
	
	TGCheckButton *fCcouple[8][8], *fCboard[8][14], *fCTrgIn[8];	
		
   TGLabel              *fLabel[2][14];
   
   TGNumberEntry        *fNumericEntries[16][16];
   
	TGTextEntry        *fTEntries[16][16];
	TGTextBuffer *tbuf[16][16];
   	

public:
   LogicMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
              UInt_t options = kHorizontalFrame);

   virtual ~LogicMenu();

	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";
	
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
   void DoSetVal();
   void DoCheckBox();
   void SetButton();
   void SwitchOffButton();
      
};