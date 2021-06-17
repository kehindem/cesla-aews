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

#include <ccs/providers/CSP_LP_LCA.h>
#include <BLE_CCS.h>
#include <ccs/CS_Peripherals_Init.h>
#include <HAL.h>

//-----------------------------------------------------------------------------
// EXTERNAL / FORWARD DECLARATIONS
//-----------------------------------------------------------------------------

/** \brief Handler for CS requests provided in provider structure. */
static int CSP_LCA_RequestHandler(const struct CS_Request_Struct* request);

static int CSP_LCA_PowerModeHandler(enum CS_PowerMode mode);

static void CSP_LCA_PollHandler(void);

static int16_t csp_lca_tx[MAX_DATA_LEN_HW];

//-----------------------------------------------------------------------------
// INTERNAL VARIABLES
//-----------------------------------------------------------------------------

/** \brief CS provider structure passed to CS. */
static struct CS_Provider_Struct lca_provider = {
        CSP_LCA_ID,
		CSP_LCA_AVAIL_BIT,
		&CSP_LCA_RequestHandler,
		&CSP_LCA_PowerModeHandler,
		&CSP_LCA_PollHandler
};

//-----------------------------------------------------------------------------
// FUNCTION DEFINITIONS
//-----------------------------------------------------------------------------
struct CS_Provider_Struct* CSP_LP_LCA_Create(void)
{
	struct CS_Provider_Struct *retval_prov = &lca_provider;

	/* Initialize and power down the LCA.*/
	LCA_Initialize();

    return retval_prov;
}

static int CSP_LCA_RequestHandler(const struct CS_Request_Struct* request)
{

    if (request->op_code & START)
    {
    	if(request->op_code & DEBUG) Configure_LCA_Debug();
    	else  Configure_LCA_Release();

    	/* Indication of request acknowledgment */
    	DIO->CFG[2] =  DIO->CFG[2] | 0x1;
    	HAL_Delay(250);
    	DIO->CFG[2] =  DIO->CFG[2] & ~0x1;

        /* Enable the LCA */
        CSP_LCA_PowerModeHandler(CS_POWER_MODE_NORMAL);

        // Save request token for use with poll handler
        lca_provider.req_token = (uint32_t) request->op_code;

        // Tell CCS that it should not send any response to peer device.
        return CS_NO_RESPONSE;
    }

    // Stop streaming was requested
	CSP_LCA_PowerModeHandler(CS_POWER_MODE_SLEEP);

    return CS_OK;
}

static int CSP_LCA_PowerModeHandler(enum CS_PowerMode mode)
{
    switch (mode)
    {
		case CS_POWER_MODE_NORMAL:
			// Activate the LCA
			ENABLE_LCA();
			//CSP_LCA_Verbose("LCA powered on.");
			break;

		case CS_POWER_MODE_SLEEP:
			 /* Put the LCA to sleep */
			DISABLE_LCA();
			//CSP_LCA_Verbose("LCA powered off.");
			break;
    }

    return CS_OK;
}

/* Packs elements from DMA output buffer into transmit packet */
static inline void Pack_Audio_Packet(int16_t src_buffer[])
{
	if(lca_provider.req_token & DEBUG)
	{
		// Insert timestamp packet header and add samples to transmit buffer
		csp_lca_tx[0] = HAL_Time() % UINT16_MAX;
		memcpy(&csp_lca_tx[1], src_buffer, lca_buffer_len*2);

	} else memcpy(csp_lca_tx, src_buffer, lca_buffer_len*2);
}

static void CSP_LCA_PollHandler(void)
{
	if(lca_buffer_1_full)
	{
		Pack_Audio_Packet(lca_buffer_1);
		lca_buffer_1_full = false;
		BLE_CCS_Notify((unsigned char *)csp_lca_tx, CS_MAX_RESPONSE_LENGTH,
				CCS_IDX_LCA_VALUE_VAL);
	}

	if(lca_buffer_2_full)
	{
		Pack_Audio_Packet(lca_buffer_2);
		lca_buffer_2_full = false;
		BLE_CCS_Notify((unsigned char *)csp_lca_tx, CS_MAX_RESPONSE_LENGTH,
				CCS_IDX_LCA_VALUE_VAL);

	}
}

