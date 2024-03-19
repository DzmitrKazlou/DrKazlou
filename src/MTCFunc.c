#include <stdio.h>
#include <stdlib.h>
#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
#include "MTCconfig.h"

//#include "TROOT.h"
//#include "TApplication.h"
//#include "TCanvas.h"

#define CAEN_USE_DIGITIZERS
//#define IGNORE_DPP_DEPRECATED


//static CAEN_DGTZ_IRQMode_t INTERRUPT_MODE = CAEN_DGTZ_IRQ_MODE_ROAK;

/*
static long get_time()
{
    long time_ms;
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;

    return time_ms;
}
*/

/*
static double linear_interp(double x0, double y0, double x1, double y1, double x) {
    if (x1 - x0 == 0) {
        fprintf(stderr, "Cannot interpolate values with same x.\n");
        return HUGE_VAL;
    }
    else {
        const double m = (y1 - y0) / (x1 - x0);
        const double q = y1 - m * x1;
        return m * x + q;
    }
}
*/



CAEN_DGTZ_ErrorCode  SwitchOffLogic(int32_t handle, int N_CH) {
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
			printf("log_address[%i][%i] = 0x%04X  log_val[%i][%i] = 0x%04X\n", i, j, log_address[i][j], i, j, reg_val[i][j]);
			
			ret = CAEN_DGTZ_ReadRegister(handle, log_address[i][j], &reg_data);
			printf(" Previously in  0x%08X: %08X \n", log_address[i][j], reg_data);
			ret = CAEN_DGTZ_WriteRegister(handle, log_address[i][j], reg_val[i][j]);	 
			ret = CAEN_DGTZ_ReadRegister(handle, log_address[i][j], &reg_data);
			printf(" In  0x%04X: %08X \n", log_address[i][j], reg_data);
			
		}
	 
    //ret = CAEN_DGTZ_ReadRegister(handle, address, &d32);
    
    //ret = CAEN_DGTZ_WriteRegister(handle, address, d32);
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





CAEN_DGTZ_ErrorCode ProgramDigitizer(int handle, DigitizerConfig_t Dcfg, CAEN_DGTZ_DPP_PSD_Params_t DPPParams) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
       /* Reset the digitizer */
    ret = CAEN_DGTZ_Reset(handle);

    if (ret) {
        printf("ERROR: can't reset the digitizer.\n");
        return ret;
    }
	
	
  /* Set the DPP acquisition mode
    This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly        Only charge (DPP-PSD/DPP-CI v2) is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime    Both charge and time are returned
    CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned */
    ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle, Dcfg.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
    // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
    ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);

    // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
    ret = CAEN_DGTZ_SetIOLevel(handle, Dcfg.IOlev);

    /* Set the digitizer's behaviour when an external trigger arrives:

    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger

    see CAENDigitizer user manual, chapter "Trigger configuration" for details */
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    // Set the enabled channels
    ret = CAEN_DGTZ_SetChannelEnableMask(handle, Dcfg.ChannelMask);

    // Set how many events to accumulate in the board memory before being available for readout
    ret = CAEN_DGTZ_SetDPPEventAggregation(handle, Dcfg.EventAggr, 0);
    
    /* Set the mode used to syncronize the acquisition between different boards.
    In this example the sync is disabled */
    ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
    
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



void calibrate(int handle,  CAEN_DGTZ_BoardInfo_t BoardInfo) {
    printf("\n");
    if (BoardSupportsCalibration(BoardInfo)) {
        //if (WDrun->AcqRun == 0) {
            int32_t ret = CAEN_DGTZ_Calibrate(handle);
            if (ret == CAEN_DGTZ_Success) {
                printf("ADC Calibration check: the board is calibrated.\n");
            }
            else {
                printf("ADC Calibration failed. CAENDigitizer ERR %d\n", ret);
            }
            printf("\n");
        //}
        //else {
        //   printf("Can't run ADC calibration while acquisition is running.\n");
        //}
    }
    else {
        printf("ADC Calibration not needed for this board family.\n");
    }
}

CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	    ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
		ret = CAEN_DGTZ_FreeDPPEvents(handle, Events);
		ret = CAEN_DGTZ_FreeDPPWaveforms(handle, Waveforms);
				
		exit(0);
		return ret;
}
