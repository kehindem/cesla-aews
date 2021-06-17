// ----------------------------------------------------------------------------
// Copyright (c) 2018 Semiconductor Components Industries LLC
// (d/b/a "ON Semiconductor").  All rights reserved.
// This software and/or documentation is licensed by ON Semiconductor under
// limited terms and conditions.  The terms and conditions pertaining to the
// software and/or documentation are available at
// http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf ("ON Semiconductor Standard
// Terms and Conditions of Sale, Section 8 Software") and if applicable the
// software license agreement.  Do not use this software and/or documentation
// unless you have carefully read and you agree to the limited terms and
// conditions.  By using this software and/or documentation, you agree to the
// limited terms and conditions.
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <ccs/providers/CSP_LP_DMIC.h>
#include <BLE_CCS.h>
#include <ccs/CS_Peripherals_Init.h>
#include <HAL.h>

//-----------------------------------------------------------------------------
// EXTERNAL / FORWARD DECLARATIONS
//-----------------------------------------------------------------------------

/** \brief Handler for CS requests provided in provider structure. */
static int CSP_DMIC_RequestHandler(const struct CS_Request_Struct* request);

static int CSP_DMIC_PowerModeHandler(enum CS_PowerMode mode);

static void CSP_DMIC_PollHandler(void);

static int16_t csp_dmic_tx[MAX_DATA_LEN_HW];
//static uint16_t packet_cnt = 0;

//-----------------------------------------------------------------------------
// INTERNAL VARIABLES
//-----------------------------------------------------------------------------

/** \brief CS provider structure passed to CS. */
static struct CS_Provider_Struct dmic_provider = {
        CSP_DMIC_ID,
		CSP_DMIC_AVAIL_BIT,
		&CSP_DMIC_RequestHandler,
		&CSP_DMIC_PowerModeHandler,
		&CSP_DMIC_PollHandler
};

//-----------------------------------------------------------------------------
// FUNCTION DEFINITIONS
//-----------------------------------------------------------------------------
struct CS_Provider_Struct* CSP_LP_DMIC_Create(void)
{
	struct CS_Provider_Struct *retval_prov = &dmic_provider;

	/* Initialize and power down the DMIC.*/
	DMIC_Initialize();

    return retval_prov;
}

static int CSP_DMIC_RequestHandler(const struct CS_Request_Struct* request)
{

    if (request->op_code & START)
    {
    	if(request->op_code & DEBUG) Configure_DMIC_Debug();
    	else  Configure_DMIC_Release();

    	/* Indication of request acknowledgment */
    	DIO->CFG[1] =  DIO->CFG[1] | 0x1;
    	HAL_Delay(250);
    	DIO->CFG[1] =  DIO->CFG[1] & ~0x1;

        /* Enable the DMIC */
        CSP_DMIC_PowerModeHandler(CS_POWER_MODE_NORMAL);

        // Save request token for use with poll handler
        dmic_provider.req_token = (uint32_t) request->op_code;

        // Tell CCS that it should not send any response to peer device.
        return CS_NO_RESPONSE;
    }

    // Stop streaming was requested
	CSP_DMIC_PowerModeHandler(CS_POWER_MODE_SLEEP);

    return CS_OK;
}

static int CSP_DMIC_PowerModeHandler(enum CS_PowerMode mode)
{
    switch (mode)
    {
		case CS_POWER_MODE_NORMAL:
			// Activate the DMIC
			ENABLE_DMIC();
			Sys_DMA_ChannelEnable(DMIC_DMA_CH);
			//CSP_DMIC_Verbose("DMIC powered on.");
			break;

		case CS_POWER_MODE_SLEEP:
			 /* Put the DMIC to sleep */
			DISABLE_DMIC();
			Sys_DMA_ChannelDisable(DMIC_DMA_CH);
			//CSP_DMIC_Verbose("DMIC powered off.");
			break;
    }

    return CS_OK;
}

static inline void buffer_cast_uint32_to_int16(int16_t dest_data[], uint32_t src_data[], uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		dest_data[i] = (int16_t) src_data[i];
	}
}

/* Packs elements from DMA output buffer into transmit packet */
static inline void Pack_Audio_Packet(uint32_t src_buffer[])
{
	if(dmic_provider.req_token & DEBUG)
	{
		// Insert timestamp packet header and add samples to transmit buffer
		csp_dmic_tx[0] = HAL_Time() % UINT16_MAX;
		buffer_cast_uint32_to_int16(&csp_dmic_tx[1], src_buffer, dmic_buffer_len);

	} else buffer_cast_uint32_to_int16(csp_dmic_tx, src_buffer, dmic_buffer_len);
}

static void CSP_DMIC_PollHandler(void)
{
	if(dmic_buffer_1_full)
	{
		Pack_Audio_Packet(dmic_buffer_1);
		dmic_buffer_1_full = false;
		BLE_CCS_Notify((unsigned char *)csp_dmic_tx, CS_MAX_RESPONSE_LENGTH,
				CCS_IDX_DMIC_VALUE_VAL);

	}

	if(dmic_buffer_2_full)
	{
		Pack_Audio_Packet(dmic_buffer_2);
		dmic_buffer_2_full = false;
		BLE_CCS_Notify((unsigned char *)csp_dmic_tx, CS_MAX_RESPONSE_LENGTH,
				CCS_IDX_DMIC_VALUE_VAL);
	}
}

