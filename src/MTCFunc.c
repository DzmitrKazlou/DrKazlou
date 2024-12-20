#include <stdio.h>
#include <stdlib.h>
#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
#include "MTCFunc.h"


#include "TROOT.h"

#include <TSystem.h>

#include "TCanvas.h"
#include "TStyle.h"


#define CAEN_USE_DIGITIZERS
//#define IGNORE_DPP_DEPRECATED


//extern DigitizerConfig_t Dcfg[2];
extern ReadoutConfig_t Rcfg;
//extern Histograms_t Histo;
//extern int loop;
extern int handle;
	extern char *buffer[2];
		
	extern CAEN_DGTZ_DPP_PSD_Event_t   *Events[2][MAX_CH];  // events buffer
	extern CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms[2];         // waveforms buffer
		
		
	
//using namespace std;

long get_time()
{
    long time_ms;
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;

    return time_ms;
}

// Comparator for sorting by time
bool compareByTime(const TimeStruct_t& a, const TimeStruct_t& b) {
    //return a.timestamp < b.timestamp;
	return a.timetag < b.timetag;
}

void SortTime(std::vector<TimeStruct_t>&times, TimeStruct_t newElement)
{	
	auto it = std::lower_bound(times.begin(), times.end(), newElement, compareByTime);
	int pos = it - times.begin( );
	//printf("S_time size %li newTime B[%i] Ch(%i) %i \n", times.size( ), newElement.b, newElement.ch, newElement.timetag);
	//printf("TimeBuffer.size( ) %li Position for insert %li \n", times.size( ), it - times.begin( ));	
    // Insert the new element at the correct position
	
    times.insert(it, newElement);
	

	//times.resize(times.size( ) + 1 );
	//for (int ind = pos+1; ind<times.size( ); ind++)
	//	times[ind] = times[ind-1];
	//times[pos] = newElement;

}


void DrawRndmHisto(TH1D *h, TCanvas *can){
	TRandom3 *rng = new TRandom3( );
	rng->SetSeed(0);

	//if (!h)
	//	h = new TH1D(" h_rndm", "h_rndm", 1000, -4, 4);
	
	h->FillRandom(rng->Rndm( )>0.5 ? "gaus" : "pol2");	

	can->cd(1);
	h->Draw("HIST");
	can->Update( );
}

void ManageCanvas( TCanvas *can, Histograms_t *Histo, int N_CH, TGCheckButton *fCa[16], TGCheckButton *fC[15], TGCheckButton *fCsub[2], int id){

if (id < MAX_CH ) {
		Histo->fDraw[id] = fCa[id]->GetState() == kButtonDown ? true : false;
		
		for (int i = 0; i<N_CH; i++)
			Histo->cTPos[i] = 0;
		
		Histo->NPadT = 0;
		int n = MAX_CH;
		for (int i = 0; i<N_CH; i++){
			if (Histo->fDraw[i] && i<n) 
				n = i;
			
			if (Histo->fDraw[i]){
				Histo->NPadT++;
				for (int j = 0; j<i+1; ++j)
					Histo->cTPos[i] += (Histo->fDraw[j] == true ? 1 : 0);
			}
		}	
		Histo->FirstToDraw = n;
	}
   
   if (id == 41)
	   Histo->fBL = fC[0]->GetState() == kButtonDown ? true : false; 
   
   // different type of histograms handler
   if ( id > 41) {
		Histo->fXYPadModified = false;
	
		Histo->fTrace = fC[1]->GetState() == kButtonDown ? true : false;
		Histo->fAmpl = fC[2]->GetState() == kButtonDown ? true : false;
		Histo->fCharge = fC[3]->GetState() == kButtonDown ? true : false;
		Histo->fInt = fC[4]->GetState() == kButtonDown ? true : false;
		Histo->fdT = fC[5]->GetState() == kButtonDown ? true : false;
		Histo->fPSD_ampl = fC[6]->GetState() == kButtonDown ? true : false;
		Histo->fPSD_int = fC[7]->GetState() == kButtonDown ? true : false;
		Histo->fQsl = fC[8]->GetState() == kButtonDown ? true : false;
		Histo->fIA = fC[9]->GetState() == kButtonDown ? true : false;
		Histo->fLayers = fC[10]->GetState() == kButtonDown ? true : false;
		Histo->fLayersCoeff  = fCsub[0]->GetState() == kButtonDown ? true : false;
		Histo->fCounts = fC[11]->GetState() == kButtonDown ? true : false;
		Histo->fXY = fC[12]->GetState() == kButtonDown ? true : false;

		printf("Charge %i \n", Histo->fCharge);
		printf("IntAmpl %i \n", Histo->fIA);
		printf("PSD_ampl %i \n", Histo->fPSD_ampl);
				
		if ( id < 55 )
			fC[id-41]->GetState() == kButtonDown ? Histo->NPad++ : Histo->NPad-- ;
		
		if (Histo->fAmpl)
			Histo->cAmpl = 1 + (Histo->fTrace ? 1 : 0);
					
		if (Histo->fInt)
			Histo->cInt = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0);
		
		if (Histo->fCharge)	
			Histo->cCharge = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0) + (Histo->fInt ? 1 : 0);
		
		if (Histo->fdT)	
			Histo->cdT = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0) + (Histo->fInt ? 1 : 0) + (Histo->fCharge ? 1 : 0);
		
		if (Histo->fIA)	
			Histo->cIA = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0) + (Histo->fInt ? 1 : 0) + (Histo->fCharge ? 1 : 0) + (Histo->fdT ? 1 : 0);
		
		if (Histo->fPSD_ampl)	
			Histo->cPSD_ampl = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0) + (Histo->fInt ? 1 : 0) + (Histo->fCharge ? 1 : 0) + (Histo->fdT ? 1 : 0) + (Histo->fIA ? 1 : 0);
		
		if (Histo->fPSD_int)	
			Histo->cPSD_int = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0) + (Histo->fInt ? 1 : 0) + (Histo->fCharge ? 1 : 0) + (Histo->fdT ? 1 : 0) + (Histo->fIA ? 1 : 0) + (Histo->fPSD_ampl ? 1 : 0);
		
		if (Histo->fQsl)	
			Histo->cQsl = 1 + (Histo->fTrace ? 1 : 0) + (Histo->fAmpl ? 1 : 0) + (Histo->fInt ? 1 : 0) + (Histo->fCharge ? 1 : 0) + (Histo->fdT ? 1 : 0) + (Histo->fIA ? 1 : 0) + (Histo->fPSD_ampl ? 1 : 0) + (Histo->fPSD_int ? 1 : 0);
		
		if (Histo->fLayers)
			Histo->cLayers = Histo->NPad - (Histo->fXY ? 1 : 0) - (Histo->fCounts ? 1 : 0);
		
		if (Histo->fCounts)
			Histo->cCounts = Histo->NPad - (Histo->fXY ? 1 : 0);
		
		if (Histo->fXY)
			Histo->cXY = Histo->NPad;
				
		
		TThread::Lock( );
		can->Clear( );
		can->SetGrid( );
		
		if (Histo->NPad == 1)
			can->Divide(1, 1, 0.001, 0.001);
		if (Histo->NPad == 2)
			can->Divide(2, 1, 0.001, 0.001);
		if (Histo->NPad > 2 && Histo->NPad < 5)
			can->Divide(2, 2, 0.001, 0.001);
		if (Histo->NPad > 4 && Histo->NPad < 7)
			can->Divide(3, 2, 0.001, 0.001);
		if (Histo->NPad > 6 && Histo->NPad < 10)
			can->Divide(3, 3, 0.001, 0.001);
		if (Histo->NPad > 9 && Histo->NPad < 13)
			can->Divide(4, 3, 0.001, 0.001);
		if (Histo->NPad > 12)
			can->Divide(4, 4, 0.001, 0.001);
		
		can->Modified();
		TThread::UnLock( );		
   }


}

char* CorrectUnits(float CountRate){

	char *str = new char[13]; // 13 enough? examples  [99 Hz] - 6; [0.1 kHz] - 8 [99 kHz] - 7 [0.3 MHz] - 8
	if (CountRate < 100)
		strcpy(str, Form("%.0f Hz",CountRate) );
	if (CountRate >= 100 && CountRate < 100000)
		strcpy(str, Form("%0.1f kHz",CountRate/1000) );
	if (CountRate >= 100000 && CountRate < 10000000)
		strcpy(str, Form("%0.2f MHz",CountRate/1000000) );
	if (CountRate >= 10000000)
		strcpy(str, "Hi!" );
	return str;
}

void CalcRate(uint64_t &PrevRateTime){
		
	char StatusText[2][1000], CName[100], str[100], last[3][20] = {};
	uint64_t CurrentTime, ElapsedTime, DAQTime;
	CurrentTime = get_time();
   	ElapsedTime = CurrentTime - PrevRateTime;
	DAQTime = (CurrentTime - Rcfg.StartTime) / 1000;

			if (Rcfg.fTimer && DAQTime > Rcfg.timer_val)
				Rcfg.loop = 0;
					        	
			if (ElapsedTime > 1000) { // 1000
				sprintf(CName,"T: %li s",  DAQTime );
				Rcfg.TLabel->SetText(CName);
				gSystem->ProcessEvents(); 

				for (int b = 0; b<Rcfg.N_B; b++){
					if (Rcfg.Nb[b] != 0){
						sprintf(CName,"Read: %.2f MB/s", (float)Rcfg.Nb[b]/((float)ElapsedTime*1048.576f) );
						sprintf(StatusText[b], "B[%i] %s", b, CName);
					
						for (int ch=0; ch<Rcfg.Nch; ch++) { //8
							if (Rcfg.TrgCnt[b][ch] != 0){
								sprintf(CName, " CH[%i]: %s", ch, CorrectUnits( (float)Rcfg.TrgCnt[b][ch]*1000/(float)ElapsedTime) );
								//sprintf(CName, " CH[%i]: %.0f kHz", ch, (float)Rcfg.TrgCnt[b][ch]/(float)ElapsedTime);
								//sprintf(CName, " CH[%i]: %.0f Hz", ch, (float)Rcfg.TrgCnt[b][ch]*1000.0f/(float)ElapsedTime);
								strcat(StatusText[b], CName);
							}
							else{
								sprintf(CName, " No data...");
								strcat(StatusText[b], CName);
							}
							Rcfg.TrgCnt[b][ch] = 0;
						}
					}
					else{
						sprintf(CName, "No data...");
						sprintf(StatusText[b], "B[%i] %s", b, CName);
					}

				Rcfg.StatusLabel[b]->SetText(StatusText[b]);					
				Rcfg.Nb[b] = 0;
				}		

            PrevRateTime = CurrentTime;
			TThread::Lock( );
			gSystem->ProcessEvents(); 
			//gSystem->Sleep(0); 
			TThread::UnLock( );
        	}
	
}

CAEN_DGTZ_ErrorCode ForceClockSync(int handle)
{    
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    gSystem->Sleep(50); 
    /* Force clock phase alignment */
    ret = CAEN_DGTZ_WriteRegister(handle, 0x813C, 1); //ADDR_FORCE_SYNC
    /* Wait an appropriate time before proceeding */
    gSystem->Sleep(100); 
    return ret;
}

CAEN_DGTZ_ErrorCode  SwitchOffLogic(int handle, int N_CH) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
uint32_t reg_data;
	
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" Previously in  0x8000: %08X \n", reg_data);
	reg_data = reg_data &~ (1<<2);
	ret = CAEN_DGTZ_WriteRegister(handle, 0x8000, reg_data);	
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" In  0x8000: %08X \n", reg_data);
	
	uint32_t CoinceLogicAddress[MAX_CH] = { 0x1080, 0x1180, 0x1280, 0x1380, 0x1480, 0x1580, 0x1680, 0x1780,
																			0x1880, 0x1980, 0x1A80, 0x1B80, 0x1C80, 0x1D80, 0x1E80, 0x1F80};
																			
	//switch on coincidence for every channel and set trigger latency
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %04X \n", CoinceLogicAddress[i], reg_data);
		reg_data = reg_data &~ (1<<18);
		ret = CAEN_DGTZ_WriteRegister(handle, CoinceLogicAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", CoinceLogicAddress[i], reg_data);
	}
	
  return ret;
}

CAEN_DGTZ_ErrorCode  SetLogic(int32_t handle, uint32_t reg_val[2][8], int N_CH) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    
	uint32_t log_address[2][8] = { {0x1084, 0x1284, 0x1484, 0x1684, 0x1884, 0x1A84, 0x1C84, 0x1E84},
													  {0x8180, 0x8184, 0x8188, 0x818C, 0x8190, 0x8194, 0x8198, 0x819C} 
													};  
	
	uint32_t CoinceLogicAddress[MAX_CH] = { 0x1080, 0x1180, 0x1280, 0x1380, 0x1480, 0x1580, 0x1680, 0x1780,
																			0x1880, 0x1980, 0x1A80, 0x1B80, 0x1C80, 0x1D80, 0x1E80, 0x1F80};
																			
 	uint32_t TriggerLatencyAddress[MAX_CH] = { 0x106C, 0x116C, 0x126C, 0x136C, 0x146C, 0x156C, 0x166C, 0x176C,
																			0x186C, 0x196C, 0x1A6C, 0x1B6C, 0x1C6C, 0x1D6C, 0x1E6C, 0x1F6C};					
																			
	uint32_t ShapedTriggerWidthAddress[MAX_CH] = { 0x1070, 0x1170, 0x1270, 0x1370, 0x1470, 0x1570, 0x1670, 0x1770,
																			0x1870, 0x1970, 0x1A70, 0x1B70, 0x1C70, 0x1D70, 0x1E70, 0x1F70};															
	
	int NCouple = (int)(N_CH/2);
	
	uint32_t reg_data;
	
	//switch on coincidence on board
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" Previously in  0x8000: %08X \n", reg_data);
	reg_data = reg_data | (1<<2);
	ret = CAEN_DGTZ_WriteRegister(handle, 0x8000, reg_data);	
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" In  0x8000: %08X \n", reg_data);
	
	//switch on coincidence for every channel
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %04X \n", CoinceLogicAddress[i], reg_data);
		reg_data = reg_data | (1<<18);
		ret = CAEN_DGTZ_WriteRegister(handle, CoinceLogicAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", CoinceLogicAddress[i], reg_data);
	}
	
	//read shaped trigger width for every channel ~ time window for coincidence
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, ShapedTriggerWidthAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %08X \n", ShapedTriggerWidthAddress[i], reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, ShapedTriggerWidthAddress[i], 0x40);	 // 0x14 = 20x8ns = 160 ns window | 0x40 = 64x8ns = 512 ns window
		ret = CAEN_DGTZ_ReadRegister(handle, ShapedTriggerWidthAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", ShapedTriggerWidthAddress[i], reg_data);
	}
	
	//set trigger latency for every channel
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, TriggerLatencyAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %08X \n", TriggerLatencyAddress[i], reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, TriggerLatencyAddress[i], 0x9);	 // 9x8ns = 72 ns latency
		ret = CAEN_DGTZ_ReadRegister(handle, TriggerLatencyAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", TriggerLatencyAddress[i], reg_data);
	}
		
	for (int i=0; i<2; i++)
		for (int j=0; j<NCouple; j++){
			if (reg_val[i][j] != 0){
				printf("log_address[%i][%i] = 0x%04X  log_val[%i][%i] = 0x%04X\n", i, j, log_address[i][j], i, j, reg_val[i][j]);
				ret = CAEN_DGTZ_ReadRegister(handle, log_address[i][j], &reg_data);
				printf(" Previously in  0x%04X: %08X \n", log_address[i][j], reg_data);
				if (i==0){
				  reg_data = (reg_data & ~0xFF) | reg_val[i][j];
				}
				else
				  reg_data = reg_val[i][j];
			  
				ret = CAEN_DGTZ_WriteRegister(handle, log_address[i][j], reg_data);	 
				ret = CAEN_DGTZ_ReadRegister(handle, log_address[i][j], &reg_data);
				printf(" In  0x%04X: %08X \n", log_address[i][j], reg_data);
			}
		}
	    
    return ret;
}

CAEN_DGTZ_ErrorCode  SetInputRange(int handle, int channel, CAEN_DGTZ_InputRange_t InputRange){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	uint32_t d32;
	uint32_t InputDynamicRangeAddress[MAX_CH] = { 0x1028, 0x1128, 0x1228, 0x1328, 0x1428, 0x1528, 0x1628, 0x1728,
												  0x1828, 0x1928, 0x1A28, 0x1B28, 0x1C28, 0x1D28, 0x1E28, 0x1F28};
		
	ret = CAEN_DGTZ_ReadRegister(handle, InputDynamicRangeAddress[channel], &d32);
	//printf("Previously in 0x%4x %i\n",InputDynamicRangeAddress[channel], d32);
    if(ret)
    	return ret;
	
	ret = CAEN_DGTZ_WriteRegister(handle, InputDynamicRangeAddress[channel], InputRange);
	
	return ret;
}

CAEN_DGTZ_ErrorCode  WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    uint32_t d32 = 0xFFFFFFFF;

    ret = CAEN_DGTZ_ReadRegister(handle, address, &d32);
    if(ret != CAEN_DGTZ_Success)
        return ret;

    data &= mask;
    d32 &= ~mask;
    d32 |= data;
    ret = CAEN_DGTZ_WriteRegister(handle, address, d32);
    return ret;
}

 CAEN_DGTZ_ErrorCode CheckBoardFailureStatus(int handle) {

	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	uint32_t status = 0;
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8104, &status);
	if (ret != 0) {
		printf("Error: Unable to read board failure status.\n");
		return ret;
	}

	usleep(200*1000);

	//read twice (first read clears the previous status)
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8104, &status);
	if (ret != 0) {
		printf("Error: Unable to read board failure status.\n");
		return ret;
	}

	if(!(status & (1 << 7))) {
		printf("Board error detected: PLL not locked.\n");
		return ret;
	}

	return ret;
}

CAEN_DGTZ_ErrorCode SetDPPParameters(int handle, DigitizerConfig_t Dcfg){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
	CAEN_DGTZ_DPP_PSD_Params_t DPPParams;
	
	 for(int i=0; i<Dcfg.Nch; i++) {
		 DPPParams.thr[i] = Dcfg.thr[i];
		 DPPParams.nsbl[i] = Dcfg.nsbl[i];
		 DPPParams.pgate[i] = Dcfg.pgate[i];
		 DPPParams.sgate[i] = Dcfg.sgate[i];
		 DPPParams.lgate[i] = Dcfg.lgate[i];
		 DPPParams.selft[i] = Dcfg.selft[i];
		 DPPParams.trgc[i] = Dcfg.trgc[i];
		 DPPParams.discr[i] = Dcfg.discr[i];
		 DPPParams.cfdd[i] = Dcfg.cfdd[i];
		 DPPParams.cfdf[i] = Dcfg.cfdf[i];
		 DPPParams.tvaw[i] = Dcfg.tvaw[i];
		 DPPParams.csens[i] = Dcfg.csens[i];
	 }	 
		DPPParams.purh = Dcfg.purh;
		DPPParams.purgap = Dcfg.purgap;
		DPPParams.blthr = Dcfg.blthr;
		//DPPParams.bltmo = Dcfg.bltmo;
		DPPParams.trgho = Dcfg.trgho;
	
    // Set the DPP specific parameters for the channels in the given channelMask
    ret = CAEN_DGTZ_SetDPPParameters(handle, Dcfg.ChannelMask, &DPPParams);
		
	if (ret) {
        printf("ERROR: can't set DPP Parameters.\n");
        return ret;
    }
	
	return ret;
}	

CAEN_DGTZ_ErrorCode ProgramDigitizer(int handle, DigitizerConfig_t Dcfg){ 
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
       /* Reset the digitizer */
    ret = CAEN_DGTZ_Reset(handle);

    if (ret) {
        printf("ERROR: can't reset the digitizer.\n");
        return ret;
    }
	
	
  // Set the DPP acquisition mode
  //  This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
  // CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly        Only charge (DPP-PSD/DPP-CI v2) is returned
  //  CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
  //  CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime    Both charge and time are returned
  //  CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned 
    ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle, Dcfg.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
    // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
    ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);

    // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
    ret = CAEN_DGTZ_SetIOLevel(handle, Dcfg.IOlev);

    // Set the digitizer's behaviour when an external trigger arrives:

    //CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    //CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    //CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    //CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger

    //see CAENDigitizer user manual, chapter "Trigger configuration" for details 
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    // Set the enabled channels
    ret = CAEN_DGTZ_SetChannelEnableMask(handle, Dcfg.ChannelMask);

    // Set how many events to accumulate in the board memory before being available for readout
    // ret = CAEN_DGTZ_SetDPPEventAggregation(handle, Dcfg.EventAggr, 0); //MARK 1: SET BOTH ZERO, see line below
	ret = CAEN_DGTZ_SetDPPEventAggregation(handle, 0, 0); 
    
    // Set the mode used to syncronize the acquisition between different boards.
    //In this example the sync is disabled 
    ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
	//ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_TrgOutSinDaisyChain);
    
	CAEN_DGTZ_DPP_PSD_Params_t DPPParams;
	
	 for(int i=0; i<Dcfg.Nch; i++) {
		 DPPParams.thr[i] = Dcfg.thr[i];
		 DPPParams.nsbl[i] = Dcfg.nsbl[i];
		 DPPParams.pgate[i] = Dcfg.pgate[i];
		 DPPParams.sgate[i] = Dcfg.sgate[i];
		 DPPParams.lgate[i] = Dcfg.lgate[i];
		 DPPParams.selft[i] = Dcfg.selft[i];
		 DPPParams.trgc[i] = Dcfg.trgc[i];
		 DPPParams.discr[i] = Dcfg.discr[i];
		 DPPParams.cfdd[i] = Dcfg.cfdd[i];
		 DPPParams.cfdf[i] = Dcfg.cfdf[i];
		 DPPParams.tvaw[i] = Dcfg.tvaw[i];
		 DPPParams.csens[i] = Dcfg.csens[i];
	 }	 
		DPPParams.purh = Dcfg.purh;
		DPPParams.purgap = Dcfg.purgap;
		DPPParams.blthr = Dcfg.blthr;
		//DPPParams.bltmo = Dcfg.bltmo; // it occurs in DPPParams structure but didn't being used  in example for SetDPPParams()
		DPPParams.trgho = Dcfg.trgho;
	
    // Set the DPP specific parameters for the channels in the given channelMask
    ret = CAEN_DGTZ_SetDPPParameters(handle, Dcfg.ChannelMask, &DPPParams);
    
    for(int i=0; i<Dcfg.Nch; i++) {
        if (Dcfg.ChannelMask & (1<<i)) {
            // Set the number of samples for each waveform 
			//for x725 and x730 Recordlength is common to paired channels (you can set different RL for different channel pairs)
			if (i % 2 == 0)
				ret = CAEN_DGTZ_SetRecordLength(handle, Dcfg.RecordLength[i], i);

            // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
            ret = CAEN_DGTZ_SetChannelDCOffset(handle, i, Dcfg.DCOffset[i]); //0x8000
            
            // Set the PreTrigger size (in samples)
            ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, Dcfg.PreTrigger[i]);  //30
            
            // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
            ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Dcfg.PulsePolarity[i]);
        }
    }

    /* Set the virtual probes

    DPP-PSD for x725 and x730 boards can save:
    2 analog waveforms:
        Analog Trace 1: it can be specified with the ANALOG_TRACE_1 parameter;
        Analog Trace 2: it can be specified with the ANALOG_TRACE_2 parameter
    2 digital waveforms:
        Digital Trace 1:   it can be specified with the DIGITAL_TRACE_1 parameters
        Digital Trace 2:   it can be specified with the DIGITAL_TRACE_2 parameters

	Virtual Probes types for Trace 1:
	CAEN_DGTZ_DPP_VIRTUALPROBE_Input        -> Save the Input signal waveform 
	CAEN_DGTZ_DPP_VIRTUALPROBE_CFD          -> Save the CFD waveform (only if discrimination mode is  CAEN_DGTZ_DPP_DISCR_MODE_CFD)

    Virtual Probes types for Trace 2:
        CAEN_DGTZ_DPP_VIRTUALPROBE_Baseline     -> Save the Baseline waveform (mean on nsbl parameter)
    	CAEN_DGTZ_DPP_VIRTUALPROBE_CFD          -> Save the CFD waveform (only if discrimination mode is  CAEN_DGTZ_DPP_DISCR_MODE_CFD)
		CAEN_DGTZ_DPP_VIRTUALPROBE_None			-> Dual trace is disabled

    Digital Probes types for Digital Trace 1(x725, x730):
        CAEN_DGTZ_DPP_DIGITALPROBE_Gate   -> GateLong waveform
        CAEN_DGTZ_DPP_DIGITALPROBE_OverThr
        CAEN_DGTZ_DPP_DIGITALPROBE_TRGOut
        CAEN_DGTZ_DPP_DIGITALPROBE_CoincWin
        CAEN_DGTZ_DPP_DIGITALPROBE_PileUp
		CAEN_DGTZ_DPP_DIGITALPROBE_Coincidence
		CAEN_DGTZ_DPP_DIGITALPROBE_Trigger

	Digital Probes types for Digital Trace 2(x725)/3(x730):
		CAEN_DGTZ_DPP_DIGITALPROBE_GateShort
		CAEN_DGTZ_DPP_DIGITALPROBE_OverThr
		CAEN_DGTZ_DPP_DIGITALPROBE_TRGVal
		CAEN_DGTZ_DPP_DIGITALPROBE_TRGHoldoff
		CAEN_DGTZ_DPP_DIGITALPROBE_PileUp
		CAEN_DGTZ_DPP_DIGITALPROBE_Coincidence
		CAEN_DGTZ_DPP_DIGITALPROBE_Trigger
*/

	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Baseline);
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Gate);
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_2, CAEN_DGTZ_DPP_DIGITALPROBE_GateShort);
    if (ret) {
        printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
        return ret;
    } else {
        return ret;
    }
}


int32_t BoardSupportsCalibration(CAEN_DGTZ_BoardInfo_t BoardInfo) {
    return
		BoardInfo.FamilyCode == CAEN_DGTZ_XX761_FAMILY_CODE ||
        BoardInfo.FamilyCode == CAEN_DGTZ_XX751_FAMILY_CODE ||
        BoardInfo.FamilyCode == CAEN_DGTZ_XX730_FAMILY_CODE ||
        BoardInfo.FamilyCode == CAEN_DGTZ_XX725_FAMILY_CODE;
}


int32_t BoardSupportsTemperatureRead(CAEN_DGTZ_BoardInfo_t BoardInfo) {
    return
        BoardInfo.FamilyCode == CAEN_DGTZ_XX751_FAMILY_CODE ||
        BoardInfo.FamilyCode == CAEN_DGTZ_XX730_FAMILY_CODE ||
        BoardInfo.FamilyCode == CAEN_DGTZ_XX725_FAMILY_CODE;
}



CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	    ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
		ret = CAEN_DGTZ_FreeDPPEvents(handle, Events);
		ret = CAEN_DGTZ_FreeDPPWaveforms(handle, Waveforms);
				
		exit(0);
		return ret;
}

void ReturnErrorHandler(CAEN_DGTZ_ErrorCode ret, char* CName, int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms){

	if (ret) {
				printf("%s ret = %i\n", CName, ret);
				//sprintf(CName, "ERR_READ_DATA");
				new TGMsgBox(gClient->GetRoot(), Rcfg.main, "Error", Form("%s \n ret = %i \n", CName, ret), kMBIconStop, kMBOk);
				ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
			}
}

void SetDefaultConfiguration(DigitizerConfig_t *Dcfg) { //CAEN_DGTZ_DPP_PSD_Params_t *DPPParams
    
    
	Dcfg->EventAggr = 1; // 1023
	Dcfg->ChannelMask = 0x3FF;
	
	
	Dcfg->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed; //MARK 2: Switch to mode LIST to discard traces for high rates
	Dcfg->IOlev = CAEN_DGTZ_IOLevel_NIM;
	
	Dcfg->Nch = MAX_CH; // v1730
		
	for(int ch=0; ch<Dcfg->Nch; ch++) {
		Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityPositive; //Negative //Positive
		Dcfg->RecordLength[ch] = 500;
		Dcfg->PreTrigger[ch] = 50;
		Dcfg->DCOffset[ch] = 0x199A; //10%
		
		//
        Dcfg->thr[ch] = 100;        // Trigger Threshold
        // The following parameter is used to specifiy the number of samples for the baseline averaging:
        //0 -> absolute Bl
        //1 -> 16samp
        //2 -> 64samp
        //3 -> 256samp
        //4 -> 1024samp
        Dcfg->nsbl[ch] = 1;
        Dcfg->lgate[ch] = 500;    // Long Gate Width (N*2ns for x730  and N*4ns for x725)
        Dcfg->sgate[ch] = 124;    // Short Gate Width (N*2ns for x730  and N*4ns for x725)
        Dcfg->pgate[ch] = 8;     // Pre Gate Width (N*2ns for x730  and N*4ns for x725) 
        // Self Trigger Mode:
        //0 -> Disabled
        //1 -> Enabled 
        Dcfg->selft[ch] = 1; // before 1
        // Trigger configuration:
        //    CAEN_DGTZ_DPP_TriggerMode_Normal ->  Each channel can self-trigger independently from the other channels
		//    CAEN_DGTZ_DPP_TriggerMode_Coincidence -> A validation signal must occur inside the shaped trigger coincidence window
        //DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerMode_Normal;
			
		//DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerConfig_Peak;
		Dcfg->trgc[ch] = CAEN_DGTZ_DPP_TriggerConfig_Threshold;

		//Discrimination mode for the event selection 
		//CAEN_DGTZ_DPP_DISCR_MODE_LED -> Leading Edge Distrimination
		//CAEN_DGTZ_DPP_DISCR_MODE_CFD -> Constant Fraction Distrimination
		Dcfg->discr[ch] = CAEN_DGTZ_DPP_DISCR_MODE_LED;

		//CFD delay (N*2ns for x730  and N*4ns for x725)  
		Dcfg->cfdd[ch] = 4;  

		//CFD fraction: 0->25%; 1->50%; 2->75%; 3->100% 
		Dcfg->cfdf[ch] = 0;

        // Trigger Validation Acquisition Window
        Dcfg->tvaw[ch] = 250; // in bins 1bin = 2 ns

        // Charge sensibility: 
		//Options for Input Range 2Vpp: 0->5fC/LSB; 1->20fC/LSB; 2->80fC/LSB; 3->320fC/LSB; 4->1.28pC/LSB; 5->5.12pC/LSB 
		//Options for Input Range 0.5Vpp: 0->1.25fC/LSB; 1->5fC/LSB; 2->20fC/LSB; 3->80fC/LSB; 4->320fC/LSB; 5->1.28pC/LSB */
        Dcfg->csens[ch] = 0;
	}
    //Pile-Up rejection Mode
    //CAEN_DGTZ_DPP_PSD_PUR_DetectOnly -> Only Detect Pile-Up
    //CAEN_DGTZ_DPP_PSD_PUR_Enabled -> Reject Pile-Up */
    Dcfg->purh = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly;
    Dcfg->purgap = 100;  // Purity Gap in LSB units (1LSB = 0.12 mV for 2Vpp Input Range, 1LSB = 0.03 mV for 0.5 Vpp Input Range )
    Dcfg->blthr = 3;     // Baseline Threshold
    Dcfg->trgho = 400;     //in 8  ns steps - !!!BULLSHIT here value already in ns!!! Trigger HoldOff 
}


void ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg, ReadoutConfig_t *Rcfg) // CAEN_DGTZ_DPP_PSD_Params_t *DPPParams) 
{
	char str[1000], str1[1000], *pread = NULL;
	int i, ch=-1, val, Off=0, tr = -1;
    int RL_val, thr_val;
     

	// Default settings 
	SetDefaultConfiguration(Dcfg);


	// read config file and assign parameters 
	while(!feof(f_ini)) {
		int read;
        char *res = NULL;
        // read a word from the file
        read = fscanf(f_ini, "%s", str);
        if( !read || (read == EOF) || !strlen(str))
			continue;
        // skip comments
        if(str[0] == '#') {
            res = fgets(str, 1000, f_ini);
			continue;
        }

        if (strcmp(str, "@ON")==0) {
            Off = 0;
            continue;
        }
		if (strcmp(str, "@OFF")==0)
            Off = 1;
        if (Off)
            continue;

       
 
        // PID: Digitizer physical Identification number M - master, S - slave
		if (strstr(str, "PID_M")!=NULL) {
			read = fscanf(f_ini, "%d", &Dcfg->PID_M);
			continue;
		}

		if (strstr(str, "PID_S")!=NULL) {
			read = fscanf(f_ini, "%d", &Dcfg->PID_S);
			continue;
		}
			
		// NB: number of Digitizers 
		if (strstr(str, "N_BOARDS")!=NULL) {
			read = fscanf(f_ini, "%d", &Rcfg->N_B);
			continue;
		}

		// Nch: Digitizer channels to work
		if (strstr(str, "N_CH")!=NULL) {
			read = fscanf(f_ini, "%d", &Dcfg->Nch);
			continue;
		}
		
		// BL_CUT: number of samples  for baseline calculation
		if (strstr(str, "BL_CUT")!=NULL) {
			read = fscanf(f_ini, "%d", &Dcfg->BL_CUT);
			continue;
		}
        // Acquisition Record Length (number of samples)
		//if (strstr(str, "RECORD_LENGTH")!=NULL) {
		//	read = fscanf(f_ini, "%d", &RL_val);
		//	for (int ch=0; ch<Dcfg->Nch; ch++)	
		//		Dcfg->RecordLength[ch] = RL_val;
		//	continue;
		//}
        
		if (strstr(str, "RECORD_LENGTH_01")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[0] = RL_val;
			Dcfg->RecordLength[1] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_23")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[2] = RL_val;
			Dcfg->RecordLength[3] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_45")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[4] = RL_val;
			Dcfg->RecordLength[5] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_67")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[6] = RL_val;
			Dcfg->RecordLength[7] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_89")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[8] = RL_val;
			Dcfg->RecordLength[9] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_1011")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[10] = RL_val;
			Dcfg->RecordLength[11] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_1213")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[12] = RL_val;
			Dcfg->RecordLength[13] = RL_val;
			continue;
		}
		
		if (strstr(str, "RECORD_LENGTH_1415")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			Dcfg->RecordLength[14] = RL_val;
			Dcfg->RecordLength[15] = RL_val;
			continue;
		}
		
        // EventAggr 
		if (strstr(str, "EVENT_AGGR")!=NULL) {
			read = fscanf(f_ini, "%d", &Dcfg->EventAggr);
			continue;
		}
			
	 ///Input polarity	
	 
		if (strstr(str, "PULSE_POLARITY")!=NULL) {
			
			read = fscanf(f_ini, "%s", str1);
			if (strcmp(str1, "POSITIVE") == 0)
				for (int ch=0; ch<Dcfg->Nch; ch++)
					Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityPositive;
			else if (strcmp(str1, "NEGATIVE") == 0)
				for (int ch=0; ch<Dcfg->Nch; ch++)		
					Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityNegative;
			else
				printf("%s: Invalid Parameter\n", str);

						
			continue;
		}
		
		// Threshold
		if (strstr(str, "TRIGGER_THRESHOLD")!=NULL) {
			read = fscanf(f_ini, "%d", &thr_val);
			for (int ch=0; ch<Dcfg->Nch; ch++)	
				Dcfg->thr[ch] = thr_val;
		}
        // Front Panel LEMO I/O level (NIM, TTL)
		
		if (strstr(str, "IO_LEVEL")!=NULL) {
			read = fscanf(f_ini, "%s", str1);
			if (strcmp(str1, "TTL")==0)
				Dcfg->IOlev = CAEN_DGTZ_IOLevel_TTL; //1;
			if (strcmp(str1, "NIM")==0)
				Dcfg->IOlev = CAEN_DGTZ_IOLevel_NIM;
			//printf("%s: invalid option\n", str);
			continue;
		}
		
       
	}
		
}
