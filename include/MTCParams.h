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
	
	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";
	
	int th_add[16] = {0x1060, 0x1160, 0x1260, 0x1360, 0x1460, 0x1560, 0x1660, 0x1760, 0x1860, 0x1960, 0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60};
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
   void DoSetVal();
   void DoCheckBox();
   void DoComboBox();
   
};