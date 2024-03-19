#include <stdio.h>
#include <stdlib.h>
#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"





//CAEN_DGTZ_ErrorCode Set_calibrated_DCO(int handle, int ch, WaveDumpConfig_t *WDcfg, CAEN_DGTZ_BoardInfo_t BoardInfo);


CAEN_DGTZ_ErrorCode SwitchOffLogic(int handle, int N_CH);

 CAEN_DGTZ_ErrorCode SetLogic(int handle, uint32_t reg_val[2][8], int N_CH);
 
 CAEN_DGTZ_ErrorCode ProgramDigitizer(int handle, DigitizerConfig_t Dcfg, CAEN_DGTZ_DPP_PSD_Params_t DPPParams); 
 
 CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms);
 
 void calibrate(int handle,  CAEN_DGTZ_BoardInfo_t BoardInfo);
 
 