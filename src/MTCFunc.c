#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
#include "MTCFunc.h"


#include "TROOT.h"

#include <TSystem.h>
//#include "TH1D.h"
//#include "TH2D.h"

//#include "TROOT.h"
//#include "TApplication.h"
#include "TCanvas.h"
#include "TStyle.h"

#define CAEN_USE_DIGITIZERS
//#define IGNORE_DPP_DEPRECATED


extern DigitizerConfig_t Dcfg;
extern ReadoutConfig_t Rcfg;
//extern int loop;
extern int handle;
	extern char *buffer;
		
	extern CAEN_DGTZ_DPP_PSD_Event_t   *Events[MAX_CH];  // events buffer
	extern CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveforms;         // waveforms buffer
		
	
	extern TCanvas *c1;
	Color_t color[16] = {kBlue, kRed, kViolet, kGreen+1, kPink-9, kOrange, kMagenta, kCyan-7, kGray, kBlack, kBlue, kRed, kGreen, kOrange-2, kBlack, kOrange+2}; 	
	
using namespace std;

long get_time()
{
    long time_ms;
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;

    return time_ms;
}

void CalcRate(int N_CH, uint64_t &PrevRateTime){
	char CName[100];
	uint64_t CurrentTime, ElapsedTime, DAQTime;
			CurrentTime = get_time();
        	ElapsedTime = CurrentTime - PrevRateTime;
			DAQTime = (CurrentTime - Rcfg.StartTime) / 1000;
			if (Rcfg.fTimer && DAQTime > Rcfg.timer_val)
				Rcfg.loop = 0;
					        	
			if (ElapsedTime > 1000) { // 1000
				sprintf(CName,"T: %li s",  DAQTime );
				//printf("%s \n", CName);
				Rcfg.TLabel->SetText(CName);
				gSystem->ProcessEvents(); 
				
            	if (Rcfg.Nb != 0){
					sprintf(CName,"Read. %.2f MB/s ", (float)Rcfg.Nb/((float)ElapsedTime*1048.576f) );
					//printf("%s \n", CName);
					Rcfg.StatusBar->SetText(CName, 0);
					
					for (int ch=0; ch<N_CH; ch++) { //8
						if (Rcfg.TrgCnt[ch] != 0){
							sprintf(CName, "CH[%i]: %.2f Hz ", ch, (float)Rcfg.TrgCnt[ch]*1000.0f/(float)ElapsedTime);
							//printf("%s \n", CName);
							if (ch < 15)
								Rcfg.StatusBar->SetText(CName, ch+1);
						}
						else{
							sprintf(CName, "No data...");
							//printf("%s \n", CName);
							if (ch < 15)
								Rcfg.StatusBar->SetText(CName, ch+1);
						}
						Rcfg.TrgCnt[ch] = 0;
					}
				}
									
            	else{
						//printf("No data...\n");
						sprintf(CName, "No data...");
						Rcfg.StatusBar->SetText(CName, 0);
				}
								
            Rcfg.Nb = 0;
            		
            PrevRateTime = CurrentTime;
			gSystem->ProcessEvents(); 
        	}
	
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
		ret = CAEN_DGTZ_WriteRegister(handle, ShapedTriggerWidthAddress[i], 0x14);	 // 0x14 = 20x8ns = 160 ns window
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
    ret = CAEN_DGTZ_SetDPPEventAggregation(handle, Dcfg.EventAggr, 0);
    
    // Set the mode used to syncronize the acquisition between different boards.
    //In this example the sync is disabled 
    ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
    
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


void SetDefaultConfiguration(DigitizerConfig_t *Dcfg) { //CAEN_DGTZ_DPP_PSD_Params_t *DPPParams
    
    
	Dcfg->EventAggr = 1; // 1023
	Dcfg->ChannelMask = 0x3FF;
	
	
	Dcfg->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
	Dcfg->IOlev = CAEN_DGTZ_IOLevel_NIM;
	
	Dcfg->Nch = MAX_CH; // v1730
	
	for(int ch=0; ch<Dcfg->Nch; ch++) {
		Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityPositive; //Negative //Positive
		Dcfg->RecordLength[ch] = 500;
		Dcfg->PreTrigger[ch] = 50;
		Dcfg->DCOffset[ch] = 0x199A; //10%
		
			// before were in CAEN_DGTZ_DPP_PSD_Params_t *DPPParams
			//
            Dcfg->thr[ch] = 100;        // Trigger Threshold
            /* The following parameter is used to specifiy the number of samples for the baseline averaging:
            0 -> absolute Bl
            1 -> 16samp
            2 -> 64samp
            3 -> 256samp
            4 -> 1024samp */
            Dcfg->nsbl[ch] = 1;
            Dcfg->lgate[ch] = 500;    // Long Gate Width (N*2ns for x730  and N*4ns for x725)
            Dcfg->sgate[ch] = 124;    // Short Gate Width (N*2ns for x730  and N*4ns for x725)
            Dcfg->pgate[ch] = 8;     // Pre Gate Width (N*2ns for x730  and N*4ns for x725) 
            /* Self Trigger Mode:
            0 -> Disabled
            1 -> Enabled */
            Dcfg->selft[ch] = 1;
            // Trigger configuration:
            //    CAEN_DGTZ_DPP_TriggerMode_Normal ->  Each channel can self-trigger independently from the other channels
			//    CAEN_DGTZ_DPP_TriggerMode_Coincidence -> A validation signal must occur inside the shaped trigger coincidence window
            //DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerMode_Normal;
			
			//DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerConfig_Peak;
			Dcfg->trgc[ch] = CAEN_DGTZ_DPP_TriggerConfig_Threshold;

			/*Discrimination mode for the event selection 
			CAEN_DGTZ_DPP_DISCR_MODE_LED -> Leading Edge Distrimination
			CAEN_DGTZ_DPP_DISCR_MODE_CFD -> Constant Fraction Distrimination*/
			Dcfg->discr[ch] = CAEN_DGTZ_DPP_DISCR_MODE_LED;

			/*CFD delay (N*2ns for x730  and N*4ns for x725)  */
			Dcfg->cfdd[ch] = 4;  

			/*CFD fraction: 0->25%; 1->50%; 2->75%; 3->100% */
			Dcfg->cfdf[ch] = 0;

            /* Trigger Validation Acquisition Window */
            Dcfg->tvaw[ch] = 250;

            /* Charge sensibility: 
			Options for Input Range 2Vpp: 0->5fC/LSB; 1->20fC/LSB; 2->80fC/LSB; 3->320fC/LSB; 4->1.28pC/LSB; 5->5.12pC/LSB 
			Options for Input Range 0.5Vpp: 0->1.25fC/LSB; 1->5fC/LSB; 2->20fC/LSB; 3->80fC/LSB; 4->320fC/LSB; 5->1.28pC/LSB */
            Dcfg->csens[ch] = 0;
        }
        /* Pile-Up rejection Mode
        CAEN_DGTZ_DPP_PSD_PUR_DetectOnly -> Only Detect Pile-Up
        CAEN_DGTZ_DPP_PSD_PUR_Enabled -> Reject Pile-Up */
        Dcfg->purh = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly;
        Dcfg->purgap = 100;  // Purity Gap in LSB units (1LSB = 0.12 mV for 2Vpp Input Range, 1LSB = 0.03 mV for 0.5 Vpp Input Range )
        Dcfg->blthr = 3;     // Baseline Threshold
        Dcfg->trgho = 8;     //8   // Trigger HoldOff
    
	
}


void ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg) // CAEN_DGTZ_DPP_PSD_Params_t *DPPParams) 
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

       
 
        // PID: Digitizer physical Identification number
		if (strstr(str, "PID")!=NULL) {
			read = fscanf(f_ini, "%d", &Dcfg->PID);
			continue;
		}
		
		
        // Acquisition Record Length (number of samples)
		if (strstr(str, "RECORD_LENGTH")!=NULL) {
			read = fscanf(f_ini, "%d", &RL_val);
			for (int ch=0; ch<Dcfg->Nch; ch++)	
				Dcfg->RecordLength[ch] = RL_val;
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

/////////////////////////////////////////////
////READOUT FUNCTIONS
/////////////////////////////////////////////


void InitReadoutConfig(ReadoutConfig_t *Rcfg, int N_CH){
	
	Rcfg->fPrint = false; // common print flag for debug
	Rcfg->fStoreTraces = false; // flag to store traces
	Rcfg->fTimer = false; // flag for timer usage
	
	Rcfg->loop = -1; // ReadoutLoop flag
	Rcfg->DrawTime = 0.5; // time between histograms drawing in sec
	for (int i = 0; i<N_CH; i++)
		Rcfg->TrgCnt[i] = 0;
	
}

void InitHisto(Histograms_t *Histo, uint32_t RecordLength[MAX_CH], int N_CH){
	char str[100];
	
	Histo->WF_XMIN = 0, Histo->WF_XMAX = RecordLength[0] * b_width;
	Histo->WF_YMIN = -500, Histo->WF_YMAX = 1000;
	Histo->ALBound = 0, Histo->ARBound = RecordLength[0] * b_width;
	Histo->ILBound = 0, Histo->IRBound = RecordLength[0] * b_width;
	Histo->fBL = true;
	Histo->fTrace = true;
	
	for (int i=0; i<N_CH; i++){
		sprintf(str,"h_trace%i", i);
		Histo->trace[i] = new TH1D(str, str, RecordLength[i], 0, RecordLength[i] * b_width);
		sprintf(str, "h_ampl%i", i);
		Histo->ampl[i]= new TH1D(str, str, 16384, 0, 16384);
		sprintf(str, "h_integral%i", i);
		Histo->integral[i]= new TH1D(str, str, 100000, 0, 100000);
		sprintf(str, "h_charge%i", i);
		Histo->charge[i]= new TH1D(str, str, 1000, 0, 65536);
		
		Histo->trace[i]->SetLineColor(color[i]);
		Histo->ampl[i]->SetLineColor(color[i]);
		Histo->integral[i]->SetLineColor(color[i]);
		Histo->charge[i]->SetLineColor(color[i]);
		
		Histo->fDraw[i] = true;
	}
	
	Histo->FirstToDraw = 0;
	Histo->NPad = 1;
	Histo->CH_2D = 0;
	Histo->PSD_BIN = 2;
	
	Histo->trace[0]->GetXaxis( )->SetRangeUser(Histo->WF_XMIN, Histo->WF_XMAX);
	Histo->trace[0]->GetXaxis( )->SetTitle(" Time, ns");
	Histo->trace[0]->GetYaxis( )->SetRangeUser(Histo->WF_YMIN, Histo->WF_YMAX);
	Histo->trace[0]->GetYaxis( )->SetTitleOffset(1.1);
	Histo->trace[0]->GetYaxis( )->SetTitle(" Channels, lbs"); 
	
	Histo->dt = new TH1D("h_dt", "h_dt", 1000, 0, 1000);
	
	Histo->counts = new TH1D("h_counts", "h_counts", 16, 0, 16);
	Histo->layers = new TH1D("h_layers", "h_layers", 16, 0, 16);
	
	///TH2D	
	Histo->int_ampl = new TH2D("h_int_ampl", "h_int_ampl", 1000, 0, 16384, 1000, 0, 1000000);
	Histo->psd_ampl = new TH2D("h_psd_ampl", "h_psd_ampl", 1000, 0, 16384, 1000, 0, 1);
	Histo->psd_int = new TH2D("h_psd_int", "h_psd_int", 1000, 0, 1000000, 1000, 0, 1);
	Histo->qs_ql = new TH2D("h_qs_ql", "h_qs_ql", 1000, 0, 50000, 1000, 0, 100000);
	Histo->xy = new TH2D("h_xy", "h_xy", 16, 0, 16, 16, 0, 16);
	Histo->rubik = new TH2D("h_rubik", "h_rubik", 5, 5, 10, 5, 0, 5);
	
}

void DrawTH2D(bool flag, TH2D *h, int cPos, char *opt){

	if (flag){
		c1->cd(cPos);
		h->SetMarkerStyle(21);
		h->SetMarkerSize(0.4);
		h->SetMarkerColor(kBlue);
		h->Draw(opt);
	}		
	
}

void DrawHisto(Histograms_t Histo, int N_CH){
			
	if (Histo.fTrace){
		c1->cd(1);
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++){
			if ( Histo.fDraw[ch] ){
				Histo.trace[ch]->Draw(ch == Histo.FirstToDraw ? "HIST" : "HIST SAME");
				if (ch == Histo.FirstToDraw)
					Histo.trace[ch]->GetYaxis()->SetRangeUser(Histo.WF_YMIN, Histo.WF_YMAX);
			}
		}
	}	
		
	if (Histo.fAmpl){		
		c1->cd(Histo.cAmpl);
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++)
			if (Histo.fDraw[ch])
				Histo.ampl[ch]->Draw(ch == Histo.FirstToDraw ? "HIST" : "HIST SAME");
	}
	
	if (Histo.fInt){	
		c1->cd(Histo.cInt);
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++)
			if (Histo.fDraw[ch])
				Histo.integral[ch]->Draw(ch == Histo.FirstToDraw ? "HIST" : "HIST SAME");
	}
	
	if (Histo.fCharge){	
		c1->cd(Histo.cCharge);
		for (int ch = Histo.FirstToDraw; ch<N_CH; ch++)
			if (Histo.fDraw[ch])
				Histo.charge[ch]->Draw(ch == Histo.FirstToDraw ? "HIST" : "HIST SAME");
	}
	
	if (Histo.fdT == true){
		c1->cd(Histo.cdT);
		Histo.dt->SetLineColor(color[2]);
		Histo.dt->Draw("HIST");
		Histo.dt->GetXaxis()->SetTitle(" Time, ns");
	}
	
	DrawTH2D(Histo.fIA, Histo.int_ampl, Histo.cIA, Histo.h2Style);//Histo.h2Style
	DrawTH2D(Histo.fPSD_ampl, Histo.psd_ampl, Histo.cPSD_ampl, Histo.h2Style);
	DrawTH2D(Histo.fPSD_int, Histo.psd_int, Histo.cPSD_int, Histo.h2Style);
	DrawTH2D(Histo.fQsl, Histo.qs_ql, Histo.cQsl, Histo.h2Style);
	
	if (Histo.fLayers){
		c1->cd(Histo.cLayers);
		Histo.layers->Reset("ICESM");
		for (int i = Histo.FirstToDraw; i<N_CH; i++)
			Histo.layers->Fill( i, Histo.fLayersCoeff == false ?  Histo.ampl[i]->GetMean( ) : Histo.ampl[i]->GetMean( ) / Histo.layers_coeff[i] );
		
		Histo.layers->SetLineColor(kMagenta+3);
		Histo.layers->SetFillColor(kMagenta+3);
		Histo.layers->Draw("bar hist");
		Histo.layers->SetBarWidth(0.95);
	}
	
	if (Histo.fCounts){
		c1->cd(Histo.cCounts);
		Histo.counts->Reset("ICESM");
		for (int i = Histo.FirstToDraw; i<N_CH; i++)
			Histo.counts->Fill(i, Histo.ampl[i]->GetEntries( ) );
		
		Histo.counts->SetLineColor(kMagenta+3);
		Histo.counts->SetFillColor(kMagenta+3);
		Histo.counts->Draw("bar hist");
		Histo.counts->SetBarWidth(0.95);
	}	
	
	if (Histo.fXY){
		c1->cd(Histo.cXY);
		char str[20];
		sprintf(str,"Ecanvas1_%i", Histo.cXY);
		TPad *pad = (TPad*)c1->GetPrimitive(str);
		pad->SetGrid( );
		Histo.xy->Draw("COLZ");
		Histo.xy->GetXaxis( )->SetLabelSize(0.08);
		Histo.xy->GetYaxis( )->SetLabelSize(0.08);
	}
	
	if (Histo.fRubik){
		c1->cd(Histo.cRubik);
		char str[20];
		sprintf(str,"Ecanvas1_%i", Histo.cRubik);
		TPad *pad = (TPad*)c1->GetPrimitive(str);
		pad->SetGrid( );
		Histo.rubik->Draw("COLZ");
		Histo.rubik->GetXaxis( )->SetLabelSize(0.08);
		Histo.rubik->GetYaxis( )->SetLabelSize(0.08);
	}
			
	c1->Update( );
}

///
void FillHisto(int ch,  uint32_t ev, Histograms_t *Histo, double &ampl){ 
			
	int BL_CUT = 20; //fNumericEntries[1]->GetNumber();
		
	Double_t BL_mean = 0,  integral = 0;
	ampl = 0;
	Int_t m_stamp;
	Double_t psd_val =0, Qs = 0, Ql = 0;
		
	Int_t p = Dcfg.PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive ? 1: -1; //POLARITY
		
	vector <double> vec, vec_bl; 
	uint16_t *WaveLine;
	WaveLine = Waveforms->Trace1;
		
	Histo->trace[ch]->Reset("ICESM");	
			
		for (int j=0; j<(int)Waveforms->Ns; j++)
			vec_bl.push_back((double)WaveLine[j]);
	
		for ( int j=0; j<BL_CUT; j++)
			BL_mean = BL_mean + vec_bl[j];	
		BL_mean /= BL_CUT;	
		
		
		for ( int j=0; j<vec_bl.size( ); j++){
			vec.push_back(vec_bl[j] - BL_mean);
						
			if (vec[j] * p > ampl && j * b_width > Histo->ALBound && j * b_width < Histo->ARBound){
				ampl = vec[j] * p;
				m_stamp = j;
			}	
			
			if (j * b_width > Histo->ILBound && j * b_width < Histo->IRBound)
				integral += vec[j] * p;
		}
		
		if (Histo->fBL){
				for ( int j=0; j<vec.size( ); j++)
					Histo->trace[ch]->Fill(j * b_width, vec[j]);
			}
			else{
				for ( int j=0; j<vec_bl.size( ); j++)
					Histo->trace[ch]->Fill(j * b_width, vec_bl[j]);
			}	
				
		Histo->integral[ch]->Fill(integral);
		Histo->ampl[ch]->Fill(ampl);
		
		//PSD
		if ( ( (Histo->fPSD_ampl) || (Histo->fPSD_int) || (Histo->fQsl)  )  && (ch == Histo->CH_2D) ){
			for (int j=m_stamp; j<vec.size( ); j++){
				if (j<(m_stamp + Histo->PSD_BIN) )
					Qs = Qs + p * vec[j];
				Ql = Ql + p * vec[j];
			}
			psd_val = 1 - ( Qs/Ql );
			
			if (Histo->fPSD_ampl) 
				Histo->psd_ampl->Fill(ampl, psd_val);
			if (Histo->fPSD_int) 
				Histo->psd_int->Fill(integral, psd_val);	
			if (Histo->fQsl) 
				Histo->qs_ql->Fill(Ql, Qs);	
		}
		//PSD
		
		if (Histo->fCharge)
			Histo->charge[ch]->Fill(Events[ch][ev].ChargeLong);
		
		if (Histo->fIA && ch == Histo->CH_2D)
			Histo->int_ampl->Fill(ampl, integral);
		
		//if(Rcfg.fPrint)
		//	printf(" ev[%i]  ch[%i] ampl %f Qs %f Ql %f  PSD %f \n", ev, ch, ampl, Qs, Ql, psd_val);
		
		vec.clear();
		vec_bl.clear();
}

/////////////////////////////////////////////////////////////////////
/////END OF HISTOGRAMS FUNCTIONS
/////////////////////////////////////////////////////////////////////

void ReadoutLoop(int handle, int N_CH, Histograms_t *Histo ){	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;	
		
	uint32_t BufferSize, NumEvents[MAX_CH];	
	uint64_t PrevRateTime = get_time(), PrevDrawTime;
	double ampl[N_CH];
	vector <int> hits{};
	
	const char* xlabel[5] = {"5", "6", "7", "8", "9"};
	const char* ylabel[5] = {"0", "1", "2", "3", "4"};
	
	if (Histo->fRubik)
		for (Int_t i=0; i<5; i++)
			Histo->rubik->Fill(xlabel[i], ylabel[i], 0);
		
	while(Rcfg.loop == 1) {
		// Calculate throughput and trigger rate (every second) 		
		CalcRate(N_CH, PrevRateTime);			   
	  		   
		ret = CAEN_DGTZ_ReadData(handle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize); // Read the buffer from the digitizer 
		if (ret) {
			printf("ERR_READ_DATA \n");
			//sprintf(CName, "ERR_READ_DATA");
			//new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
			ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
        }
		
		//printf("BufferSize: %d\n", BufferSize);	
        		
        if (BufferSize == 0) 
			continue;
				
		Rcfg.Nb += BufferSize;
		
		ret = CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)&Events, NumEvents);
        if (ret) {
            //sprintf(CName, "GET_DPPEVENTS");
			printf("GET_DPPEVENTS \n");
			//new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
            ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
        }
        
		if (Rcfg.fPrint)
			printf(" ---------------------------------------- \n");			
		
		Rcfg.Nev = 0;
		hits.clear( );
//		
// previous channel based output
/*		
		for (int ch=0; ch<N_CH; ch++) { 
			ampl[ch] = 0;
			Histo->trace[ch]->Reset("ICESM");
			if (Dcfg.ChannelMask & (1<<ch) ){
				Rcfg.Nev +=(int)NumEvents[ch];
				for (uint32_t ev=0; ev<(int)NumEvents[ch]; ev++) {
				
					CAEN_DGTZ_DecodeDPPWaveforms(handle, (void**)&Events[ch][ev], Waveforms);
					Rcfg.TrgCnt[ch]++;
										
					FillHisto(ch, ev, Histo, ampl[ch]); // all data performance
														   					
					gSystem->ProcessEvents(); 
				} // events loop
		
			} // check enabled channels
			if ( ampl[ch] > 0 )
				hits.push_back(ch);
		}// channels loop
*/		
// previous channel based output
//				
//	
//num events based output
//
		int MaxNumEvents = 0;
		for(int ch=0; ch<N_CH; ch++) 
			if (Dcfg.ChannelMask & (1<<ch) ){
				if (MaxNumEvents<NumEvents[ch])
					MaxNumEvents = NumEvents[ch];
				//printf("NumEvents[%i] %i \n",ch, NumEvents[ch]);
			}    
		
		for (uint32_t ev=0; ev<MaxNumEvents; ev++) {
			for (int ch=0; ch<N_CH; ch++) {
			  
				if ( (Dcfg.ChannelMask & (1<<ch)) && (ev < NumEvents[ch]) ){
					if (ev ==0 )
						Rcfg.Nev +=(int)NumEvents[ch];
					ampl[ch] = 0;
					Histo->trace[ch]->Reset("ICESM");

				    CAEN_DGTZ_DecodeDPPWaveforms(handle, (void**)&Events[ch][ev], Waveforms);
					Rcfg.TrgCnt[ch]++;
					
					FillHisto(ch, ev, Histo, ampl[ch]); // all data performance

					if (Histo->fdT && ch == 1){
						uint64_t time_ps0 = ( ( (uint64_t)(Events[0][ev].Extras & 0xFFFF0000)<<16) + Events[0][ev].TimeTag ) * 1000 * 2 + 2 * (Events[0][ev].Extras &~ 0xFFFFFC00); // extra time in picoseconds
						uint64_t time_ps1 = ( ( (uint64_t)(Events[1][ev].Extras & 0xFFFF0000)<<16) + Events[1][ev].TimeTag ) * 1000 * 2 + 2 * (Events[1][ev].Extras &~ 0xFFFFFC00); // extra time in picoseconds
						
						uint32_t timetag0 = 2 * Events[0][ev].TimeTag;
						uint32_t timetag1 = 2 * Events[1][ev].TimeTag;
						uint32_t fine0 = 2 * (Events[0][ev].Extras &~ 0xFFFFFC00);
						uint32_t fine1 = 2 * (Events[1][ev].Extras &~ 0xFFFFFC00);
						double dt = (double)(time_ps1 - time_ps0) / 1000;
						if (Rcfg.fPrint)
							printf(" [0] %ld [1] %ld  [t0] %d [t1] %d [f0] %d [f1] %d dt %f ns  \n", time_ps0, time_ps1, timetag0, timetag1, fine0, fine1, dt); //dt/1000
						Histo->dt->Fill(dt);
				    }
									
					if (ampl[ch] > 0)
						hits.push_back(ch); // collect hits
					gSystem->ProcessEvents();
			    }// check enabled channels		
			} // channels loop
		}// events loop
	//	
	//num events based output
	
	
		if (Histo->fXY){
			if (Rcfg.fPrint)
				printf("Mult : %lu\n", hits.size( ) );

			if (hits.size() == 2){
				// Discard evts with ambiguous hits
				Histo->xy->Fill(hits[0], hits[1]);
				if (Rcfg.fPrint)
					printf("XY : %d %d\n", hits[0], hits[1]);
			}
		}		
		
		/*
		if (Histo->fdT){
			double dt = (Events[1][0].TimeTag > Events[0][0].TimeTag) ? (double)(Events[1][0].TimeTag - Events[0][0].TimeTag) : (double)(Events[0][0].TimeTag - Events[1][0].TimeTag);
			if (Rcfg.fPrint)
				printf(" [1] %d [0] %d dt %f ns  \n", Events[1][0].TimeTag, Events[0][0].TimeTag, dt); //dt/1000
			Histo->dt->Fill(dt);
		}
		*/
		
		if (Histo->fRubik && Rcfg.Nev!=0){
			Histo->rubik->Reset("ICESM" );
			
			for (int ch=0; ch<N_CH; ch++){
				if (ampl[ch] > 0 && ch < 5) {
					for (int i=0; i<5; i++)
						Histo->rubik->Fill(xlabel[i], ylabel[ch], 1);
				}
				if (ampl[ch] > 0 && ch >= 5) {
					for (int i=0; i<5; i++)
						Histo->rubik->Fill(xlabel[ch-5], ylabel[i], 1);
				}
			}
		}	
		
		// check elapsed time before drawing
		if ( (get_time() - PrevDrawTime) >= Rcfg.DrawTime*1000 && Rcfg.Nev!=0){	
			DrawHisto(*Histo, N_CH);	
			PrevDrawTime = get_time();
		}	
		
		if (Rcfg.fPrint)
			printf(" ---------------------------------------- \n");			
		
		
		gSystem->ProcessEvents(); 
		
		
		
    } // end of readout loop		
				
	//return ret;
}

CAEN_DGTZ_ErrorCode DataAcquisition(int N_CH, Histograms_t *Histo){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
			
	while(1) {
		gSystem->ProcessEvents(); 
		if (Rcfg.loop == 1){
			ret = CAEN_DGTZ_SWStartAcquisition(handle);
			
			if (ret) {
				printf("ERR_START_ACQUISITION %i \n", ret);
				//new TGMsgBox(gClient->GetRoot(), fMain, "Error", CName, kMBIconStop, kMBOk);
				ret = QuitMain(handle, buffer, (void**)&Events, Waveforms);
			}
			ReadoutLoop(handle, N_CH, Histo);
		}
		
		if (Rcfg.loop == 0){
			ret = CAEN_DGTZ_SWStopAcquisition(handle);
			printf("Acquisition stopped \n");
			Rcfg.loop = -1;
		}
	}
	return ret;
}
