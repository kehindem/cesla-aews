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

#include <BLE_CCS.h>
#include <ccs/CS.h>
#include "BDK.h"

#include <stdarg.h>

#include "BLE_PeripheralServer.h"
#include "aes.h"


#define AES_SALT  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x56, \
                    0xf7, 0xfc, 0x9c, 0x1b, 0x38, 0x44, 0xe2, 0x8b }

#define AES_KEY   { 0xa1, 0x70, 0xb5, 0x05, 0xad, 0xd7, 0x0b, 0xc5, \
                    0x83, 0x03, 0xfa, 0x05, 0xb7, 0xa0, 0x8c, 0xce }

#define AES_DATA_LENGTH 16

uint32_t CS_GetHandleIndex(uint8_t provider_id);

static void CS_PlatformReadHandler(struct BLE_CCS_RxIndData *ind)
{
    uint8_t request_arr[CCS_CHARACTERISTIC_VALUE_LENGTH + 1];

    memcpy(request_arr, ind->data, ind->data_len);
    request_arr[ind->data_len] = 0;

    CS_ProcessRequest((const struct CS_Request_Struct *) request_arr);
}

int CS_PlatformInit(void)
{
    /* Encrypt MAC ID */
    uint8_t aes_input[AES_DATA_LENGTH] = AES_SALT;
    uint8_t aes_output[AES_DATA_LENGTH];
    const unsigned char aes_key[AES_DATA_LENGTH] = AES_KEY;
    mbedtls_aes_context ctx;

    Device_Param_Read(PARAM_ID_PUBLIC_BLE_ADDRESS, aes_input);
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, aes_key, 128);
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, aes_input, aes_output);
    mbedtls_aes_free(&ctx);
    BDK_BLE_SetManufSpecificData(aes_output, AES_DATA_LENGTH);

    /* INitialize CCS Service Profile and assign our request handler. */
	BLE_CCS_Initialize(&CS_PlatformReadHandler);

	return CS_OK;
}

int CS_PlatformWriteString(const char* tx_data, int tx_data_len, uint8_t provider_id)
{
    if (BLE_CCS_Notify((unsigned char*)tx_data, tx_data_len, \
    		CS_GetHandleIndex(provider_id)) == 0)
    {
        return CS_OK;
    }
    else
    {
        return CS_ERROR;
    }
}

int CS_PlatformWriteBytes(const uint16_t* tx_data, int tx_data_len, uint8_t provider_id)
{
    if (BLE_CCS_Notify((unsigned char *)tx_data, tx_data_len, \
    		CS_GetHandleIndex(provider_id)) == 0)
    {
        return CS_OK;
    }
    else
    {
        return CS_ERROR;
    }
}

uint32_t CS_GetHandleIndex(uint8_t provider_id)
{
	switch(provider_id)
	{
		case DMIC_AUDIO:
			return CCS_IDX_DMIC_VALUE_VAL;
		case LEFT_CHNL_AUDIO:
			return CCS_IDX_LCA_VALUE_VAL;
		case RIGHT_CHNL_AUDIO:
			return CCS_IDX_RCA_VALUE_VAL;
		case LEFT_CHNL_FEATURES:
			return CCS_IDX_LCF_VALUE_VAL;
		case RIGHT_CHNL_FEATURES:
			return CCS_IDX_RCF_VALUE_VAL;
		default:
			// System (i.e. error messages)
			return CCS_IDX_SCP_VALUE_VAL;
	}
}

uint32_t CS_PlatformTime()
{
	return HAL_Time();
}

void CS_PlatformLogPrintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	va_end(args);
}

void CS_PlatformLogVprintf(const char* fmt, va_list args)
{


}

void CS_PlatformLogLock(void)
{

}

void CS_PlatformLogUnlock(void)
{

}
