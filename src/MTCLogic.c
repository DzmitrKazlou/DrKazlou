#include "MTCLogic.h"
#include "MTCFunc.h"

extern ReadoutConfig_t Rcfg;
extern DigitizerConfig_t Dcfg[2];
extern uint32_t log_val[2][2][8];
extern int handle[2];
	
	
//////////////////////////////////
////LOGIC_MENU
//////////////////////////////////


LogicMenu::LogicMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, int N_CH, UInt_t options)
{
Nch = N_CH;	
printf("N_CH %i \n", N_CH);
char titlelabel[10], str[20];
int bit[2] = {7, 13}; // bit for coinc logic  in couple | board
	
fMain = new TGTransientFrame(p, main, w, h, options);
fMain->Connect("CloseWindow()", "LogicMenu", this, "CloseWindow()");
fMain->DontCallClose( ); // to avoid double deletions.
fMain->SetCleanup(kDeepCleanup);
fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame); //

fTab = new TGTab(fMain, 100, 300);
fTab->Connect("Selected(Int_t)", "LogicMenu", this, "DoTab()");

for (int b = 0; b<Rcfg.N_B; b++){
	fCF = fTab->AddTab( Rcfg.fInit ? Form("B[%i] : %i",b, Dcfg[b].SerialNumber) : Form("B[%i]", b) );
	vframe[b] = new TGVerticalFrame(fCF, 200, 40);

	hf_main[b] = new TGHorizontalFrame(vframe[b], 200, 40);
	
    fCF_couple[b] = new TGCompositeFrame(hf_main[b], 200, 60, kVerticalFrame); //| kFixedWidth
	fHF0couple[b] = new TGHorizontalFrame(fCF_couple[b], 100, 30);
	fCF_couple[b]->AddFrame(fHF0couple[b], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	fCF_board[b] = new TGCompositeFrame(hf_main[b], 200, 60, kVerticalFrame); //| kFixedWidth
	fHF0board[b] = new TGHorizontalFrame(fCF_board[b], 100, 30);
	fCF_board[b]->AddFrame(fHF0board[b], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	
	for (int i = 0; i < bit[0]; i++) { 
		sprintf(str, "b%i", i);	
		fLabel[b][0][i] = new TGLabel(fHF0couple[b], str);
		fLabel[b][0][i]->SetTextFont(labelFont);
		if (i == 0)
			fHF0couple[b]->AddFrame(fLabel[b][0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 0, 2, 2)); // left right top bottom
		else
			fHF0couple[b]->AddFrame(fLabel[b][0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 1, 1, 2, 2));
	}				
	
	fHF0couple[b]->Resize( );
		
	for (int i = 0; i < bit[1]; i++) { 
		sprintf(str, "b%i", i);	
		fLabel[b][1][i] = new TGLabel(fHF0board[b], str);
		fLabel[b][1][i]->SetTextFont(labelFont);
		if (i == 12)
			fHF0board[b]->AddFrame(fLabel[b][1][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 70, 0, 2, 2)); // left right top bottom
		else
			fHF0board[b]->AddFrame(fLabel[b][1][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 2, 2, 2, 2));
	}				
	
	fHF0board[b]->Resize( );
			
	for (int i = 0; i<(int)(Nch/2); i++){
		sprintf(str, "CH%i-CH%i", 2*i, 2*i+1);	
		fGF[b][0][i] = new TGGroupFrame(fCF_couple[b], str, kVerticalFrame);
		fGF[b][0][i]->SetTitlePos(TGGroupFrame::kLeft); 
		fCF_couple[b]->AddFrame(fGF[b][0][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
		fGF[b][1][i] = new TGGroupFrame(fCF_board[b], str, kVerticalFrame);
		fGF[b][1][i]->SetTitlePos(TGGroupFrame::kLeft); 
		fCF_board[b]->AddFrame(fGF[b][1][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
			
   		fHFcouple[b][i] = new TGHorizontalFrame(fGF[b][0][i], 20, 5);
		fHFboard[b][i] = new TGHorizontalFrame(fGF[b][1][i], 20, 5);
		fGF[b][0][i]->AddFrame(fHFcouple[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
		fGF[b][1][i]->AddFrame(fHFboard[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
			
		for (int j=bit[0]-1; j>=0; j--){	
			fCcouple[b][i][j] = new TGCheckButton(fHFcouple[b][i], "", i * bit[0] + j + 500 * b);	
			if (log_val[b][0][i] & (1<<j))
				fCcouple[b][i][j]->SetState(kButtonDown); 
			else
				fCcouple[b][i][j]->SetState(kButtonUp); 
			//fCcouple[i][j]->GetState( ) == kButtonDown ? log_val[0][i] |= (1<<j) : log_val[0][i] &= ~(1<<j); 
			fCcouple[b][i][j]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
			fHFcouple[b][i]->AddFrame(fCcouple[b][i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 4, 4));
		}
				
		sprintf(str, "%02X", log_val[b][0][i]);	
						
		fTEntries[b][0][i] = new TGTextEntry(fHFcouple[b][i], str, i + 500 * b); 
		fTEntries[b][0][i]->Connect("ReturnPressed()", "LogicMenu", this, "DoSetVal()");	
		fTEntries[b][0][i]->Resize(40, fTEntries[b][0][i]->GetDefaultHeight( ) );
		fTEntries[b][0][i]->SetFont(paramFont);
		fHFcouple[b][i]->AddFrame(fTEntries[b][0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY,  5, 0, 3, 3) ); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		fCTrgIn[b][i] = new TGCheckButton(fHFboard[b][i], "TRG-IN", i + 200 + 500 * b);	
		if (log_val[b][1][i] & (1<<30))
				fCTrgIn[b][i]->SetState(kButtonDown); 
			else
				fCTrgIn[b][i]->SetState(kButtonUp); 
		fCTrgIn[b][i]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
		fHFboard[b][i]->AddFrame(fCTrgIn[b][i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
		
		for (int j=bit[1]-1; j>=0; j--){	
			fCboard[b][i][j] = new TGCheckButton(fHFboard[b][i], "", i * bit[1] + j+100 + 500 * b);	
			if (log_val[b][1][i] & (1<<j))
				fCboard[b][i][j]->SetState(kButtonDown); 
			else
				fCboard[b][i][j]->SetState(kButtonUp); 
			//fCboard[i][j]->GetState( ) == kButtonDown ? log_val[1][i] |= (1<<j) : log_val[1][i] &= ~(1<<j); 
			fCboard[b][i][j]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
			if (j == 12)
				fHFboard[b][i]->AddFrame(fCboard[b][i][j], new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 6, 4, 4));
			else if (j == 11 || j == 10)
				fHFboard[b][i]->AddFrame(fCboard[b][i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 8, 4, 4));
			else 
				fHFboard[b][i]->AddFrame(fCboard[b][i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 3, 3, 4, 4));
		}
		
		sprintf(str, "%04X", log_val[b][1][i]);	
		fTEntries[b][1][i] = new TGTextEntry(fHFboard[b][i], str, i + 10 + 500 * b); // tbuf[1][i]
		fTEntries[b][1][i]->Connect("ReturnPressed()", "LogicMenu", this, "DoSetVal()");	
		fTEntries[b][1][i]->Resize(50, fTEntries[b][1][i]->GetDefaultHeight());
		fTEntries[b][1][i]->SetFont(paramFont);
		fHFboard[b][i]->AddFrame(fTEntries[b][1][i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
				
		fHFcouple[b][i]->Resize( );
		fHFboard[b][i]->Resize( );
		fGF[b][0][i]->Resize( );	
		fGF[b][1][i]->Resize( );	
	}
	
	fCF_couple[b]->Resize( );
	fCF_board[b]->Resize( );
	
	hf_main[b]->AddFrame(fCF_couple[b], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) );
	hf_main[b]->AddFrame(fCF_board[b], new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2) );
	hf_main[b]->Resize();
	vframe[b]->AddFrame(hf_main[b], new TGLayoutHints(kLHintsTop, 2, 2 , 2, 2) );
	
	hf_add[b] = new TGHorizontalFrame(vframe[b], 450, 60); 
 	cInfo[b] = new TGLabel(hf_add[b], "[b1:b0]  00|01|10|11 \n  AND|EVEN|ODD|OR \n[b2] Enable [b1:b0] \n[b5:b4] 01: val0=val1=mb signal \n 10: val0=val1=trg0 AND trg1 \n[b6] Enable [b5:b4]");
	hf_add[b]->AddFrame(cInfo[b], new TGLayoutHints(kLHintsLeft | kLHintsCenterY , 2, 102, 2, 2)); //|  kLHintsExpandX
	
	fSetButton[b] = new TGTextButton(hf_add[b]," S&et ", b);
	fSetButton[b]->SetFont(sFont);
    fSetButton[b]->Resize(50, 30);
	fSetButton[b]->Connect("Clicked()","LogicMenu", this, "SetButton()");
  	hf_add[b]->AddFrame(fSetButton[b], new TGLayoutHints(kLHintsCenterY, 4, 4, 4, 4));

	fSwitchOffButton[b] = new TGTextButton(hf_add[b],"  OFF  ", b);
    fSwitchOffButton[b]->SetFont(sFont);
    fSwitchOffButton[b]->Resize(50, 30);
	fSwitchOffButton[b]->Connect("Clicked()","LogicMenu",this,"SwitchOffButton()");	
    hf_add[b]->AddFrame(fSwitchOffButton[b], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));
	
	bInfo[b] = new TGLabel(hf_add[b], "[b7:b0] trigger from couples \n[b9:b8] 00|01|10 OR|AND|MAJ \n[b12:b10] MAJ \n\t 01 - at least two couples");
	hf_add[b]->AddFrame(bInfo[b], new TGLayoutHints(kLHintsLeft | kLHintsCenterY |  kLHintsExpandX, 42, 2, 2, 2));
		
	hf_add[b]->Resize();
	vframe[b]->AddFrame(hf_add[b], new TGLayoutHints(kLHintsBottom | kLHintsCenterX,       2, 2, 2, 2));

	vframe[b]->Resize();
	fCF->AddFrame(vframe[b], new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

}//board loop

fCF->Resize( );	
fMain->AddFrame(fTab, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
//fMain->AddFrame(vframe[b], new TGLayoutHints(kLHintsCenterX,       2, 2, 2, 2));
fMain->MapSubwindows( );
fMain->Resize( );
fMain->CenterOnParent();
fMain->SetWindowName("Logical scheme");
fMain->MapWindow( );

}



LogicMenu::~LogicMenu()
{

   fMain->DeleteWindow();  // deletes fMain
}

void LogicMenu::CloseWindow()
{
   

   delete this;
}

void LogicMenu::DoClose()
{
    //Handle Close button.
	
   CloseWindow();
}

void LogicMenu::DoSetVal()
{
	TGTextEntry *te = (TGTextEntry *) gTQSender;
	Int_t id = te->WidgetId();
	int b = id < 500 ? 0 : 1;
	id = id < 500 ? id : (id-500);
	Int_t i, j;
	int bit[2] = {7, 13};
		
	if (id < 10){
		i = 0;
		j = id;	
	}
	else{
		i = 1;
		j = id -10;	
	}
		
	log_val[b][i][j] = strtol( fTEntries[b][i][j]->GetText( ), 0, 16);
	
	if (log_val[b][0][j] > 0xFF ){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", "This value can`t be \n more then 0xFF \n", kMBIconStop, kMBOk);
		fTEntries[b][0][j]->SetText("0");
		log_val[b][0][j] = 0;
	}	
	if (log_val[b][1][j] > 0xFFF ){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", "This value can`t be \n more then 0xFFF \n", kMBIconStop, kMBOk);
		fTEntries[b][1][j]->SetText("0");
		log_val[b][1][j] = 0;
	}
		
	
	printf("smth changed in %i i = %i j = %i, new val = %s num : %04lX \n", id, i, j, fTEntries[b][i][j]->GetText( ), strtol( fTEntries[b][i][j]->GetText( ), 0, 16) ); 
	printf("in memory %04X \n", log_val[b][i][j] ); 
	
	if (i ==0)
		for (int k=bit[i]-1; k>=0; k--){	
			if (log_val[b][i][j] & (1<<k))
				fCcouple[b][j][k]->SetState(kButtonDown); 
			else
				fCcouple[b][j][k]->SetState(kButtonUp); 
		}	
	else	
		for (int k=bit[i]-1; k>=0; k--){
			if (log_val[b][i][j] & (1<<k))
				fCboard[b][j][k]->SetState(kButtonDown); 
			else
				fCboard[b][j][k]->SetState(kButtonUp); 
		}
	
}


void LogicMenu::DoCheckBox()
{	
	//int b = 0;
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	int b = id < 500 ? 0 : 1;
	id = id < 500 ? id : (id-500);
	Int_t i, j, n, bit[2] = {7, 13};
	char str[10];
			
	if (id < 100){
		i  = id / bit[0];
		j = id  % bit[0];
		
		fCcouple[b][i][j]->GetState( ) == kButtonDown ? log_val[b][0][i] |= (1<<j) : log_val[b][0][i] &= ~(1<<j); 		
		sprintf(str, "%02X", log_val[b][0][i]);	
		printf("%04X \n", log_val[b][0][i]);	
		fTEntries[b][0][i]->SetText(str);
	}
	
	if (id >= 100 && id < 200){
		i = (id - 100) / bit[1];
		j = (id - 100)  % bit[1];
				
		fCboard[b][i][j]->GetState( ) == kButtonDown ? log_val[b][1][i] |= (1<<j) : log_val[b][1][i] &= ~(1<<j); 		
		sprintf(str, "%04X", log_val[b][1][i]);	
		printf("%04X \n", log_val[b][1][i]);	
		fTEntries[b][1][i]->SetText(str);
	}
	
	if (id >= 200){
		i = (id - 200);
		j = 30;
				
		fCTrgIn[b][i]->GetState( ) == kButtonDown ? log_val[b][1][i] |= (1<<j) : log_val[b][1][i] &= ~(1<<j); 		
		sprintf(str, "%08X", log_val[b][1][i]);	
		printf("%08X \n", log_val[b][1][i]);	
		fTEntries[b][1][i]->SetText(str);
	}
	
	printf("checkbox changed %i i = %i j = %i \n", id, i, j ); 
	
}

void LogicMenu::SetButton( )
{	TGTextButton *tb = (TGTextButton *) gTQSender;
	int b = tb->WidgetId( );

	CAEN_DGTZ_ErrorCode ret  = CAEN_DGTZ_Success;
	ret = SetLogic(handle[b], log_val[b], Nch);
    	
  printf("SetButton B[%i] Nch: %i ret = %i\n", b, Nch, ret); 
}


void LogicMenu::DoTab( ){
	TGTab *tab = (TGTab *) gTQSender;
	printf("Active Tab #%i\n", tab->GetCurrent( ) );
}

void LogicMenu::SwitchOffButton( )
{
    //switch off coincidence on board
	TGTextButton *tb = (TGTextButton *) gTQSender;
	int b = tb->WidgetId( );
	
	CAEN_DGTZ_ErrorCode ret  = CAEN_DGTZ_Success;
	ret = SwitchOffLogic(handle[b], Nch);
	
	printf("Coincidence OFF B[%i] Nch: %i ret = %i\n", b, Nch, ret); 
}


void LogicMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName( ) );
}