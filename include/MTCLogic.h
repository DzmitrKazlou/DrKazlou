#include <stdlib.h>


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
#include <RQ_OBJECT.h>

#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGFontDialog.h>

class LogicMenu {
	
RQ_OBJECT("LogicMenu")

private:
   TGTransientFrame     *fMain;
   TGCompositeFrame     *fCF_couple[2], *fCF_board[2], *fCF;
   TGTab *fTab;
   TGTextButton      *fSetButton[2], *fSwitchOffButton[2];
   
	
	TGGroupFrame *fGF[2][2][8];	
	TGHorizontalFrame    *fHF0couple[2], *fHF0board[2], *fHFcouple[2][8], *fHFboard[2][8], *hf_main[2], *hf_add[2];
	TGVerticalFrame *vframe[2];
	TGCheckButton *fCcouple[2][8][8], *fCboard[2][8][14], *fCTrgIn[2][8];	
		
	TGLabel	*fLabel[2][2][14], *cInfo[2], *bInfo[2];
   
	TGNumberEntry	*fNumericEntries[2][16][16];
   
	TGTextEntry        *fTEntries[2][16][16];
	TGTextBuffer *tbuf[2][16][16];
   	

public:
   LogicMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, int N_CH,
              UInt_t options = kHorizontalFrame);

   virtual ~LogicMenu();

	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";

	int Nch;

   // slots
	void TryToClose( );
	void CloseWindow( );
	void DoClose( );
	void DoSetVal( );
	void DoCheckBox( );
	void DoTab( );	
	void SetButton( );
	void SwitchOffButton( );
      
};