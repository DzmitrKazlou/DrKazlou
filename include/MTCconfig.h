
/***************************************************************************//**
* \note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
******************************************************************************/

	#include <unistd.h>
    #include <stdint.h>   /* C99 compliant compilers: uint64_t */
    #include <ctype.h>    /* toupper() */
    #include <sys/time.h>
	#include <stdio.h>
#include <string.h>
#include <math.h>
	#define		_PACKED_		__attribute__ ((packed, aligned(1)))
	#define		_INLINE_		__inline__ 

//  #define Sleep(t) usleep((t)*1000);

#define DEFAULT_CONFIG_FILE  "Config.txt"  /* local directory */


#define MAX_CH  16          /* max. number of channels */
#define MAX_SET 16           /* max. number of independent settings */
#define MAX_GROUPS  8          /* max. number of groups */

#define MAX_GW  1000        /* max. number of generic write commads */

#define PLOT_REFRESH_TIME 1000

#define VME_INTERRUPT_LEVEL      1
#define VME_INTERRUPT_STATUS_ID  0xAAAA
#define INTERRUPT_TIMEOUT        200  // ms
        
#define PLOT_WAVEFORMS   0
#define PLOT_FFT         1
#define PLOT_HISTOGRAM   2

#define CFGRELOAD_CORRTABLES_BIT (0)
#define CFGRELOAD_DESMODE_BIT (1)

#define NPOINTS 2
#define NACQS   50


/* ###########################################################################
   Typedefs
   ###########################################################################
*/

typedef enum {
	OFF_BINARY=	0x00000001,			// Bit 0: 1 = BINARY, 0 =ASCII
	OFF_HEADER= 0x00000002,			// Bit 1: 1 = include header, 0 = just samples data
} OUTFILE_FLAGS;


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
} DigitizerConfig_t;


/* ###########################################################################
*  Functions
*  ########################################################################### */


static void SetDefaultConfiguration(DigitizerConfig_t *Dcfg, CAEN_DGTZ_DPP_PSD_Params_t *DPPParams); 
	
/*! \fn      int ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg) 
*   \brief   Read the configuration file and set the DigitizerConfig paremeters
*            
*   \param   f_ini        Pointer to the config file
*   \param   Dcfg:   Pointer to the DigitizerConfig data structure
*   \return  0 = Success; negative numbers are error codes
*/
int ParseConfigFile(FILE *f_ini, DigitizerConfig_t *Dcfg, CAEN_DGTZ_DPP_PSD_Params_t *DPPParams);

/*! \fn      int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask)
*   \brief   writes 'data' on register at 'address' using 'mask' as bitmask
*
*   \param   handle :   Digitizer handle
*   \param   address:   Address of the Register to write
*   \param   data   :   Data to Write on the Register
*   \param   mask   :   Bitmask to use for data masking
*   \return  0 = Success; negative numbers are error codes
*/






