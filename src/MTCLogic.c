#include "MTCLogic.h"
#include "MTCFunc.h"

extern int N_CH;
extern uint32_t log_val[2][8];
extern int handle;
extern CAEN_DGTZ_ErrorCode ret;

	
	
//////////////////////////////////
////LOGIC_MENU
//////////////////////////////////


LogicMenu::LogicMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, UInt_t options)
{
		
	char titlelabel[10], str[20];
	int b[2] = {7, 13}; // bit for coinc logic  in couple | board
	
   fMain = new TGTransientFrame(p, main, w, h, options);
   fMain->Connect("CloseWindow()", "LogicMenu", this, "CloseWindow()");
   fMain->DontCallClose( ); // to avoid double deletions.

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);


   fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame); //
	
	TGVerticalFrame *vframe1 = new TGVerticalFrame(fMain, 200, 40);
	TGHorizontalFrame *hframe1 = new TGHorizontalFrame(vframe1, 200, 40);
	
    f1 = new TGCompositeFrame(hframe1, 200, 60, kVerticalFrame); //| kFixedWidth
	fHF0couple = new TGHorizontalFrame(f1, 100, 30);
	f1->AddFrame(fHF0couple, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	f2 = new TGCompositeFrame(hframe1, 200, 60, kVerticalFrame); //| kFixedWidth
	fHF0board = new TGHorizontalFrame(f2, 100, 30);
	f2->AddFrame(fHF0board, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	
	for (int i = 0; i < b[0]; i++) { 
		sprintf(str, "b%i", i);	
		fLabel[0][i] = new TGLabel(fHF0couple, str);
		fLabel[0][i]->SetTextFont(labelFont);
		if (i == 0)
			fHF0couple->AddFrame(fLabel[0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 0, 2, 2)); // left right top bottom
		else
			fHF0couple->AddFrame(fLabel[0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 1, 1, 2, 2));
	}				
	
	fHF0couple->Resize();
		
	for (int i = 0; i < b[1]; i++) { 
		sprintf(str, "b%i", i);	
		fLabel[1][i] = new TGLabel(fHF0board, str);
		fLabel[1][i]->SetTextFont(labelFont);
		if (i == 12)
			fHF0board->AddFrame(fLabel[1][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 70, 0, 2, 2)); // left right top bottom
		else
			fHF0board->AddFrame(fLabel[1][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY, 2, 2, 2, 2));
	}				
	
	fHF0board->Resize();
			
	for (int i = 0; i<(int)(N_CH/2); i++){
		sprintf(str, "CH%i-CH%i", 2*i, 2*i+1);	
		fGF[0][i] = new TGGroupFrame(f1, str, kVerticalFrame);
		fGF[0][i]->SetTitlePos(TGGroupFrame::kLeft); 
		f1->AddFrame(fGF[0][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
		fGF[1][i] = new TGGroupFrame(f2, str, kVerticalFrame);
		fGF[1][i]->SetTitlePos(TGGroupFrame::kLeft); 
		f2->AddFrame(fGF[1][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
			
   		fHFcouple[i] = new TGHorizontalFrame(fGF[0][i], 20, 5);
		fHFboard[i] = new TGHorizontalFrame(fGF[1][i], 20, 5);
		fGF[0][i]->AddFrame(fHFcouple[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
		fGF[1][i]->AddFrame(fHFboard[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));	
			
		for (int j=b[0]-1; j>=0; j--){	
			fCcouple[i][j] = new TGCheckButton(fHFcouple[i], "", i * b[0] + j);	
			if (log_val[0][i] & (1<<j))
				fCcouple[i][j]->SetState(kButtonDown); 
			else
				fCcouple[i][j]->SetState(kButtonUp); 
			//fCcouple[i][j]->GetState( ) == kButtonDown ? log_val[0][i] |= (1<<j) : log_val[0][i] &= ~(1<<j); 
			fCcouple[i][j]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
			fHFcouple[i]->AddFrame(fCcouple[i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 4, 4));
		}
				
		sprintf(str, "%02X", log_val[0][i]);	
						
		fTEntries[0][i] = new TGTextEntry(fHFcouple[i], str, i); 
		fTEntries[0][i]->Connect("ReturnPressed()", "LogicMenu", this, "DoSetVal()");	
		fTEntries[0][i]->Resize(40, fTEntries[0][i]->GetDefaultHeight());
		fTEntries[0][i]->SetFont(paramFont);
		fHFcouple[i]->AddFrame(fTEntries[0][i], new TGLayoutHints(kLHintsRight | kLHintsCenterY,  5, 0, 3, 3) ); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		fCTrgIn[i] = new TGCheckButton(fHFboard[i], "TRG-IN", i + 200);	
		if (log_val[1][i] & (1<<30))
				fCTrgIn[i]->SetState(kButtonDown); 
			else
				fCTrgIn[i]->SetState(kButtonUp); 
		fCTrgIn[i]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
		fHFboard[i]->AddFrame(fCTrgIn[i], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
		
		for (int j=b[1]-1; j>=0; j--){	
			fCboard[i][j] = new TGCheckButton(fHFboard[i], "", i * b[1] + j+100);	
			if (log_val[1][i] & (1<<j))
				fCboard[i][j]->SetState(kButtonDown); 
			else
				fCboard[i][j]->SetState(kButtonUp); 
			//fCboard[i][j]->GetState( ) == kButtonDown ? log_val[1][i] |= (1<<j) : log_val[1][i] &= ~(1<<j); 
			fCboard[i][j]->Connect("Clicked()", "LogicMenu", this, "DoCheckBox()");
			if (j == 12)
				fHFboard[i]->AddFrame(fCboard[i][j], new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 6, 4, 4));
			else if (j == 11 || j == 10)
				fHFboard[i]->AddFrame(fCboard[i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 8, 4, 4));
			else 
				fHFboard[i]->AddFrame(fCboard[i][j], new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 3, 3, 4, 4));
		}
		
		sprintf(str, "%04X", log_val[1][i]);	
		fTEntries[1][i] = new TGTextEntry(fHFboard[i], str, i + 10); // tbuf[1][i]
		fTEntries[1][i]->Connect("ReturnPressed()", "LogicMenu", this, "DoSetVal()");	
		fTEntries[1][i]->Resize(50, fTEntries[1][i]->GetDefaultHeight());
		fTEntries[1][i]->SetFont(paramFont);
		fHFboard[i]->AddFrame(fTEntries[1][i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
				
		fHFcouple[i]->Resize( );
		fHFboard[i]->Resize( );
		fGF[0][i]->Resize( );	
		fGF[1][i]->Resize( );	
	}
	
	f1->Resize();
	f2->Resize();
	
	hframe1->AddFrame(f1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2) );
	hframe1->AddFrame(f2, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2) );
	hframe1->Resize();
	vframe1->AddFrame(hframe1, new TGLayoutHints(kLHintsTop, 2, 2 , 2, 2) );
	
	TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fMain, 450, 60); 
 	TGLabel *cInfo = new TGLabel(hframe2, "[b1:b0]  00|01|10|11 \n  AND|EVEN|ODD|OR \n[b2] Enable [b1:b0] \n[b5:b4] 01: val0=val1=mb signal \n 10: val0=val1=trg0 AND trg1 \n[b6] Enable [b5:b4]");
	hframe2->AddFrame(cInfo, new TGLayoutHints(kLHintsLeft | kLHintsCenterY , 2, 102, 2, 2)); //|  kLHintsExpandX
	
	fSetButton = new TGTextButton(hframe2," S&et ", 1);
	fSetButton->SetFont(sFont);
    fSetButton->Resize(50, 30);
	fSetButton->Connect("Clicked()","LogicMenu", this, "SetButton()");
  	hframe2->AddFrame(fSetButton, new TGLayoutHints(kLHintsCenterY, 4, 4, 4, 4));

	fSwitchOffButton = new TGTextButton(hframe2,"  OFF  ", 1);
    fSwitchOffButton->SetFont(sFont);
    fSwitchOffButton->Resize(50, 30);
	fSwitchOffButton->Connect("Clicked()","LogicMenu",this,"SwitchOffButton()");	
    hframe2->AddFrame(fSwitchOffButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 4, 4, 4, 4));
	
	TGLabel *bInfo = new TGLabel(hframe2, "[b7:b0] trigger from couples \n[b9:b8] 00|01|10 OR|AND|MAJ \n[b12:b10] MAJ \n\t 01 - at least two couples");
	hframe2->AddFrame(bInfo, new TGLayoutHints(kLHintsLeft | kLHintsCenterY |  kLHintsExpandX, 42, 2, 2, 2));
		
	hframe2->Resize();
	vframe1->AddFrame(hframe2, new TGLayoutHints(kLHintsBottom | kLHintsCenterX,       2, 2, 2, 2));
	vframe1->Resize();
	
    fMain->AddFrame(vframe1, new TGLayoutHints(kLHintsCenterX,       2, 2, 2, 2));
	

	fMain->MapSubwindows();
	fMain->Resize();

	fMain->CenterOnParent();

	fMain->SetWindowName("Logical scheme");

	fMain->MapWindow();
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
	Int_t i, j;
	int b[2] = {7, 13};
		
	if (id < 10){
		i = 0;
		j = id;	
	}
	else{
		i = 1;
		j = id -10;	
	}
		
	log_val[i][j] = strtol( fTEntries[i][j]->GetText( ), 0, 16);
	
	if (log_val[0][j] > 0xFF ){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", "This value can`t be \n more then 0xFF \n", kMBIconStop, kMBOk);
		fTEntries[0][j]->SetText("0");
		log_val[0][j] = 0;
	}	
	if (log_val[1][j] > 0xFFF ){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", "This value can`t be \n more then 0xFFF \n", kMBIconStop, kMBOk);
		fTEntries[1][j]->SetText("0");
		log_val[1][j] = 0;
	}
		
	
	printf("smth changed in %i i = %i j = %i, new val = %s num : %04lX \n", id, i, j, fTEntries[i][j]->GetText( ), strtol( fTEntries[i][j]->GetText( ), 0, 16) ); 
	printf("in memory %04X \n", log_val[i][j] ); 
	
	if (i ==0)
		for (int k=b[i]-1; k>=0; k--){	
			if (log_val[i][j] & (1<<k))
				fCcouple[j][k]->SetState(kButtonDown); 
			else
				fCcouple[j][k]->SetState(kButtonUp); 
		}	
	else	
		for (int k=b[i]-1; k>=0; k--){
			if (log_val[i][j] & (1<<k))
				fCboard[j][k]->SetState(kButtonDown); 
			else
				fCboard[j][k]->SetState(kButtonUp); 
		}
	
}


void LogicMenu::DoCheckBox()
{
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	Int_t i, j, n, b[2] = {7, 13};
	char str[10];
			
	if (id < 100){
		i  = id / b[0];
		j = id  % b[0];
		
		fCcouple[i][j]->GetState( ) == kButtonDown ? log_val[0][i] |= (1<<j) : log_val[0][i] &= ~(1<<j); 		
		sprintf(str, "%02X", log_val[0][i]);	
		printf("%04X \n", log_val[0][i]);	
		fTEntries[0][i]->SetText(str);
	}
	
	if (id >= 100 && id < 200){
		i = (id - 100) / b[1];
		j = (id - 100)  % b[1];
				
		fCboard[i][j]->GetState( ) == kButtonDown ? log_val[1][i] |= (1<<j) : log_val[1][i] &= ~(1<<j); 		
		sprintf(str, "%04X", log_val[1][i]);	
		printf("%04X \n", log_val[1][i]);	
		fTEntries[1][i]->SetText(str);
	}
	
	if (id >= 200){
		i = (id - 200);
		j = 30;
				
		fCTrgIn[i]->GetState( ) == kButtonDown ? log_val[1][i] |= (1<<j) : log_val[1][i] &= ~(1<<j); 		
		sprintf(str, "%08X", log_val[1][i]);	
		printf("%08X \n", log_val[1][i]);	
		fTEntries[1][i]->SetText(str);
	}
	
	printf("checkbox changed %i i = %i j = %i \n", id, i, j ); 
	
}

void LogicMenu::SetButton()
{
    
  ret = SetLogic(handle, log_val, N_CH);
  
  	
  printf("SetButton \n" ); 
}

void LogicMenu::SwitchOffButton()
{
    //switch off coincidence on board
	
	ret = SwitchOffLogic(handle, N_CH);
	
	printf("Coincidence OFF \n" ); 
}


void LogicMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}