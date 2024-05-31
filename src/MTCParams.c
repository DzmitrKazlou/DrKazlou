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
	
	const char *paramlabel[] = {"Enable", "Polarity", "RecordLength", "Pretrigger", "DCOffset", "thr", "nsbl", "lgate", "sgate", "pgate","selft", "trgc", "discr", "cfdd", "cfdf", "tvaw", "csens"};		
	
	int iStyle[]	= {0, 2}; 
	double Lmin[]	= {-10000, -50, -50,-50}; 	
	double Lmax[]	= {10000, 10000, 10000,10000}; 	
	
	fVF0 = new TGVerticalFrame(f1, 200, 300);
	f1->AddFrame(fVF0, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));//
	
	for (j = 0; j < 17; j++) { 
	
		fLabel[j] = new TGLabel(fVF0, paramlabel[j]);
		fLabel[j]->SetTextFont(labelFont);
		if (j == 0)
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 14, 2)); // left right top bottom
		else
			fVF0->AddFrame(fLabel[j], new TGLayoutHints(kLHintsCenterX, 0, 0, 4, 4));
	}				
	
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

		// polarity comboboxies
		fCPol[i] = new TGComboBox(fVF[i], i);	
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
		
		// sel triggers checkboxies	
		fCselft[i] = new TGCheckButton(fVF[i], "", i+16);	
		Dcfg.selft[i] ==0 ? fCselft[i]->SetState(kButtonUp) : fCselft[i]->SetState(kButtonDown); 
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
		j =7;
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
		j =8;
		tbuf[i][j] = new TGTextBuffer(10);
		sprintf(str, "%i", Dcfg.tvaw[i]);	
		tbuf[i][j]->AddText(0, str);
				
		fTEntries[i][j] = new TGTextEntry(fVF[i], tbuf[i][j], j*16 + i); //
		fTEntries[i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");	
		fTEntries[i][j]->Resize(50, fTEntries[i][j]->GetDefaultHeight());
		fTEntries[i][j]->SetFont(paramFont);
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
	
	printf("smth changed in %i i = %i j = %i, new val = %s num : %i \n", id, i, j, tbuf[i][j]->GetString(), atoi( tbuf[i][j]->GetString() ) );
	
	
	if (id <16){
		Dcfg.RecordLength[i] = atoi( tbuf[i][j]->GetString() );
		if (id == 0)
			ret = CAEN_DGTZ_SetRecordLength(handle, Dcfg.RecordLength[id], id);
	}	
	if (id >=16 && id <32)
		Dcfg.PreTrigger[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=32 && id <48){
		Dcfg.DCOffset[id-32] = atoi( tbuf[i][j]->GetString() );
		ret = CAEN_DGTZ_SetChannelDCOffset(handle, id-32, Dcfg.DCOffset[id-32]); //0x8000
	}	
	if (id >=48 && id <64){
		Dcfg.thr[i] = atoi( tbuf[i][j]->GetString() );
		printf(" thr[%i] set to : %i \n", i, Dcfg.thr[i]);	
		uint32_t reg_data;
		//ret = CAEN_DGTZ_ReadRegister(handle, 0x1080, &reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, th_add[id-48], Dcfg.thr[i]);
		ret = CAEN_DGTZ_ReadRegister(handle, th_add[id-48], &reg_data);
		printf(" In  0x%04X 0x%04X \n", th_add[id-48], reg_data);	
		
	}	
	
	if (id >=64 && id <80)
		Dcfg.lgate[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=80 && id <96)
		Dcfg.sgate[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=96 && id <112)
		Dcfg.pgate[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=112 && id <128)
		Dcfg.cfdd[i] = atoi( tbuf[i][j]->GetString() );
	if (id >=128 && id <144)
		Dcfg.tvaw[i] = atoi( tbuf[i][j]->GetString() );

	
	ret = SetDPPParameters(handle, Dcfg);
	for (int i=0;  i <N_CH; i++)
		if (Dcfg.thr[i] > 4000 )
			ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], Dcfg.thr[i]);
	
	
}

void ParamsMenu::DoCheckBox()
{
	TGCheckButton *chb = (TGCheckButton *) gTQSender;
	Int_t id = chb->WidgetId();
	printf("checkbox changed %i \n", id);
	
	
	if (id <16){
		fC[id]->GetState()== kButtonDown ? Dcfg.ChannelMask  |=  (1<<id) : Dcfg.ChannelMask  &= ~ (1<<id);
		printf("channelmask: %i \n", Dcfg.ChannelMask);
	}	
	
	if (id >=16 && id <32 )
		fCselft[id-16]->GetState()== kButtonUp ? Dcfg.selft[id-16] = 0 : Dcfg.selft[id-16] = 1;
	
	ret = SetDPPParameters(handle, Dcfg);
	for (int i=0;  i <N_CH; i++)
		if (Dcfg.thr[i] > 4000 )
			ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], Dcfg.thr[i]);
		
}

void ParamsMenu::DoComboBox()
{
	TGComboBox *cb = (TGComboBox *) gTQSender;
	Int_t id = cb->WidgetId();
		
	printf("combobox changed %i  \n", id);
	
	if (id <16 && fCPol[id]->GetSelected() == 0 ){
		Dcfg.PulsePolarity[id]  = CAEN_DGTZ_PulsePolarityNegative;
		printf("Ch[%i] : %s  \n", id, "Negative");
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, id, Dcfg.PulsePolarity[id]);
	}
	
	if (id <16 && fCPol[id]->GetSelected() == 1 ){
		Dcfg.PulsePolarity[id]  = CAEN_DGTZ_PulsePolarityPositive;
		printf("Ch[%i] : %s  \n", id, "Positive");
		ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, id, Dcfg.PulsePolarity[id]);
	}
	
	if (id >=16 && id <32){
		Dcfg.nsbl[id-16] = fCnsbl[id-16]->GetSelected();
		printf("Ch[%i] nsbl : %i  \n", id-16, fCnsbl[id-16]->GetSelected());
	}	
	
	if (id >=48 && id <64){
		fCdiscr[id-48]->GetSelected() == 0 ? Dcfg.discr[id-48] = CAEN_DGTZ_DPP_DISCR_MODE_LED : Dcfg.discr[id-48] = CAEN_DGTZ_DPP_DISCR_MODE_CFD;
		printf("Ch[%i] discr : %s \n", id-48, fCdiscr[id-48]->GetSelected() == 0 ? "LED" : "CFD" );
	}	
	
	if (id >=64 && id <80){
		Dcfg.cfdf[id-64] = fCcfdf[id-64]->GetSelected();
		printf("Ch[%i] cfdf : %i  \n", id-64, fCcfdf[id-64]->GetSelected()*25 + 25 );
	}	
	
	if (id >=80 && id <96){
		Dcfg.csens[id-80] = fCcsens[id-80]->GetSelected();
		printf("Ch[%i] csens : %i \n", id-80, fCcsens[id-80]->GetSelected());
	}	
	
	ret = SetDPPParameters(handle, Dcfg);
	for (int i=0;  i <N_CH; i++)
		if (Dcfg.thr[i] > 4000 )
			ret = CAEN_DGTZ_WriteRegister(handle, th_add[i], Dcfg.thr[i]);
		
}

void ParamsMenu::TryToClose()
{
  
   printf("Can't close the window '%s' : a message box is still open\n", fMain->GetWindowName());
}
