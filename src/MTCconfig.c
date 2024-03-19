/******************************************************************************
* 
* CAEN SpA - Front End Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* \note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
* -----------------------------------------------------------------------------
* Dconfig contains the functions for reading the configuration file and 
* setting the parameters in the Dcfg structure
******************************************************************************/


#include <../include/CAENDigitizerType.h>
#include "CAENDigitizer.h"
#include "MTCconfig.h"
#include <stdlib.h>


static void SetDefaultConfiguration(DigitizerConfig_t *Dcfg, CAEN_DGTZ_DPP_PSD_Params_t *DPPParams) {
    
    
	Dcfg->EventAggr = 1; // 1023
	Dcfg->ChannelMask = 0x3FF;
	
	
	Dcfg->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
	Dcfg->IOlev = CAEN_DGTZ_IOLevel_NIM;
	
	Dcfg->Nch = 16; // v1730
	
	for(int ch=0; ch<Dcfg->Nch; ch++) {
		Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityPositive; //Negative //Positive
		Dcfg->RecordLength[ch] = 500;
		Dcfg->PreTrigger[ch] = 50;
		Dcfg->DCOffset[ch] = 0x199A; //10%
			
            DPPParams->thr[ch] = 100;        // Trigger Threshold
            /* The following parameter is used to specifiy the number of samples for the baseline averaging:
            0 -> absolute Bl
            1 -> 16samp
            2 -> 64samp
            3 -> 256samp
            4 -> 1024samp */
            DPPParams->nsbl[ch] = 1;
            DPPParams->lgate[ch] = 500;    // Long Gate Width (N*2ns for x730  and N*4ns for x725)
            DPPParams->sgate[ch] = 124;    // Short Gate Width (N*2ns for x730  and N*4ns for x725)
            DPPParams->pgate[ch] = 8;     // Pre Gate Width (N*2ns for x730  and N*4ns for x725) 
            /* Self Trigger Mode:
            0 -> Disabled
            1 -> Enabled */
            DPPParams->selft[ch] = 1;
            // Trigger configuration:
            //    CAEN_DGTZ_DPP_TriggerMode_Normal ->  Each channel can self-trigger independently from the other channels
			//    CAEN_DGTZ_DPP_TriggerMode_Coincidence -> A validation signal must occur inside the shaped trigger coincidence window
            //DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerMode_Normal;
			
			//DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerConfig_Peak;
			DPPParams->trgc[ch] = CAEN_DGTZ_DPP_TriggerConfig_Threshold;

			/*Discrimination mode for the event selection 
			CAEN_DGTZ_DPP_DISCR_MODE_LED -> Leading Edge Distrimination
			CAEN_DGTZ_DPP_DISCR_MODE_CFD -> Constant Fraction Distrimination*/
			DPPParams->discr[ch] = CAEN_DGTZ_DPP_DISCR_MODE_LED;

			/*CFD delay (N*2ns for x730  and N*4ns for x725)  */
			DPPParams->cfdd[ch] = 4;  

			/*CFD fraction: 0->25%; 1->50%; 2->75%; 3->100% */
			DPPParams->cfdf[ch] = 0;

            /* Trigger Validation Acquisition Window */
            DPPParams->tvaw[ch] = 250;

            /* Charge sensibility: 
			Options for Input Range 2Vpp: 0->5fC/LSB; 1->20fC/LSB; 2->80fC/LSB; 3->320fC/LSB; 4->1.28pC/LSB; 5->5.12pC/LSB 
			Options for Input Range 0.5Vpp: 0->1.25fC/LSB; 1->5fC/LSB; 2->20fC/LSB; 3->80fC/LSB; 4->320fC/LSB; 5->1.28pC/LSB */
            DPPParams->csens[ch] = 0;
        }
        /* Pile-Up rejection Mode
        CAEN_DGTZ_DPP_PSD_PUR_DetectOnly -> Only Detect Pile-Up
        CAEN_DGTZ_DPP_PSD_PUR_Enabled -> Reject Pile-Up */
        DPPParams->purh = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly;
        DPPParams->purgap = 100;  // Purity Gap in LSB units (1LSB = 0.12 mV for 2Vpp Input Range, 1LSB = 0.03 mV for 0.5 Vpp Input Range )
        DPPParams->blthr = 3;     // Baseline Threshold
        DPPParams->trgho = 8;     //8   // Trigger HoldOff
    
	
}


int ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg, CAEN_DGTZ_DPP_PSD_Params_t *DPPParams) 
{
	char str[1000], str1[1000], *pread = NULL;
	int i, ch=-1, val, Off=0, tr = -1;
    int ret = 0;
	int RL_val, thr_val;
     

	/* Default settings */
	SetDefaultConfiguration(Dcfg, DPPParams);


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


		// Post Trigger (percent of the acquisition window)
		//if (strstr(str, "POST_TRIGGER")!=NULL) {
		//	read = fscanf(f_ini, "%d", &Dcfg->PostTrigger);
		//	continue;
		//}


	
		
	 ///Input polarity	
	 
		if (strstr(str, "PULSE_POLARITY")!=NULL) {
			//CAEN_DGTZ_PulsePolarity_t pp= CAEN_DGTZ_PulsePolarityPositive;
			//CAEN_DGTZ_TriggerPolarity_t pp  = CAEN_DGTZ_TriggerOnRisingEdge;
			read = fscanf(f_ini, "%s", str1);
			if (strcmp(str1, "POSITIVE") == 0)
				for (int ch=0; ch<Dcfg->Nch; ch++)
					Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityPositive;
				//pp = CAEN_DGTZ_TriggerOnRisingEdge;
			else if (strcmp(str1, "NEGATIVE") == 0)
				for (int ch=0; ch<Dcfg->Nch; ch++)		
					Dcfg->PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityNegative;
				//pp = CAEN_DGTZ_TriggerOnFallingEdge;
			else
				printf("%s: Invalid Parameter\n", str);

						
			continue;
		}
	
		//DC offset (percent of the dynamic range, -50 to 50)
		//if (!strcmp(str, "DC_OFFSET")) 
		

		// Baseline cut
		//if (!strcmp(str, "BASELINE_LEVEL")) 
		
		
		// Threshold
		if (strstr(str, "TRIGGER_THRESHOLD")!=NULL) {
			read = fscanf(f_ini, "%d", &thr_val);
			for (int ch=0; ch<Dcfg->Nch; ch++)	
				DPPParams->thr[ch] = thr_val;
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
	
	
	return ret;
}
