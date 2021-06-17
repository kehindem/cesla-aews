// ----------------------------------------------------------------------------
// Copyright (c) 2021 McMaster University
// CS_Peripherals.h
// Author: 		CESLA
// ----------------------------------------------------------------------------

#ifndef _CS_PERIPHERALS_H_
#define _CS_PERIPHERALS_H_

#include <ccs/CS.h>
//-----------------------------------------------------------------------------
// APPLICATION LAYER DEFINES
//-----------------------------------------------------------------------------
// #include <RTE_app_config.h>

/* Sampling rate setting to be used by the DMIC.
 *
 * Higher integration times will increase measurement precision
 * environments but will use more power.
 */

//#define CSN_LP_ALS_INTEG_TIME_SETTING  RTE_APP_ICS_AL_INTEG_TIME
//#define CLK_SELECTION RTE_APP_CCS_DMIC_CLK_SEL


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// PERIPHERALS - DMIC CONFIGURATIONS
// Note: initially disabled until required by application
//-----------------------------------------------------------------------------
#define AUDIO_DMIC0_GAIN                0x800
#define AUDIO_CONFIG		             OD_AUDIOSLOWCLK            | \
                                         DMIC_AUDIOCLK              | \
                                         DECIMATE_BY_64             | \
                                         OD_UNDERRUN_PROTECT_ENABLE | \
                                         OD_DATA_MSB_ALIGNED        | \
                                         DMIC0_DATA_LSB_ALIGNED     | \
                                         DMIC1_DATA_LSB_ALIGNED     | \
                                         OD_DMA_REQ_DISABLE         | \
                                         DMIC0_DMA_REQ_DISABLE       | \
                                         DMIC1_DMA_REQ_DISABLE      | \
                                         OD_INT_GEN_DISABLE         | \
										 DMIC0_INT_GEN_DISABLE		| \
                                         DMIC1_INT_GEN_DISABLE      | \
                                         OD_DISABLE                 | \
                                         DMIC0_DISABLE              | \
                                         DMIC1_DISABLE

//-----------------------------------------------------------------------------
// PERIPHERALS - ADC EXTERNAL INTERFACE CONFIGURATIONS
//-----------------------------------------------------------------------------
#if RTE_APP_ADC_SAMPLING_RATE==0 										/* 1250Hz */
	#define ADC_PRESCALE_VALUE          	ADC_PRESCALE_200
#elif RTE_APP_ADC_SAMPLING_RATE == 1									/* 12.5kHz */
	#define ADC_PRESCALE_VALUE          	ADC_PRESCALE_20H
#elif RTE_APP_ADC_SAMPLING_RATE == 2									/* 25kHz */
	#define ADC_PRESCALE_VALUE          	ADC_PRESCALE_20H
#elif RTE_APP_ADC_SAMPLING_RATE == 3									/* 50kHz */
	#define ADC_PRESCALE_VALUE          	ADC_PRESCALE_20H
#endif


//-----------------------------------------------------------------------------
// PERIPHERAL LOW POWER STATUS
//-----------------------------------------------------------------------------
/* Stores current power state of the peripherals. */
#define DMIC_IS_AWAKE()					AUDIO->CFG & DMIC0_ENABLE

//-----------------------------------------------------------------------------
// PERIPHERALS - DMA CONFIGURATION
//-----------------------------------------------------------------------------
#define 	DMA_CONFIG					(DMA_ENABLE                 | \
										 DMA_ADDR_CIRC				| \
										 DMA_SRC_ADDR_STATIC		| \
										 DMA_DEST_ADDR_INC			| \
										 DMA_TRANSFER_P_TO_M		| \
										 DMA_PRIORITY_0				| \
										 DMA_SRC_DMIC				| \
										 DMA_DEST_I2C				| \
										 DMA_SRC_WORD_SIZE_16		| \
										 DMA_DEST_WORD_SIZE_16		| \
										 DMA_START_INT_DISABLE		| \
										 DMA_COUNTER_INT_DISABLE	| \
										 DMA_COMPLETE_INT_ENABLE	| \
										 DMA_ERROR_INT_ENABLE		| \
										 DMA_DISABLE_INT_DISABLE	| \
										 DMA_LITTLE_ENDIAN			| \
										 DMA_SRC_ADDR_POS			| \
										 DMA_DEST_ADDR_POS			| \
										 DMA_SRC_ADDR_STEP_SIZE_1	| \
										 DMA_DEST_ADDR_STEP_SIZE_1)

//-----------------------------------------------------------------------------
// ASSOCIATED DMA CHANNELS
//-----------------------------------------------------------------------------
#define DMIC_DMA_CH						6
#define LCA_DMA_CH						1
#define RCA_DMA_CH						2
#define LCF_DMA_CH						3
#define RCF_DMA_CH						7 //TODO: double check DMA CH7 availability

#define TOKEN(left, channel, right) left ## channel ## right 		// layer of indirection
#define DMA_IRQHandler(channel) TOKEN(DMA, channel, _IRQHandler) 	// DMAx_IRQHandler
#define DMA_IRQn(channel) TOKEN(DMA, channel, _IRQn) 				// DMAx_IRQn

//-----------------------------------------------------------------------------
// ASSOCIATED ADC CHANNELS
//-----------------------------------------------------------------------------
#define VBATT_ADC_CH					0
#define LCA_ADC_CH						1
#define RCA_ADC_CH						2

//-----------------------------------------------------------------------------
// ASSOCIATED DIO Pads
//-----------------------------------------------------------------------------
#define DMIC_CLK_OUT_PAD				10
#define DMIC_INPUT_PAD					6
#define LCA_DIO_PAD						3
#define RCA_DIO_PAD						0

// Allocated DSP pins
// DIO7, DIO8, DIO11 & DIO14

// Indicators
#define GREEN_LED						LED_GREEN 	//1
#define RED_LED							LED_RED 	//2

// Control Switches
#define RECOVERY						PIN_RECOVERY //13
#define WAKE_BUTTON						15

#define TOKEN2(left, pad) left ## pad  							// layer of indirection
#define ADC_POS_INPUT_DIO(pad) TOKEN2(ADC_POS_INPUT_DIO, pad) 	// ADC_POS_INPUT_DIOx
#define ADC_INT_CH(pad) TOKEN2(ADC_INT_CH, pad)					//ADC_INT_CHx


//-----------------------------------------------------------------------------
// EXPORTED INITIALIZATION FUNCTIONS
//-----------------------------------------------------------------------------
extern void DMIC_Initialize(void); // Initialize and power down DMIC
extern void LCA_Initialize(void);  // Initialize DIO3, ADC1
extern void RCA_Initialize(void);  // Initialize DIO0, ADC2

extern void Configure_DMIC_Debug(void);
extern void Configure_DMIC_Release(void);

extern void Configure_LCA_Debug(void);
extern void Configure_LCA_Release(void);

extern void Configure_RCA_Debug(void);
extern void Configure_RCA_Release(void);

//-----------------------------------------------------------------------------
// STEREO CONFIGURATION INTERNAL VARIABLES
//-----------------------------------------------------------------------------
bool lca_enabled;
bool rca_enabled;
//-----------------------------------------------------------------------------
// EXPORTED PERIPHERAL ENABLE/DISABLE FUNCTIONS
//-----------------------------------------------------------------------------
#define ENABLE_DMIC()					AUDIO->CFG |= DMIC0_ENABLE
#define DISABLE_DMIC()					AUDIO->CFG &= ~DMIC0_ENABLE
#define ENABLE_LCA()					{Sys_ADC_Set_BATMONIntConfig(INT_EBL_ADC | ADC_INT_CH(LCA_ADC_CH));\
										NVIC_EnableIRQ(ADC_BATMON_IRQn);\
										lca_enabled = true;}
#define DISABLE_LCA()					{Sys_ADC_Set_BATMONIntConfig(INT_DIS_ADC | ADC_INT_CH(LCA_ADC_CH));\
										lca_enabled = false;}
#define ENABLE_RCA()					{Sys_ADC_Set_BATMONIntConfig(INT_EBL_ADC | ADC_INT_CH(RCA_ADC_CH));\
										NVIC_EnableIRQ(ADC_BATMON_IRQn);\
										rca_enabled = true;}
#define DISABLE_RCA()					{Sys_ADC_Set_BATMONIntConfig(INT_DIS_ADC | ADC_INT_CH(RCA_ADC_CH));\
										rca_enabled = false;}

//-----------------------------------------------------------------------------
// BUFFERED DMIC DATA
//-----------------------------------------------------------------------------
// Status Variables
bool dmic_buffer_1_full;
bool dmic_buffer_2_full;
// Data
uint32_t* dmic_buffer_1;
uint32_t* dmic_buffer_2;
uint8_t dmic_buffer_len;

//-----------------------------------------------------------------------------
// BUFFERED LCA DATA
//-----------------------------------------------------------------------------
// Status Variables
bool lca_buffer_1_full;
bool lca_buffer_2_full;
// Data
int16_t* lca_buffer_1;
int16_t* lca_buffer_2;
uint8_t lca_buffer_len;

//-----------------------------------------------------------------------------
// BUFFERED RCA DATA
//-----------------------------------------------------------------------------
// Status Variables
bool rca_buffer_1_full;
bool rca_buffer_2_full;
// Data
int16_t* rca_buffer_1;
int16_t* rca_buffer_2;
uint8_t rca_buffer_len;

#endif /* CS_PERIPHERALS_H_ */
