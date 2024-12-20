#include "MTCParams.h"
#include "MTCFunc.h"

//extern int N_CH;
extern int handle[2];
extern ReadoutConfig_t Rcfg;
extern DigitizerConfig_t Dcfg[2];


ParamsMenu::ParamsMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, int N_CH, UInt_t options)
{		
Nch = N_CH;
char titlelabel[10], str[10];
int j;

const char *paramlabel[] = {"Enable", "InputRange","Polarity", "RecordLength", "Pretrigger", "DCOffset", "thr", "nsbl", "lgate", "sgate", "pgate","selft", "trgc", "discr", "cfdd", "cfdf", "tvaw", "csens"};		
int iStyle[]	= {0, 2}; 
double Lmin[]	= {-10000, -50, -50,-50}; 	
double Lmax[]	= {10000, 10000, 10000,10000}; 	

fMain = new TGTransientFrame(p, main, w, h, options);
fMain->Connect("CloseWindow()", "ParamsMenu", this, "CloseWindow()");
fMain->DontCallClose(); // to avoid double deletions.
fMain->SetCleanup(kDeepCleanup);
fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame);

	
fTab = new TGTab(fMain, 100, 300);
fTab->Connect("Selected(Int_t)", "ParamsMenu", this, "DoTab()");

for (int b = 0; b<Rcfg.N_B; b++){
	fCF = fTab->AddTab( Rcfg.fInit ? Form("B[%i] : %i",b, Dcfg[b].SerialNumber) : Form("B[%i]", b) );
	f1[b] = new TGCompositeFrame(fCF, 400, 200, kHorizontalFrame); //| kFixedWidth
		
	fVF0[b] = new TGVerticalFrame(f1[b], 200, 300);
	f1[b]->AddFrame(fVF0[b], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	fCAll[b] = new TGCheckButton(fVF0[b], "all in once", 666);	
	fVF0[b]->AddFrame(fCAll[b], new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 0));
	
	for (j = 0; j < 18; j++) { 
	
		fLabel[b][j] = new TGLabel(fVF0[b], paramlabel[j]);
		fLabel[b][j]->SetTextFont(labelFont);
		//all of that if's for correct lines allignment
		if (j == 0)
			fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 2)); // left right top bottom
		if ( j > 0 && j < 4)
			fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 3)); // 0 0 3 4 
		if (j > 3 && j < 8)
			fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 4)); // 0 0 3 4 
		if (j > 7 && j < 11)
			fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 4, 4)); // 0 0 3 4 
		if (j > 10)
			fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 4)); // 0 0 3 4 
	}				
	
	fSetButton[b] = new TGTextButton(fVF0[b], "&Apply", b);
    fSetButton[b]->SetFont(sFont); 
    fSetButton[b]->Resize(60, 30);
    fSetButton[b]->Connect("Clicked()","ParamsMenu", this,"SetButton( )");

    fVF0[b]->AddFrame(fSetButton[b], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 4, 4, 4, 4));	
	fVF0[b]->Resize();
	
	for (int i = 0; i<Nch; i++){
		sprintf(titlelabel, "CH%i", i);	
		fGF[b][i] = new TGGroupFrame(f1[b], titlelabel, kVerticalFrame);
		fGF[b][i]->SetTitlePos(TGGroupFrame::kCenter); 
		f1[b]->AddFrame(fGF[b][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
		
			
   		fVF[b][i] = new TGVerticalFrame(fGF[b][i], 8, 30);
		fGF[b][i]->AddFrame(fVF[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));	
			
		// enable channels checkboxies	
		fC[b][i] = new TGCheckButton(fVF[b][i], "", i + 500*b);	
		(Dcfg[b].ChannelMask & (1<<i)) ? fC[b][i]->SetState(kButtonDown) : fC[b][i]->SetState(kButtonUp); 
		fC[b][i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
		fVF[b][i]->AddFrame(fC[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

		// InputRange comboboxies
		fCIRange[b][i] = new TGComboBox(fVF[b][i], i + 500*b); 	
		fCIRange[b][i]->AddEntry("2 Vpp", 0);
		fCIRange[b][i]->AddEntry("0.5 Vpp", 1);
		Dcfg[b].InputRange[i] == InputRange_2Vpp ? fCIRange[b][i]->Select(0) : fCIRange[b][i]->Select(1);
		fCIRange[b][i]->Resize(50, 20);
		fCIRange[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[b][i]->AddFrame(fCIRange[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

		// polarity comboboxies
		fCPol[b][i] = new TGComboBox(fVF[b][i], i + 100 + 500*b);	
		fCPol[b][i]->AddEntry("Negative", 0);
		fCPol[b][i]->AddEntry("Positive", 1);
		Dcfg[b].PulsePolarity[i] == CAEN_DGTZ_PulsePolarityNegative? fCPol[b][i]->Select(0) : fCPol[b][i]->Select(1);
		fCPol[b][i]->Resize(50, 20);
		fCPol[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[b][i]->AddFrame(fCPol[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
			
		// RecordLength, pretrigger, DCOffset textentries	
		for (j = 0; j < 4; j++) { 
			
			tbuf[b][i][j] = new TGTextBuffer(10);
			
			switch (j){
				case 0 :
					sprintf(str, "%i", Dcfg[b].RecordLength[i]);	
					break;
				case 1 :
					sprintf(str, "%i", Dcfg[b].PreTrigger[i]);	
					break;	
				case 2 :
					sprintf(str, "%i", Dcfg[b].DCOffset[i]);	
					break;
				case 3 :
					sprintf(str, "%i", Dcfg[b].thr[i]);	
					break;	
			}
			
			
			tbuf[b][i][j]->AddText(0, str);
				
			fTEntries[b][i][j] = new TGTextEntry(fVF[b][i], tbuf[b][i][j], j*16 + i + 500*b); //
			if (j==0)
				fTEntries[b][i][j]->SetEnabled(0); // disabled for better times
			fTEntries[b][i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
			fTEntries[b][i][j]->Resize(50, fTEntries[b][i][j]->GetDefaultHeight());
			fTEntries[b][i][j]->SetFont(paramFont);
			fVF[b][i]->AddFrame(fTEntries[b][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		}
		
		// nsbl comboboxies
		fCnsbl[b][i] = new TGComboBox(fVF[b][i], i + 16 + 500*b);	
		fCnsbl[b][i]->AddEntry("0", 0);
		fCnsbl[b][i]->AddEntry("16", 1);
		fCnsbl[b][i]->AddEntry("64", 2);
		fCnsbl[b][i]->AddEntry("256", 3);
		fCnsbl[b][i]->AddEntry("1024", 4);
		fCnsbl[b][i]->Select(Dcfg[b].nsbl[i]);
		fCnsbl[b][i]->Resize(50, 20);
		fCnsbl[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[b][i]->AddFrame(fCnsbl[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		// lgate, sgate, pgate textentries	
		for (j = 4; j < 7; j++) { 
			tbuf[b][i][j] = new TGTextBuffer(10);
			
			switch (j){
				case 4 :
					sprintf(str, "%i", Dcfg[b].lgate[i]);	
					break;
				case 5 :
					sprintf(str, "%i", Dcfg[b].sgate[i]);	
					break;	
				case 6 :
					sprintf(str, "%i", Dcfg[b].pgate[i]);	
					break;
			}
				
			tbuf[b][i][j]->AddText(0, str);
				
			fTEntries[b][i][j] = new TGTextEntry(fVF[b][i], tbuf[b][i][j], j*16 + i + 500*b); //
			fTEntries[b][i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
			fTEntries[b][i][j]->Resize(50, fTEntries[b][i][j]->GetDefaultHeight());
			fTEntries[b][i][j]->SetFont(paramFont);
			fVF[b][i]->AddFrame(fTEntries[b][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		}
		
		// self triggers checkboxies	
		fCselft[b][i] = new TGCheckButton(fVF[b][i], "", i + 16 + 500*b);	
		Dcfg[b].selft[i] == 0 ? fCselft[b][i]->SetState(kButtonUp) : fCselft[b][i]->SetState(kButtonDown); 
		fCselft[b][i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
		fVF[b][i]->AddFrame(fCselft[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 4, 4));
		
		//trgc Trigger Configuration
		fCtrgc[b][i] = new TGComboBox(fVF[b][i], i + 32 + 500*b);	
		fCtrgc[b][i]->AddEntry("Peak", 0);
		fCtrgc[b][i]->AddEntry("Threshold", 1);
		fCtrgc[b][i]->Resize(50, 20);
		fCtrgc[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fCtrgc[b][i]->SetEnabled(0); // disabled for better times
		fVF[b][i]->AddFrame(fCtrgc[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//discr LED/CFD
		fCdiscr[b][i] = new TGComboBox(fVF[b][i], i + 48 + 500*b);	
		fCdiscr[b][i]->AddEntry("LED", 0);
		fCdiscr[b][i]->AddEntry("CFD", 1);
		fCdiscr[b][i]->Select(Dcfg[b].discr[i]==CAEN_DGTZ_DPP_DISCR_MODE_LED ? 0 : 1);
		fCdiscr[b][i]->Resize(50, 20);
		fCdiscr[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[b][i]->AddFrame(fCdiscr[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//cfdd textentry
		j = 7;
		tbuf[b][i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", Dcfg[b].cfdd[i]);	
		tbuf[b][i][j]->AddText(0, str);
				
		fTEntries[b][i][j] = new TGTextEntry(fVF[b][i], tbuf[b][i][j], j*16 + i + 500*b); //
		fTEntries[b][i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[b][i][j]->Resize(50, fTEntries[b][i][j]->GetDefaultHeight());
		fTEntries[b][i][j]->SetFont(paramFont);
		fVF[b][i]->AddFrame(fTEntries[b][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		//cfdf fraction 0 - 25%, 1 - 50%, 2 - 75%, 3 - 100%
		fCcfdf[b][i] = new TGComboBox(fVF[b][i], i + 64 + 500*b);	
		fCcfdf[b][i]->AddEntry("25%", 0);
		fCcfdf[b][i]->AddEntry("50%", 1);
		fCcfdf[b][i]->AddEntry("75%", 2);
		fCcfdf[b][i]->AddEntry("100%", 3);
		fCcfdf[b][i]->Select(Dcfg[b].cfdf[i]);
		fCcfdf[b][i]->Resize(50, 20);
		fCcfdf[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[b][i]->AddFrame(fCcfdf[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//tvaw textentry
		j = 8;
		tbuf[b][i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", Dcfg[b].tvaw[i]);	
		tbuf[b][i][j]->AddText(0, str);
				
		fTEntries[b][i][j] = new TGTextEntry(fVF[b][i], tbuf[b][i][j], j*16 + i + 500*b); //
		fTEntries[b][i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[b][i][j]->Resize(50, fTEntries[b][i][j]->GetDefaultHeight());
		fTEntries[b][i][j]->SetFont(paramFont);
		fTEntries[b][i][j]->SetEnabled(0); // disabled for better times
		fVF[b][i]->AddFrame(fTEntries[b][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		//csens charge sensibility
		fCcsens[b][i] = new TGComboBox(fVF[b][i], i + 80 + 500*b);	
		fCcsens[b][i]->AddEntry("5fC/LSB", 0);
		fCcsens[b][i]->AddEntry("20fC/LSB", 1);
		fCcsens[b][i]->AddEntry("80fC/LSB", 2);
		fCcsens[b][i]->AddEntry("320fC/LSB", 3);
		fCcsens[b][i]->AddEntry("1.28pC/LSB", 4);
		fCcsens[b][i]->AddEntry("5.12pC/LSB", 5);
		fCcsens[b][i]->Select(Dcfg[b].csens[i]);
		fCcsens[b][i]->Resize(50, 20);
		fCcsens[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[b][i]->AddFrame(fCcsens[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		fVF[b][i]->Resize();
		fGF[b][i]->Resize();	
	}// boards loop
	
	fCF->AddFrame(f1[b], new TGLayoutHints(kLHintsTop | kLHintsRight, 5, 5, 10, 5) );
	fCF->Resize( );	
}
	///////////////////////////////////////////
	////Check Addresses Tab
	///////////////////////////////////////////
	fCF = fTab->AddTab("Addresses");
			
	TGHorizontalFrame *hf_address = new TGHorizontalFrame(fCF, 200, 40);
	
	TGLabel *fBLabel = new TGLabel(hf_address, "Board");
	fBLabel->SetTextFont(labelFont);
	hf_address->AddFrame(fBLabel, new TGLayoutHints(kLHintsCenterX, 5, 0, 3, 3)); // left right top bottom

	fBoardNE = new TGNumberEntry(hf_address, 0, 8, 1000, (TGNumberFormat::EStyle) 0); 
	fBoardNE->Resize(20, fBoardNE->GetDefaultHeight());
	  //fBoardNE->Connect("ValueSet(Long_t)", "ParamsMenu", this, "DoSetVal()");
    hf_address->AddFrame(fBoardNE, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 0, 3, 3));
	
	fAddressTBuf = new TGTextBuffer(10);
	fAddressTBuf->AddText(0, "ADD");
	fAddressTEntry = new TGTextEntry(hf_address, fAddressTBuf, 1001);	
	fAddressTEntry->Connect("ReturnPressed()", "ParamsMenu", this, "DoAddresses()");	
	fAddressTEntry->Resize(50, fAddressTEntry->GetDefaultHeight());
	fAddressTEntry->SetFont(paramFont);
	hf_address->AddFrame(fAddressTEntry, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)

	fAdValTBuf = new TGTextBuffer(10);
	fAdValTBuf->AddText(0, "VAL");
	fAdValTEntry = new TGTextEntry(hf_address, fAdValTBuf, 1002);	
	fAdValTEntry->Connect("ReturnPressed()", "ParamsMenu", this, "DoAddresses()");	
	fAdValTEntry->Resize(100, fAdValTEntry->GetDefaultHeight());
	fAdValTEntry->SetFont(paramFont);
	hf_address->AddFrame(fAdValTEntry, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
	fCF->AddFrame(hf_address, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 5, 10, 0) );

	TGHorizontalFrame *hf_buttons = new TGHorizontalFrame(fCF, 200, 40);

	fReadButton = new TGTextButton(hf_buttons, "&Read", 1);
    fReadButton->SetFont(sFont); 
    fReadButton->Resize(60, 30);
    fReadButton->Connect("Clicked()","ParamsMenu", this,"ReadButton( )");
	hf_buttons->AddFrame(fReadButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)

	fWriteButton = new TGTextButton(hf_buttons, "&Write", 1);
    fWriteButton->SetFont(sFont); 
    fWriteButton->Resize(60, 30);
    fWriteButton->Connect("Clicked()","ParamsMenu", this,"WriteButton( )");
	hf_buttons->AddFrame(fWriteButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)

	fCF->AddFrame(hf_buttons, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 5, 10, 0) );
	
	fCF->Resize( );	

		
	fMain->AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsRight, 5, 5, 10, 5) );
	


	fMain->MapSubwindows();
	fMain->Resize();

	fMain->CenterOnParent();

	fMain->SetWindowName("Digi params");

	fMain->MapWindow();
}



ParamsMenu::~ParamsMenu()
{

   fMain->DeleteWindow();  // deletes fMain
}

void ParamsMenu::CloseWindow()
{
   delete this;
}

void ParamsMenu::SetButton( )
{	
	TGTextButton *tb = (TGTextButton *) gTQSender;	
	int b = tb->WidgetId( );
	printf(" button id %i B[%i] \n", b, b);

    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		

	for (int i = 0; i < Nch; i++){
		//ChannelMask
		ret = CAEN_DGTZ_WriteRegister(handle[b], EnableMaskAddress, Dcfg[b].ChannelMask);	
		//InputRange
		ret = SetInputRange(handle[b], i, Dcfg[b].InputRange[i]);	
		//Polarity
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle[b], i, (CAEN_DGTZ_PulsePolarity_t)Dcfg[b].PulsePolarity[i]);
		//RecordLength
		ret = CAEN_DGTZ_SetRecordLength(handle[b], Dcfg[b].RecordLength[i], i);
		//PreTrigger
		ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle[b], i, Dcfg[b].PreTrigger[i]);
		//DCOffset
		ret = CAEN_DGTZ_SetChannelDCOffset(handle[b], i, Dcfg[b].DCOffset[i]); 
		//thr
		ret = CAEN_DGTZ_WriteRegister(handle[b], ThresholdAddress[i], Dcfg[b].thr[i]); 
		//nsbl
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		reg_data = ( reg_data & ~(0x700000) ) | (Dcfg[b].nsbl[i]<<20); // mask to clean and change b22:b20
		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);	
		//lgate
		ret = CAEN_DGTZ_WriteRegister(handle[b], LongGateWidthAddress[i], Dcfg[b].lgate[i]); 
		//sgate
		ret = CAEN_DGTZ_WriteRegister(handle[b], ShortGateWidthAddress[i], Dcfg[b].sgate[i]);
		//pgate
		ret = CAEN_DGTZ_WriteRegister(handle[b], GateOffsetAddress[i], Dcfg[b].pgate[i]); 
		//self
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		printf(" Previously in 0x%04X val %i \n", DPPAlgControlAddress[i], Dcfg[b].selft[i]);
		Dcfg[b].selft[i] ==0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); //[b24] responsible for self trigger
		printf(" Self [%i] val %i \n", i, Dcfg[b].selft[i]); // 1 - Enabled,  0 - Disabled
		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);	// 1 - Disabled, 0 - Enabled
		//discr
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		Dcfg[b].discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_CFD ? reg_data |=  (1<<6) : reg_data &=  ~(1<<6);
		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);	
		//cfdd
		ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[i], &reg_data);
		reg_data = (reg_data &~ (0x0F) ) + Dcfg[b].cfdd[i];
		ret = CAEN_DGTZ_WriteRegister(handle[b], CFDSettingsAddress[i], reg_data);
		//cfdf
		ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[i], &reg_data);
		reg_data = (reg_data &~ (0xF00 ) ) | ( Dcfg[b].cfdf[i] << 8);
		ret = CAEN_DGTZ_WriteRegister(handle[b], CFDSettingsAddress[i], reg_data);
		//tvaw
		ret = CAEN_DGTZ_WriteRegister(handle[b], ShapedTriggerWidthAddress[i], Dcfg[b].tvaw[i]);	 // 0x14 = 20x8ns = 160 ns window | 0x40 = 64x8ns = 512 ns window
		//csens
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		reg_data = (reg_data &~ (0x00000007 ) ) | ( Dcfg[b].csens[i] << 0); // clean up [b2:0]
		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);  
	}


	printf("Settings applied ret = %i\n", ret);
}

void ParamsMenu::DoClose()
{
    //Handle Close button.
	
   CloseWindow();
}

void ParamsMenu::DoSetVal()
{	
	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		
	TGTextEntry *te = (TGTextEntry *) gTQSender;
	Int_t id = te->WidgetId();
	printf("textentry changed %i \n", id);

	int b = id < 500 ? 0 : 1;
	id = id >= 500 ? id-500 : id;

	Int_t i = id%16;
	Int_t j = id  / 16;
	char str[20];
	
	fCAll[b]->GetState()== kButtonDown ? fAll = true : fAll = false;
	//printf("smth changed in %i i = %i j = %i, new val = %s num : %i \n", id, i, j, tbuf[i][j]->GetString(), atoi( tbuf[i][j]->GetString() ) );
	
	//RecordLength
	if (id <16){
		Dcfg[b].RecordLength[i] = atoi( tbuf[b][i][j]->GetString() );
		ret = CAEN_DGTZ_SetRecordLength(handle[b], Dcfg[b].RecordLength[i], i);
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].RecordLength[n] = Dcfg[b].RecordLength[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					ret = CAEN_DGTZ_SetRecordLength(handle[b], Dcfg[b].RecordLength[n], n);
				}
			}
		}
	}
	
	//pretrigger
	if (id >=16 && id <32){
		Dcfg[b].PreTrigger[i] = atoi( tbuf[b][i][j]->GetString() );
		ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle[b], i, Dcfg[b].PreTrigger[i]);  //30
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].PreTrigger[n] = Dcfg[b].PreTrigger[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle[b], n, Dcfg[b].PreTrigger[n]);
				}
			}
		}
	}
	
	// DCOffset
	if (id >=32 && id <48){
		Dcfg[b].DCOffset[i] = atoi( tbuf[b][i][j]->GetString() );
		ret = CAEN_DGTZ_SetChannelDCOffset(handle[b], i, Dcfg[b].DCOffset[i]); //0x8000
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].DCOffset[n] = Dcfg[b].DCOffset[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					ret = CAEN_DGTZ_SetChannelDCOffset(handle[b], n, Dcfg[b].DCOffset[n]); //0x8000
				}
			}
		}
	}
	
	// thresholds
	if (id >=48 && id <64){
		Dcfg[b].thr[i] = atoi( tbuf[b][i][j]->GetString() );
		printf(" thr[%i] set to : %i \n", i, Dcfg[b].thr[i]);	
		//ret = CAEN_DGTZ_ReadRegister(handle[b], 0x1080, &reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle[b], ThresholdAddress[i], Dcfg[b].thr[i]);
		ret = CAEN_DGTZ_ReadRegister(handle[b], ThresholdAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n", ThresholdAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].thr[n] = Dcfg[b].thr[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle[b], ThresholdAddress[n], Dcfg[b].thr[n]);
					ret = CAEN_DGTZ_ReadRegister(handle[b], ThresholdAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", ThresholdAddress[n], reg_data);	
				}
			}
		}
		
	}
		
	// Long Gate [samples]
	if (id >=64 && id <80){
		Dcfg[b].lgate[i] = atoi( tbuf[b][i][j]->GetString( ) );
		printf(" lgate[%i] set to : %i \n", i, Dcfg[b].lgate[i]);	
		ret = CAEN_DGTZ_WriteRegister(handle[b], LongGateWidthAddress[i], Dcfg[b].lgate[i]);
		ret = CAEN_DGTZ_ReadRegister(handle[b], LongGateWidthAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n", LongGateWidthAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].lgate[n] = Dcfg[b].lgate[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle[b], LongGateWidthAddress[n], Dcfg[b].lgate[n]);
					ret = CAEN_DGTZ_ReadRegister(handle[b], LongGateWidthAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", LongGateWidthAddress[n], reg_data);	
				}
			}
		}
	}	
		
	// Short Gate [samples]
	if (id >=80 && id <96){
		Dcfg[b].sgate[i] = atoi( tbuf[b][i][j]->GetString() );
		printf(" sgate[%i] set to : %i \n", i, Dcfg[b].sgate[i]);	
		ret = CAEN_DGTZ_WriteRegister(handle[b], ShortGateWidthAddress[i], Dcfg[b].sgate[i]);
		ret = CAEN_DGTZ_ReadRegister(handle[b], ShortGateWidthAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n", ShortGateWidthAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].sgate[n] = Dcfg[b].sgate[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle[b], ShortGateWidthAddress[n], Dcfg[b].sgate[n]);
					ret = CAEN_DGTZ_ReadRegister(handle[b], ShortGateWidthAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", ShortGateWidthAddress[n], reg_data);	
				}
			}
		}
	}	
	
	// Gate Offset (pregate)[samples]
	if (id >=96 && id <112){
		Dcfg[b].pgate[i] = atoi( tbuf[b][i][j]->GetString() );
		printf(" pgate[%i] set to : %i \n", i, Dcfg[b].pgate[i]);	
		ret = CAEN_DGTZ_WriteRegister(handle[b], GateOffsetAddress[i], Dcfg[b].pgate[i]);
		ret = CAEN_DGTZ_ReadRegister(handle[b], GateOffsetAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n",GateOffsetAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].pgate[n] = Dcfg[b].pgate[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle[b], GateOffsetAddress[n], Dcfg[b].pgate[n]);
					ret = CAEN_DGTZ_ReadRegister(handle[b], GateOffsetAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", GateOffsetAddress[n], reg_data);	
				}
			}
		}
	}
	// CFD delay [samples]
	if (id >=112 && id <128){
		Dcfg[b].cfdd[i] = atoi( tbuf[b][i][j]->GetString() );
		ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[i], &reg_data);
		printf(" Previously in  0x%04X: %08X \n", CFDSettingsAddress[i], reg_data);
		reg_data = (reg_data &~ (0x0F) ) + Dcfg[b].cfdd[i];
		ret = CAEN_DGTZ_WriteRegister(handle[b], CFDSettingsAddress[i], reg_data);
		ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[i], &reg_data);
		printf(" In  0x%04X 0x%08X \n", CFDSettingsAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if (n != i){
					Dcfg[b].cfdd[n] = Dcfg[b].cfdd[i];
					fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString( ));
					
					ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[n], &reg_data);
					printf(" Previously in  0x%04X: %08X \n", CFDSettingsAddress[n], reg_data);
					reg_data = (reg_data &~ (0x0F) ) + Dcfg[b].cfdd[n];
					ret = CAEN_DGTZ_WriteRegister(handle[b], CFDSettingsAddress[n], reg_data);
					ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[n], &reg_data);
					printf(" In  0x%04X 0x%08X \n", CFDSettingsAddress[n], reg_data);	
				}
			}
		}
	}

	//if (id >=128 && id <144)
	//	Dcfg[b].tvaw[i] = atoi( tbuf[i][j]->GetString() );
	
}

void ParamsMenu::DoCheckBox()
{		
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	printf("checkbox changed %i \n", id);
	int b = id < 500 ? 0 : 1;
	id = id >= 500 ? id-500 : id;

	fCAll[b]->GetState()== kButtonDown ? fAll = true : fAll = false;
	
	//enabled channels mask
	if (id <16){
		
		fC[b][id]->GetState()== kButtonDown ? Dcfg[b].ChannelMask  |=  (1<<id) : Dcfg[b].ChannelMask  &= ~ (1<<id);
		ret = CAEN_DGTZ_ReadRegister(handle[b], EnableMaskAddress, &reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle[b], EnableMaskAddress, Dcfg[b].ChannelMask);	
				
		if (fAll){
			for (int i = 0; i < Nch; i++){ 
				if ( i!=id  ){
					fC[b][i]->SetState( fC[b][id]->GetState( ) );
					fC[b][i]->GetState( )== kButtonDown ? Dcfg[b].ChannelMask  |=  (1<<i) : Dcfg[b].ChannelMask  &= ~ (1<<i);
				}	
			}
			
			ret = CAEN_DGTZ_ReadRegister(handle[b], EnableMaskAddress, &reg_data);
			ret = CAEN_DGTZ_WriteRegister(handle[b], EnableMaskAddress, Dcfg[b].ChannelMask);	
		}
	}
	
	//self triggers
	if (id >=16 && id <32 ){
		fCselft[b][id-16]->GetState()== kButtonDown ? Dcfg[b].selft[id-16] = 1 : Dcfg[b].selft[id-16] = 0;
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[id-16], &reg_data);
		Dcfg[b].selft[id-16] ==0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); //[b24] responsible for self trigger
 		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[id-16], reg_data);	
				
		if (fAll){
			for (int i = 0; i < Nch; i++){ 
				if ( i!=(id-16) ){
					fCselft[b][i]->SetState( fCselft[b][id-16]->GetState( ) );
					fCselft[b][i]->GetState()== kButtonDown ? Dcfg[b].selft[i] = 1 : Dcfg[b].selft[i] = 0;
					
					ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
					Dcfg[b].selft[i] ==0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); //[b24] responsible for self trigger
					ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);	
					
				}	
			}
		}
	}
		
}

void ParamsMenu::DoComboBox()
{	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		
	TGComboBox *cb = (TGComboBox *) gTQSender;
	Int_t id = cb->WidgetId();

	int b = id < 500 ? 0 : 1;
	id = id >= 500 ? id-500 : id;
	Int_t i = id%16;
		
	fCAll[b]->GetState( )== kButtonDown ? fAll = true : fAll = false;		
	//printf("combobox changed %i  \n", id);

	// InputRange
	if (id < 16){
		Dcfg[b].InputRange[i] = fCIRange[b][i]->GetSelected() == 0 ? InputRange_2Vpp : InputRange_0_5Vpp;
		ret = SetInputRange(handle[b], i, Dcfg[b].InputRange[i]);
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if ( n!=i) {
					fCIRange[b][n]->Select(fCIRange[b][i]->GetSelected( ) );
					Dcfg[b].InputRange[n] = Dcfg[b].InputRange[i];
				}
			}
		}	
			
	}

	// polarity 
	if (id >= 100 && id < 116){
		Dcfg[b].PulsePolarity[i]  = fCPol[b][i]->GetSelected() == 0 ?  CAEN_DGTZ_PulsePolarityNegative : CAEN_DGTZ_PulsePolarityPositive;
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle[b], i, (CAEN_DGTZ_PulsePolarity_t)Dcfg[b].PulsePolarity[i]);
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if ( n!=i) {
					fCPol[b][n]->Select(fCPol[b][i]->GetSelected( ) );
					Dcfg[b].PulsePolarity[n]  = Dcfg[b].PulsePolarity[i];
					ret = CAEN_DGTZ_SetChannelPulsePolarity(handle[b], n, Dcfg[b].PulsePolarity[n]); // due to Select function  it can be setted up twice
				}
			}
		}	
	}
	
	// number samples for baseline 
	if (id >=16 && id <32){
		Dcfg[b].nsbl[i] = fCnsbl[b][i]->GetSelected();
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
				
		reg_data = ( reg_data & ~(0x700000) ) | (Dcfg[b].nsbl[i]<<20); // mask to clean and change b22:b20
						
 		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if ( n!=i) {
					fCnsbl[b][n]->Select(fCnsbl[b][i]->GetSelected( ) );
					Dcfg[b].nsbl[n]  = Dcfg[b].nsbl[i];
				}
			}
		}	
			
	}
	
	// discriminator type 
	if (id >=48 && id <64){
		fCdiscr[b][i]->GetSelected( ) == 0 ? Dcfg[b].discr[i] = CAEN_DGTZ_DPP_DISCR_MODE_LED : Dcfg[b].discr[i] = CAEN_DGTZ_DPP_DISCR_MODE_CFD;
		printf("Ch[%i] discr : %s \n", i, fCdiscr[b][i]->GetSelected() == 0 ? "LED" : "CFD" );
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		printf(" Previously in  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		Dcfg[b].discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_CFD ? reg_data |=  (1<<6) : reg_data &=  ~(1<<6);
		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if ( n!=i) {
					fCdiscr[b][n]->Select(fCdiscr[b][i]->GetSelected( ) );
					Dcfg[b].discr[n]  = Dcfg[b].discr[i];
					//not necessary due to strange work of "Select" function
					//ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[n], reg_data);	
					//ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[n], &reg_data);
					//printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[n], reg_data);
				}
			}
		}	
		
	}
	
	//CFD fraction: 0->25%; 1->50%; 2->75%; 3->100% 
		
	if (id >=64 && id <80){
		Dcfg[b].cfdf[i] = fCcfdf[b][i]->GetSelected();
		ret = CAEN_DGTZ_ReadRegister(handle[b], CFDSettingsAddress[i], &reg_data);
		reg_data = (reg_data &~ (0xF00 ) ) | ( Dcfg[b].cfdf[i] << 8);
		ret = CAEN_DGTZ_WriteRegister(handle[b], CFDSettingsAddress[i], reg_data);
				
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if ( n!=i) {
					fCcfdf[b][n]->Select(fCcfdf[b][i]->GetSelected( ) );
					Dcfg[b].cfdf[n]  = Dcfg[b].cfdf[i];
				}
			}
		}	
	}	
	
	// charge sensitivity 0 - 5;  5fC - 5.12pC
	if (id >=80 && id <96){
		Dcfg[b].csens[id-80] = fCcsens[b][id-80]->GetSelected();
		printf("Ch[%i] csens : %i \n", id-80, fCcsens[b][id-80]->GetSelected());
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		printf(" Previously in  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		reg_data = (reg_data &~ (0x00000007 ) ) | ( Dcfg[b].csens[i] << 0); // clean up [b2:0]
		ret = CAEN_DGTZ_WriteRegister(handle[b], DPPAlgControlAddress[i], reg_data);
		ret = CAEN_DGTZ_ReadRegister(handle[b], DPPAlgControlAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		
		
		if (fAll){
			for (int n = 0; n < Nch; n++){ 
				if ( n!=i) {
					fCcsens[b][n]->Select(fCcsens[b][i]->GetSelected( ) );
					Dcfg[b].csens[n]  = Dcfg[b].csens[i];
				}
			}
		}
	}	
			
}


void ParamsMenu::DoTab( ){
	TGTab *tab = (TGTab *) gTQSender;
	printf("Active Tab #%i\n", tab->GetCurrent( ) );
}

void ParamsMenu::ReadButton( )
{	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		

	printf("Read Button  \n");

	char str[10];
	int b = (int)fBoardNE->GetNumber( );
	address = strtol( fAddressTBuf->GetString( ), 0, 16);
	ret = CAEN_DGTZ_ReadRegister(handle[b], address, &reg_data);
	printf("B[%i] ADD %s %04X  VAL %04X\n", b, fAddressTBuf->GetString( ), address, reg_data);
	
	sprintf(str, "%04X", reg_data);
	fAdValTEntry->SetText(str);	

	if(ret)
		printf("Error in Read Button  ret = %i\n", ret);

}


void ParamsMenu::WriteButton( )
{	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		

	printf("Write Button  \n");

	char str[10];
	int b = (int)fBoardNE->GetNumber( );
	address = strtol( fAddressTBuf->GetString( ), 0, 16);
	reg_data = strtol( fAdValTBuf->GetString( ), 0, 16 );
	printf("B[%i] ADD %s %04X  VAL %04X\n", b, fAddressTBuf->GetString( ), address, reg_data);
	
	ret = CAEN_DGTZ_WriteRegister(handle[b], address, reg_data);
	
	//sprintf(str, "%04X", reg_data);
	//fAdValTEntry->SetText(str);	

	if(ret)
		printf("Error in Write Button  ret = %i\n", ret);

}

void ParamsMenu::DoAddresses()
{
   TGTextEntry *te = (TGTextEntry *) gTQSender;
	Int_t id = te->WidgetId();
	
	//char str[20];
		
	printf("smth changed in %i \n", id );
	printf(" new val = %s num : %i \n", fAdValTBuf->GetString(), atoi( fAdValTBuf->GetString() ) );
	printf(" new val = %s num : %i \n", fAddressTBuf->GetString(), atoi( fAddressTBuf->GetString() ) );
  
}

void ParamsMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}
