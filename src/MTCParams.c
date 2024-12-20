#include "MTCParams.h"
#include "MTCFunc.h"

extern int N_CH;
extern int handle;
extern DigitizerConfig_t Dcfg;
extern CAEN_DGTZ_ErrorCode ret;

ParamsMenu::ParamsMenu(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, UInt_t options)
{
		
	char titlelabel[10], str[10];
	int j;

   fMain = new TGTransientFrame(p, main, w, h, options);
   fMain->Connect("CloseWindow()", "ParamsMenu", this, "CloseWindow()");
   fMain->DontCallClose(); // to avoid double deletions.

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);


   fMain->ChangeOptions((fMain->GetOptions() & ~kVerticalFrame) | kHorizontalFrame);

   f1 = new TGCompositeFrame(fMain, 400, 200, kHorizontalFrame); //| kFixedWidth
	
	const char *paramlabel[] = {"Enable", "InputRange", "Polarity", "RecordLength", "Pretrigger", "DCOffset", "thr", "nsbl", "lgate", "sgate", "pgate","selft", "trgc", "discr", "cfdd", "cfdf", "tvaw", "csens"};		
	
	int iStyle[]	= {0, 2}; 
	double Lmin[]	= {-10000, -50, -50,-50}; 	
	double Lmax[]	= {10000, 10000, 10000,10000}; 	
	
	fVF0 = new TGVerticalFrame(f1, 200, 300);
	f1->AddFrame(fVF0, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	fCAll = new TGCheckButton(fVF0, "all in once", 666);	
	fVF0->AddFrame(fCAll, new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 0));
	
	for (j = 0; j < 18; j++) { 
	
		fLabel[j] = new TGLabel(fVF0, paramlabel[j]);
		fLabel[j]->SetTextFont(labelFont);
		if (j == 0)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 2)); // left right top bottom
		if ( j > 0 && j < 4)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 3)); // 0 0 3 4 
		if (j > 3 && j < 8)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 4)); // 0 0 3 4 
		if (j > 7 && j < 11)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 4, 4)); // 0 0 3 4 
		if (j > 10)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 4)); // 0 0 3 4 
	}				
	
	fSetButton = new TGTextButton(fVF0, "&Apply", 1);
    fSetButton->SetFont(sFont); 
    fSetButton->Resize(60, 30);
    fSetButton->Connect("Clicked()","ParamsMenu", this,"SetButton( )");

    fVF0->AddFrame(fSetButton, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 4, 4, 4, 4));	
	fVF0->Resize();
	
	for (int i = 0; i<N_CH; i++){
		sprintf(titlelabel, "CH%i", i);	
		fGF[i] = new TGGroupFrame(f1, titlelabel, kVerticalFrame);
		fGF[i]->SetTitlePos(TGGroupFrame::kCenter); 
		f1->AddFrame(fGF[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));//
		
			
   		fVF[i] = new TGVerticalFrame(fGF[i], 8, 30);
		fGF[i]->AddFrame(fVF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));	
			
		// enable channels checkboxies	
		fC[i] = new TGCheckButton(fVF[i], "", i);	
		(Dcfg.ChannelMask & (1<<i)) ? fC[i]->SetState(kButtonDown) : fC[i]->SetState(kButtonUp); 
		fC[i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
		fVF[i]->AddFrame(fC[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		// InputRange comboboxies
		fCIRange[i] = new TGComboBox(fVF[i], i); 	
		fCIRange[i]->AddEntry("2 Vpp", 0);
		fCIRange[i]->AddEntry("0.5 Vpp", 1);
		
		Dcfg.InputRange[i] == InputRange_2Vpp ? fCIRange[i]->Select(0) : fCIRange[i]->Select(1);
		fCIRange[i]->Resize(50, 20);
		fCIRange[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCIRange[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		// polarity comboboxies
		fCPol[i] = new TGComboBox(fVF[i], i + 100); //i	
		fCPol[i]->AddEntry("Negative", 0);
		fCPol[i]->AddEntry("Positive", 1);
		
		Dcfg.PulsePolarity[i] == CAEN_DGTZ_PulsePolarityNegative? fCPol[i]->Select(0) : fCPol[i]->Select(1);
		fCPol[i]->Resize(50, 20);
		fCPol[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCPol[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
			
		// RecordLength, pretrigger, DCOffset textentries	
		for (j = 0; j < 4; j++) { 
			
			tbuf[i][j] = new TGTextBuffer(10);
			
			switch (j){
				case 0 :
					sprintf(str, "%i", Dcfg.RecordLength[i]);	
					break;
				case 1 :
					sprintf(str, "%i", Dcfg.PreTrigger[i]);	
					break;	
				case 2 :
					sprintf(str, "%i", Dcfg.DCOffset[i]);	
					break;
				case 3 :
					sprintf(str, "%i", Dcfg.thr[i]);	
					break;	
			}
			
			
			tbuf[i][j]->AddText(0, str);
				
			fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
			fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
			fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
			fTEntries[i][j]->SetFont(paramFont);
			fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		}
		
		// nsbl comboboxies
		fCnsbl[i] = new TGComboBox(fVF[i], i+16);	
		fCnsbl[i]->AddEntry("0", 0);
		fCnsbl[i]->AddEntry("16", 1);
		fCnsbl[i]->AddEntry("64", 2);
		fCnsbl[i]->AddEntry("256", 3);
		fCnsbl[i]->AddEntry("1024", 4);
		fCnsbl[i]->Select(Dcfg.nsbl[i]);
		fCnsbl[i]->Resize(50, 20);
		fCnsbl[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCnsbl[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		// lgate, sgate, pgate textentries	
		for (j = 4; j < 7; j++) { 
			tbuf[i][j] = new TGTextBuffer(10);
			
			switch (j){
				case 4 :
					sprintf(str, "%i", Dcfg.lgate[i]);	
					break;
				case 5 :
					sprintf(str, "%i", Dcfg.sgate[i]);	
					break;	
				case 6 :
					sprintf(str, "%i", Dcfg.pgate[i]);	
					break;
			}
			
			
			tbuf[i][j]->AddText(0, str);
				
			fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
			fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
			fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
			fTEntries[i][j]->SetFont(paramFont);
			fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		}
		
		// self triggers checkboxies	
		fCselft[i] = new TGCheckButton(fVF[i], "", i+16);	
		Dcfg.selft[i] == 0 ? fCselft[i]->SetState(kButtonUp) : fCselft[i]->SetState(kButtonDown); 
		fCselft[i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
		fVF[i]->AddFrame(fCselft[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 4, 4));
		
		//trgc Trigger Configuration
		fCtrgc[i] = new TGComboBox(fVF[i], i+32);	
		fCtrgc[i]->AddEntry("Peak", 0);
		fCtrgc[i]->AddEntry("Threshold", 1);
		fCtrgc[i]->Resize(50, 20);
		fCtrgc[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fCtrgc[i]->SetEnabled(0); // disabled for better times
		fVF[i]->AddFrame(fCtrgc[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//discr LED/CFD
		fCdiscr[i] = new TGComboBox(fVF[i], i+48);	
		fCdiscr[i]->AddEntry("LED", 0);
		fCdiscr[i]->AddEntry("CFD", 1);
		fCdiscr[i]->Select(Dcfg.discr[i]==CAEN_DGTZ_DPP_DISCR_MODE_LED ? 0 : 1);
		fCdiscr[i]->Resize(50, 20);
		fCdiscr[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCdiscr[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//cfdd textentry
		j = 7;
		tbuf[i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", Dcfg.cfdd[i]);	
		tbuf[i][j]->AddText(0, str);
				
		fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
		fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
		fTEntries[i][j]->SetFont(paramFont);
		fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		//cfdf fraction 0 - 25%, 1 - 50%, 2 - 75%, 3 - 100%
		fCcfdf[i] = new TGComboBox(fVF[i], i+64);	
		fCcfdf[i]->AddEntry("25%", 0);
		fCcfdf[i]->AddEntry("50%", 1);
		fCcfdf[i]->AddEntry("75%", 2);
		fCcfdf[i]->AddEntry("100%", 3);
		fCcfdf[i]->Select(Dcfg.cfdf[i]);
		fCcfdf[i]->Resize(50, 20);
		fCcfdf[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCcfdf[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		//tvaw textentry
		j = 8;
		tbuf[i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", Dcfg.tvaw[i]);	
		tbuf[i][j]->AddText(0, str);
				
		fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
		fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
		fTEntries[i][j]->SetFont(paramFont);
		fTEntries[i][j]->SetEnabled(0); // disabled for better times
		fVF[i]->AddFrame(fTEntries[i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3)); //kLHintsCenterY | kLHintsLeft  //new TGLayoutHints(0, 0, 0, 4, 4)
		
		//csens charge sensibility
		fCcsens[i] = new TGComboBox(fVF[i], i+80);	
		fCcsens[i]->AddEntry("5fC/LSB", 0);
		fCcsens[i]->AddEntry("20fC/LSB", 1);
		fCcsens[i]->AddEntry("80fC/LSB", 2);
		fCcsens[i]->AddEntry("320fC/LSB", 3);
		fCcsens[i]->AddEntry("1.28pC/LSB", 4);
		fCcsens[i]->AddEntry("5.12pC/LSB", 5);
		fCcsens[i]->Select(Dcfg.csens[i]);
		fCcsens[i]->Resize(50, 20);
		fCcsens[i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
		fVF[i]->AddFrame(fCcsens[i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));
		
		fVF[i]->Resize();
		fGF[i]->Resize();	
	}
	
	fMain->AddFrame(f1, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 5, 10, 0) );


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
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;		
	
	for (int i = 0; i < N_CH; i++){
		//ChannelMask
		ret = CAEN_DGTZ_WriteRegister(handle, EnableMaskAddress, Dcfg.ChannelMask);	
		//InputRange
		ret = SetInputRange(handle, i, Dcfg.InputRange[i]);	
		//Polarity
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, i, (CAEN_DGTZ_PulsePolarity_t)Dcfg.PulsePolarity[i]);
		//RecordLength
		ret = CAEN_DGTZ_SetRecordLength(handle, Dcfg.RecordLength[i], i);
		//PreTrigger
		ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, Dcfg.PreTrigger[i]);
		//DCOffset
		ret = CAEN_DGTZ_SetChannelDCOffset(handle, i, Dcfg.DCOffset[i]); 
		//thr
		ret = CAEN_DGTZ_WriteRegister(handle, ThresholdAddress[i], Dcfg.thr[i]); 
		//nsbl
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		reg_data = ( reg_data & ~(0x700000) ) | (Dcfg.nsbl[i]<<20); // mask to clean and change b22:b20
		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);	
		//lgate
		ret = CAEN_DGTZ_WriteRegister(handle, LongGateWidthAddress[i], Dcfg.lgate[i]); 
		//sgate
		ret = CAEN_DGTZ_WriteRegister(handle, ShortGateWidthAddress[i], Dcfg.sgate[i]);
		//pgate
		ret = CAEN_DGTZ_WriteRegister(handle, GateOffsetAddress[i], Dcfg.pgate[i]); 
		//self
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		printf(" Previously in 0x%04X val %i \n", DPPAlgControlAddress[i], Dcfg.selft[i]);
		Dcfg.selft[i] ==0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); //[b24] responsible for self trigger
		printf(" Self [%i] val %i \n", i, Dcfg.selft[i]); // 1 - Enabled,  0 - Disabled
		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);	// 1 - Disabled, 0 - Enabled
		//discr
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		Dcfg.discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_CFD ? reg_data |=  (1<<6) : reg_data &=  ~(1<<6);
		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);	
		//cfdd
		ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[i], &reg_data);
		reg_data = (reg_data &~ (0x0F) ) + Dcfg.cfdd[i];
		ret = CAEN_DGTZ_WriteRegister(handle, CFDSettingsAddress[i], reg_data);
		//cfdf
		ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[i], &reg_data);
		reg_data = (reg_data &~ (0xF00 ) ) | ( Dcfg.cfdf[i] << 8);
		ret = CAEN_DGTZ_WriteRegister(handle, CFDSettingsAddress[i], reg_data);
		//tvaw
		ret = CAEN_DGTZ_WriteRegister(handle, ShapedTriggerWidthAddress[i], Dcfg.tvaw[i]);	 // 0x14 = 20x8ns = 160 ns window | 0x40 = 64x8ns = 512 ns window
		//csens
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		reg_data = (reg_data &~ (0x00000007 ) ) | ( Dcfg.csens[i] << 0); // clean up [b2:0]
		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);  
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
	TGTextEntry *te = (TGTextEntry *) gTQSender;
	Int_t id = te->WidgetId();
	Int_t i = id%16;
	Int_t j = id  / 16;
	
	fCAll->GetState()== kButtonDown ? fAll = true : fAll = false;
	//printf("smth changed in %i i = %i j = %i, new val = %s num : %i \n", id, i, j, tbuf[i][j]->GetString(), atoi( tbuf[i][j]->GetString() ) );
	
	//RecordLength
	if (id <16){
		Dcfg.RecordLength[i] = atoi( tbuf[i][j]->GetString() );
		ret = CAEN_DGTZ_SetRecordLength(handle, Dcfg.RecordLength[i], i);
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.RecordLength[n] = Dcfg.RecordLength[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					ret = CAEN_DGTZ_SetRecordLength(handle, Dcfg.RecordLength[n], n);
				}
			}
		}
	}
	
	//pretrigger
	if (id >=16 && id <32){
		Dcfg.PreTrigger[i] = atoi( tbuf[i][j]->GetString() );
		ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, Dcfg.PreTrigger[i]);  //30
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.PreTrigger[n] = Dcfg.PreTrigger[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle, n, Dcfg.PreTrigger[n]);
				}
			}
		}
	}
	
	// DCOffset
	if (id >=32 && id <48){
		Dcfg.DCOffset[i] = atoi( tbuf[i][j]->GetString() );
		ret = CAEN_DGTZ_SetChannelDCOffset(handle, i, Dcfg.DCOffset[i]); //0x8000
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.DCOffset[n] = Dcfg.DCOffset[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					ret = CAEN_DGTZ_SetChannelDCOffset(handle, n, Dcfg.DCOffset[n]); //0x8000
				}
			}
		}
	}
	
	// thresholds
	if (id >=48 && id <64){
		Dcfg.thr[i] = atoi( tbuf[i][j]->GetString() );
		printf(" thr[%i] set to : %i \n", i, Dcfg.thr[i]);	
		//ret = CAEN_DGTZ_ReadRegister(handle, 0x1080, &reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, ThresholdAddress[i], Dcfg.thr[i]);
		ret = CAEN_DGTZ_ReadRegister(handle, ThresholdAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n", ThresholdAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.thr[n] = Dcfg.thr[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle, ThresholdAddress[n], Dcfg.thr[n]);
					ret = CAEN_DGTZ_ReadRegister(handle, ThresholdAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", ThresholdAddress[n], reg_data);	
				}
			}
		}
		
	}
	
	
	// Long Gate [samples]
	if (id >=64 && id <80){
		Dcfg.lgate[i] = atoi( tbuf[i][j]->GetString() );
		printf(" lgate[%i] set to : %i \n", i, Dcfg.lgate[i]);	
		ret = CAEN_DGTZ_WriteRegister(handle, LongGateWidthAddress[i], Dcfg.lgate[i]);
		ret = CAEN_DGTZ_ReadRegister(handle, LongGateWidthAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n", LongGateWidthAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.lgate[n] = Dcfg.lgate[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle, LongGateWidthAddress[n], Dcfg.lgate[n]);
					ret = CAEN_DGTZ_ReadRegister(handle, LongGateWidthAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", LongGateWidthAddress[n], reg_data);	
				}
			}
		}
	}	
		
	// Short Gate [samples]
	if (id >=80 && id <96){
		Dcfg.sgate[i] = atoi( tbuf[i][j]->GetString() );
		printf(" sgate[%i] set to : %i \n", i, Dcfg.sgate[i]);	
		ret = CAEN_DGTZ_WriteRegister(handle, ShortGateWidthAddress[i], Dcfg.sgate[i]);
		ret = CAEN_DGTZ_ReadRegister(handle, ShortGateWidthAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n", ShortGateWidthAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.sgate[n] = Dcfg.sgate[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle, ShortGateWidthAddress[n], Dcfg.sgate[n]);
					ret = CAEN_DGTZ_ReadRegister(handle, ShortGateWidthAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", ShortGateWidthAddress[n], reg_data);	
				}
			}
		}
	}	
	
	// Gate Offset (pregate)[samples]
	if (id >=96 && id <112){
		Dcfg.pgate[i] = atoi( tbuf[i][j]->GetString() );
		printf(" pgate[%i] set to : %i \n", i, Dcfg.pgate[i]);	
		ret = CAEN_DGTZ_WriteRegister(handle, GateOffsetAddress[i], Dcfg.pgate[i]);
		ret = CAEN_DGTZ_ReadRegister(handle, GateOffsetAddress[i], &reg_data);
		printf(" In  0x%04X 0x%04X \n",GateOffsetAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.pgate[n] = Dcfg.pgate[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					
					ret = CAEN_DGTZ_WriteRegister(handle, GateOffsetAddress[n], Dcfg.pgate[n]);
					ret = CAEN_DGTZ_ReadRegister(handle, GateOffsetAddress[n], &reg_data);
					printf(" In  0x%04X 0x%04X \n", GateOffsetAddress[n], reg_data);	
				}
			}
		}
	}
	// CFD delay [samples]
	if (id >=112 && id <128){
		Dcfg.cfdd[i] = atoi( tbuf[i][j]->GetString() );
		ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[i], &reg_data);
		printf(" Previously in  0x%04X: %08X \n", CFDSettingsAddress[i], reg_data);
		reg_data = (reg_data &~ (0x0F) ) + Dcfg.cfdd[i];
		ret = CAEN_DGTZ_WriteRegister(handle, CFDSettingsAddress[i], reg_data);
		ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[i], &reg_data);
		printf(" In  0x%04X 0x%08X \n", CFDSettingsAddress[i], reg_data);	
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if (n != i){
					Dcfg.cfdd[n] = Dcfg.cfdd[i];
					fTEntries[n][j]->SetText(tbuf[i][j]->GetString( ));
					
					ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[n], &reg_data);
					printf(" Previously in  0x%04X: %08X \n", CFDSettingsAddress[n], reg_data);
					reg_data = (reg_data &~ (0x0F) ) + Dcfg.cfdd[n];
					ret = CAEN_DGTZ_WriteRegister(handle, CFDSettingsAddress[n], reg_data);
					ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[n], &reg_data);
					printf(" In  0x%04X 0x%08X \n", CFDSettingsAddress[n], reg_data);	
				}
			}
		}
	}

	//if (id >=128 && id <144)
	//	Dcfg.tvaw[i] = atoi( tbuf[i][j]->GetString() );

	//ret = SetDPPParameters(handle, Dcfg);
	//for (int i=0;  i <N_CH; i++)
	//	if (Dcfg.thr[i] > 4000 )
	//		ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], Dcfg.thr[i]);
	
	
}

void ParamsMenu::DoCheckBox()
{
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	//printf("checkbox changed %i \n", id);
	fCAll->GetState()== kButtonDown ? fAll = true : fAll = false;
	
	//enabled channels mask
	if (id <16){
		
		fC[id]->GetState()== kButtonDown ? Dcfg.ChannelMask  |=  (1<<id) : Dcfg.ChannelMask  &= ~ (1<<id);
		ret = CAEN_DGTZ_ReadRegister(handle, EnableMaskAddress, &reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, EnableMaskAddress, Dcfg.ChannelMask);	
				
		if (fAll){
			for (int i = 0; i < N_CH; i++){ 
				if ( i!=id  ){
					fC[i]->SetState( fC[id]->GetState( ) );
					fC[i]->GetState()== kButtonDown ? Dcfg.ChannelMask  |=  (1<<i) : Dcfg.ChannelMask  &= ~ (1<<i);
				}	
			}
			
			ret = CAEN_DGTZ_ReadRegister(handle, EnableMaskAddress, &reg_data);
			ret = CAEN_DGTZ_WriteRegister(handle, EnableMaskAddress, Dcfg.ChannelMask);	
		}
		printf(" ChannelMask 0x%04X \n",Dcfg.ChannelMask);	
	}
	
	//self triggers
	if (id >=16 && id <32 ){
		fCselft[id-16]->GetState()== kButtonDown ? Dcfg.selft[id-16] = 1 : Dcfg.selft[id-16] = 0;
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[id-16], &reg_data);
		Dcfg.selft[id-16] ==0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); //[b24] responsible for self trigger
 		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[id-16], reg_data);	
				
		if (fAll){
			for (int i = 0; i < N_CH; i++){ 
				if ( i!=(id-16) ){
					fCselft[i]->SetState( fCselft[id-16]->GetState( ) );
					fCselft[i]->GetState()== kButtonDown ? Dcfg.selft[i] = 1 : Dcfg.selft[i] = 0;
					
					ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
					Dcfg.selft[i] ==0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); //[b24] responsible for self trigger
					ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);	
					
				}	
			}
				
		}
		
		
	}
	
	//ret = SetDPPParameters(handle, Dcfg);
	//for (int i=0;  i <N_CH; i++)
	//	if (Dcfg.thr[i] > 4000 )
	//		ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], Dcfg.thr[i]);
		
}

void ParamsMenu::DoComboBox()
{
	TGComboBox *cb = (TGComboBox *) gTQSender;
	Int_t id = cb->WidgetId();
	Int_t i = id%16;
		
	fCAll->GetState()== kButtonDown ? fAll = true : fAll = false;		
	//printf("combobox changed %i  \n", id);
	
	// InputRange
	if (id < 16){
		Dcfg.InputRange[i] = fCIRange[i]->GetSelected() == 0 ? InputRange_2Vpp : InputRange_0_5Vpp;
		ret = SetInputRange(handle, i, Dcfg.InputRange[i]);
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if ( n!=i) {
					fCIRange[n]->Select(fCIRange[i]->GetSelected( ) );
					Dcfg.InputRange[n] = Dcfg.InputRange[i];
				}
			}
		}	
			
	}
	
	// polarity 
	if (id > 99){ //if (id <16){
		Dcfg.PulsePolarity[i]  = fCPol[i]->GetSelected() == 0 ?  CAEN_DGTZ_PulsePolarityNegative : CAEN_DGTZ_PulsePolarityPositive;
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Dcfg.PulsePolarity[i]);
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if ( n!=i) {
					fCPol[n]->Select(fCPol[i]->GetSelected( ) );
					Dcfg.PulsePolarity[n] = Dcfg.PulsePolarity[i];
					ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, n, Dcfg.PulsePolarity[n]); // due to Select function  it can be setted up twice
				}
			}
		}	
	}
	
	// number samples for baseline 
	if (id >=16 && id <32){
		Dcfg.nsbl[i] = fCnsbl[i]->GetSelected();
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
				
		reg_data = ( reg_data & ~(0x700000) ) | (Dcfg.nsbl[i]<<20); // mask to clean and change b22:b20
						
 		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if ( n!=i) {
					fCnsbl[n]->Select(fCnsbl[i]->GetSelected( ) );
					Dcfg.nsbl[n]  = Dcfg.nsbl[i];
				}
			}
		}	
			
	}
	
	// discriminator type 
	if (id >=48 && id <64){
		fCdiscr[i]->GetSelected() == 0 ? Dcfg.discr[i] = CAEN_DGTZ_DPP_DISCR_MODE_LED : Dcfg.discr[i] = CAEN_DGTZ_DPP_DISCR_MODE_CFD;
		printf("Ch[%i] discr : %s \n", i, fCdiscr[i]->GetSelected() == 0 ? "LED" : "CFD" );
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		printf(" Previously in  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		Dcfg.discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_CFD ? reg_data |=  (1<<6) : reg_data &=  ~(1<<6);
		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if ( n!=i) {
					fCdiscr[n]->Select(fCdiscr[i]->GetSelected( ) );
					Dcfg.discr[n]  = Dcfg.discr[i];
					//not necessary due to strange work of "Select" function
					//ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[n], reg_data);	
					//ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[n], &reg_data);
					//printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[n], reg_data);
				}
			}
		}	
		
	}
	
	//CFD fraction: 0->25%; 1->50%; 2->75%; 3->100% 
		
	if (id >=64 && id <80){
		Dcfg.cfdf[i] = fCcfdf[i]->GetSelected();
		ret = CAEN_DGTZ_ReadRegister(handle, CFDSettingsAddress[i], &reg_data);
		reg_data = (reg_data &~ (0xF00 ) ) | ( Dcfg.cfdf[i] << 8);
		ret = CAEN_DGTZ_WriteRegister(handle, CFDSettingsAddress[i], reg_data);
				
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if ( n!=i) {
					fCcfdf[n]->Select(fCcfdf[i]->GetSelected( ) );
					Dcfg.cfdf[n]  = Dcfg.cfdf[i];
				}
			}
		}	
	}	
	
	// charge sensitivity 0 - 5;  5fC - 5.12pC
	if (id >=80 && id <96){
		Dcfg.csens[id-80] = fCcsens[id-80]->GetSelected();
		printf("Ch[%i] csens : %i \n", id-80, fCcsens[id-80]->GetSelected());
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		printf(" Previously in  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		reg_data = (reg_data &~ (0x00000007 ) ) | ( Dcfg.csens[i] << 0); // clean up [b2:0]
		ret = CAEN_DGTZ_WriteRegister(handle, DPPAlgControlAddress[i], reg_data);
		ret = CAEN_DGTZ_ReadRegister(handle, DPPAlgControlAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", DPPAlgControlAddress[i], reg_data);
		
		
		if (fAll){
			for (int n = 0; n < N_CH; n++){ 
				if ( n!=i) {
					fCcsens[n]->Select(fCcsens[i]->GetSelected( ) );
					Dcfg.csens[n]  = Dcfg.csens[i];
				}
			}
		}
	}	
			
}

void ParamsMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}
