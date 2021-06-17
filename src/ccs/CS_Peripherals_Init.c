// ----------------------------------------------------------------------------
// Copyright (c) 2021 McMaster University
// CESLA Custom Service Data
// CS_Peripherals.c
// Author: 		CESLA 
// ----------------------------------------------------------------------------

#include <ccs/CS_Peripherals_Init.h>
#include <HAL.h>

/* Internal Variables */
static uint8_t lca_buffer_idx;
static uint8_t rca_buffer_idx;
#if STILL_DEBUGGING_THIS
static int16_t lca_sample;
static int16_t rca_sample;
#endif

/* Circular Buffers */
static uint32_t dmic_values[CS_MAX_RESPONSE_LENGTH];
static int16_t lca_values[CS_MAX_RESPONSE_LENGTH];
static int16_t rca_values[CS_MAX_RESPONSE_LENGTH];

#if STILL_DEBUGGING_THIS
static void LCA_DMA_Start_Transfer(void);
#endif

/* Initialize DMIC to 3.125kHz and power down */
void DMIC_Initialize(void)
{
	/** Configure DMIC input to test INMP522 microphone. */

	/* Configure AUDIOCLK to 2 MHz and AUDIOSLOWCLK to 1 MHz. */
	/* System clock is (48 / 6) MHz. */
	CLK->DIV_CFG1 &= ~(AUDIOCLK_PRESCALE_64 | AUDIOSLOWCLK_PRESCALE_4);
	CLK->DIV_CFG1 |= AUDIOCLK_PRESCALE_4 | AUDIOSLOWCLK_PRESCALE_2;

	/* Configure OD, DMIC0 and DMIC1 */
	Sys_Audio_Set_Config(AUDIO_CONFIG);

	Sys_Audio_Set_DMICConfig(DMIC0_DCRM_CUTOFF_20HZ | DMIC1_DCRM_CUTOFF_20HZ |
							 DMIC1_DELAY_DISABLE | DMIC0_FALLING_EDGE |
							 DMIC1_RISING_EDGE, 0);

	Sys_Audio_DMICDIOConfig(DIO_6X_DRIVE | DIO_LPF_DISABLE | DIO_NO_PULL,
							DMIC_CLK_OUT_PAD, DMIC_INPUT_PAD, DIO_MODE_AUDIOCLK);

	/* Configure Gains for DMIC0, DMIC1 and OD */
	AUDIO->DMIC0_GAIN = AUDIO_DMIC0_GAIN;
}

static inline void Configure_DMIC_DMA_Release()
{
	AUDIO->CFG |= DMIC0_DMA_REQ_ENABLE;

	// Clear DMA status register
	Sys_DMA_ClearChannelStatus(DMIC_DMA_CH);

	Sys_DMA_ChannelConfig(DMIC_DMA_CH, DMA_CONFIG, CS_MAX_RESPONSE_LENGTH, \
			CS_MAX_RESPONSE_LENGTH/2, (uint32_t) &(AUDIO_DMIC_DATA->DMIC0_DATA_SHORT), \
			(uint32_t) dmic_values);

	NVIC_EnableIRQ(DMA_IRQn(DMIC_DMA_CH));
	NVIC_SetPriority(DMA_IRQn(DMIC_DMA_CH), 3);
}

static inline void Configure_DMIC_DMA_Debug()
{
	AUDIO->CFG |= DMIC0_DMA_REQ_ENABLE;

	// Clear DMA status register
	Sys_DMA_ClearChannelStatus(DMIC_DMA_CH);


	// Complete transfer after each 18 samples to accommodate packet headers
	Sys_DMA_ChannelConfig(DMIC_DMA_CH, DMA_CONFIG, 2*dmic_buffer_len, \
			dmic_buffer_len, (uint32_t) &(AUDIO_DMIC_DATA->DMIC0_DATA_SHORT),\
			(uint32_t) dmic_values);

	NVIC_EnableIRQ(DMA_IRQn(DMIC_DMA_CH));
	NVIC_SetPriority(DMA_IRQn(DMIC_DMA_CH), 3);
}

void Configure_DMIC_Debug()
{
	dmic_buffer_len = (CS_MAX_RESPONSE_LENGTH-2)/2;
	dmic_buffer_1 = dmic_values;
	dmic_buffer_2 = &dmic_values[dmic_buffer_len];

	Configure_DMIC_DMA_Debug();
}

void Configure_DMIC_Release()
{
	dmic_buffer_len = CS_MAX_RESPONSE_LENGTH/2;
	dmic_buffer_1 = dmic_values;
	dmic_buffer_2 = &dmic_values[dmic_buffer_len];

	Configure_DMIC_DMA_Release();
}

/* ----------------------------------------------------------------------------
 * Function      : void DMA<<dmic_dma_ch>>_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : This function handles interrupts for DMIC DMA channel
 *                 counter interrupt and transfer completion events.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void DMA_IRQHandler(DMIC_DMA_CH)(void)
{
	uint16_t status = Sys_DMA_Get_ChannelStatus(DMIC_DMA_CH);

	if(status & DMA_COUNTER_INT_STATUS) dmic_buffer_1_full = true;
	else if(status & DMA_COMPLETE_INT_STATUS) dmic_buffer_2_full = true;

	Sys_DMA_ClearChannelStatus(DMIC_DMA_CH);
}

void LCA_Initialize(void)
{
	// System clock is 48/6 MHz
	// Configure slow clock to 4MHz
//	CLK->DIV_CFG0 |= SLOWCLK_PRESCALE_2;

	/* Configure DIO3 */
	Sys_DIO_Config(LCA_DIO_PAD, DIO_NO_PULL | DIO_MODE_DISABLE); //DIO_LPF_ENABLE

	/* Calibrate ADC (Normalize 0V ADC Offset) */
	Sys_ADC_InputSelectConfig(LCA_ADC_CH, ADC_NEG_INPUT_GND | ADC_POS_INPUT_GND);

	/* TODO: Configure ADC 25kHz (max for 14-bit resolution with slowclk 4MHz)*/
	/* Configure ADC 1.25kHz (14-bit resolution)*/
	Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL | ADC_PRESCALE_VALUE);
	Sys_ADC_InputSelectConfig(LCA_ADC_CH, ADC_NEG_INPUT_GND | ADC_POS_INPUT_DIO(LCA_DIO_PAD));
}

void RCA_Initialize(void)
{
	// System clock is 48/6 MHz
	// Configure slow clock to 4MHz
//	CLK->DIV_CFG0 |= SLOWCLK_PRESCALE_2;

	/* Configure DIO3 */
	Sys_DIO_Config(RCA_DIO_PAD, DIO_NO_PULL | DIO_MODE_DISABLE); //DIO_LPF_ENABLE

	/* Calibrate ADC (Normalize 0V ADC Offset) */
	Sys_ADC_InputSelectConfig(RCA_ADC_CH, ADC_NEG_INPUT_GND | ADC_POS_INPUT_GND);

	/* TODO: Configure ADC 25kHz (max for 14-bit resolution with slowclk 4MHz)*/
	/* Configure ADC 1.25kHz (14-bit resolution)*/
	Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL | ADC_PRESCALE_VALUE);
	Sys_ADC_InputSelectConfig(RCA_ADC_CH, ADC_NEG_INPUT_GND | ADC_POS_INPUT_DIO(RCA_DIO_PAD));
}


static inline void LCA_ADC_IRQHandler(void)
{
	// Add new sample
	lca_values[lca_buffer_idx] = (int16_t) ADC->DATA_AUDIO_CH[LCA_ADC_CH];
	// Increment circular buffer index
	lca_buffer_idx = (lca_buffer_idx+1) % (2*lca_buffer_len);
	// Update buffer status
	if(lca_buffer_idx == lca_buffer_len) lca_buffer_1_full = true;
	else if(lca_buffer_idx == 0) lca_buffer_2_full = true;
}

static inline void RCA_ADC_IRQHandler(void)
{
	// Add new sample
	rca_values[rca_buffer_idx] = (int16_t) ADC->DATA_AUDIO_CH[RCA_ADC_CH];
	// Increment circular buffer index
	rca_buffer_idx = (rca_buffer_idx+1) % (2*rca_buffer_len);
	// Update buffer status
	if(rca_buffer_idx == rca_buffer_len) rca_buffer_1_full = true;
	else if(rca_buffer_idx == 0) rca_buffer_2_full = true;
}

void ADC_BATMON_IRQHandler(void)
{
	if(Sys_ADC_Get_BATMONStatus() & ADC_READY_TRUE)
	{
	#if STILL_DEBUGGING_THIS
		LCA_DMA_Start_Transfer();
	#else
		if(lca_enabled) LCA_ADC_IRQHandler();
		if(rca_enabled) RCA_ADC_IRQHandler();
	#endif
	}
}

#if STILL_DEBUGGING_THIS
static inline void Configure_LCA_DMA(void)
{
	// Clear DMA status register
	Sys_DMA_ClearChannelStatus(LCA_DMA_CH);

	Sys_DMA_ChannelConfig(LCA_DMA_CH, DMA_CONFIG, 1,0,
			(uint32_t) &(ADC->DATA_TRIM_CH[LCA_ADC_CH]), (uint32_t) &lca_sample);

	NVIC_EnableIRQ(DMA_IRQn(LCA_DMA_CH));
}

static inline void Configure_RCA_DMA(void)
{
	// Clear DMA status register
	Sys_DMA_ClearChannelStatus(LCA_DMA_CH);

	Sys_DMA_ChannelConfig(LCA_DMA_CH, DMA_CONFIG, 1,0,
			(uint32_t) &(ADC->DATA_TRIM_CH[LCA_ADC_CH]), (uint32_t) &lca_sample);

	NVIC_EnableIRQ(DMA_IRQn(LCA_DMA_CH));
}
#endif

void Configure_LCA_Debug()
{
	lca_buffer_len = (CS_MAX_RESPONSE_LENGTH-2)/2;
	lca_buffer_1 = lca_values;
	lca_buffer_2 = &lca_values[lca_buffer_len];

#if STILL_DEBUGGING_THIS
	Configure_LCA_DMA();
#endif
}

void Configure_LCA_Release()
{
	lca_buffer_len = CS_MAX_RESPONSE_LENGTH/2;
	lca_buffer_1 = lca_values;
	lca_buffer_2 = &lca_values[lca_buffer_len];

#if STILL_DEBUGGING_THIS
	Configure_LCA_DMA();
#endif
}

void Configure_RCA_Debug()
{
	rca_buffer_len = (CS_MAX_RESPONSE_LENGTH-2)/2;
	rca_buffer_1 = rca_values;
	rca_buffer_2 = &rca_values[rca_buffer_len];

#if STILL_DEBUGGING_THIS
	Configure_RCA_DMA();
#endif
}

void Configure_RCA_Release()
{
	rca_buffer_len = CS_MAX_RESPONSE_LENGTH/2;
	rca_buffer_1 = rca_values;
	rca_buffer_2 = &rca_values[rca_buffer_len];

#if STILL_DEBUGGING_THIS
	Configure_RCA_DMA();
#endif
}

#if STILL_DEBUGGING_THIS
/* ----------------------------------------------------------------------------
 * Function      : void DMA<<lca_dma_ch>>_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : This function handles interrupts for LCA DMA channel
 *                 transfer completion events.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

void DMA_IRQHandler(LCA_DMA_CH)(void)
{
	uint16_t status = Sys_DMA_Get_ChannelStatus(LCA_DMA_CH);

	if(status & DMA_COMPLETE_INT_STATUS)
	{

	}

	Sys_DMA_ClearChannelStatus(LCA_DMA_CH);
}

/* ----------------------------------------------------------------------------
 * Function      : void DMA<<rca_dma_ch>>_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : This function handles interrupts for RCA DMA channel
 *                 transfer completion events.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */

void DMA_IRQHandler(RCA_DMA_CH)(void)
{
	uint16_t status = Sys_DMA_Get_ChannelStatus(RCA_DMA_CH);

	if(status & DMA_COMPLETE_INT_STATUS)
	{

	}

	Sys_DMA_ClearChannelStatus(RCA_DMA_CH);
}

/* ----------------------------------------------------------------------------
 * static void LCA_DMA_Start (void)
 * ----------------------------------------------------------------------------
 * Description   : Start the LCA ADC DMA channel data transfer
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
static void LCA_DMA_Start_Transfer(void)
{
    /* Clear dma status register */
	Sys_DMA_ClearChannelStatus(LCA_DMA_CH);

    /* Clear pending flag */
    NVIC_ClearPendingIRQ(DMA_IRQn(LCA_DMA_CH));

    /* Enable the interrupt */
    NVIC_EnableIRQ(DMA_IRQn(LCA_DMA_CH));

    /* Start the DMA */
    Sys_DMA_ChannelEnable(LCA_DMA_CH);
}


/* ----------------------------------------------------------------------------
 * static void RCA_DMA_Start (void)
 * ----------------------------------------------------------------------------
 * Description   : Start the RCA ADC DMA channel data transfer
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
static void RCA_DMA_Start_Transfer(void)
{
    /* Clear dma status register */
	Sys_DMA_ClearChannelStatus(RCA_DMA_CH);

    /* Clear pending flag */
    NVIC_ClearPendingIRQ(DMA_IRQn(RCA_DMA_CH));

    /* Enable the interrupt */
    NVIC_EnableIRQ(DMA_IRQn(RCA_DMA_CH));

    /* Start the DMA */
    Sys_DMA_ChannelEnable(RCA_DMA_CH);
}
#endif
