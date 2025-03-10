#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
//#include "MTCconfig.h"

#define MAX_CH 16 
#define b_width 2 //2 ns bin width for 500MS/s V1730S

typedef struct
{
    //CAEN_DGTZ_ConnectionType LinkType;
	uint32_t PID;
	int Nch;
    //uint32_t VMEBaseAddress;
	uint32_t RecordLength[MAX_CH];
	uint32_t ChannelMask;
	uint32_t PreTrigger[MAX_CH];
	int  DCOffset[MAX_CH]; //uint32_t
    int EventAggr;
    CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_CH];
    CAEN_DGTZ_DPP_AcqMode_t AcqMode;
    CAEN_DGTZ_IOLevel_t IOlev;
	
	//part from CAEN_DGTZ_DPP_PSD_Params_t to not use two structures
	
	int thr[MAX_CH];
	int nsbl[MAX_CH];
	int tvaw[MAX_CH];
	int pgate[MAX_CH];
	int sgate[MAX_CH];
	int lgate[MAX_CH];
	int selft[MAX_CH];
	CAEN_DGTZ_DPP_TriggerConfig_t trgc[MAX_CH];
	int discr[MAX_CH];
	int cfdd[MAX_CH];
	int cfdf[MAX_CH];
	int csens[MAX_CH];
	CAEN_DGTZ_DPP_PUR_t purh; 
	int purgap;
	int blthr;
	int  bltmo; //not used in example but exist in structure of CAEN_DGTZ_DPP_PSD_Params_t
	int trgho;
	
	
} DigitizerConfig_t;


long get_time();

CAEN_DGTZ_ErrorCode SwitchOffLogic(int handle, int N_CH);

 CAEN_DGTZ_ErrorCode SetLogic(int handle, uint32_t reg_val[2][8], int N_CH);
 
 CAEN_DGTZ_ErrorCode SetDPPParameters(int handle, DigitizerConfig_t Dcfg);
 
 CAEN_DGTZ_ErrorCode ProgramDigitizer(int handle, DigitizerConfig_t Dcfg); 
 
 CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer, void **Events, CAEN_DGTZ_DPP_PSD_Waveforms_t *Waveforms);
 
 void SetDefaultConfiguration(DigitizerConfig_t *Dcfg); //, CAEN_DGTZ_DPP_PSD_Params_t *DPPParams); 

int ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg); // CAEN_DGTZ_DPP_PSD_Params_t *DPPParams);

 
 
 