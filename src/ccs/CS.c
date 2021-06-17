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


//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------

#include <ccs/CS.h>
#include <ccs/CS_Platform.h>
#include "RTE_Components.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO: Add LED indication to indicate that a connection has been closed.

#if defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION && CS_LOG_WITH_ANSI_COLORS != 0
#include "ansi_color.h"
#endif

//-----------------------------------------------------------------------------
// DEFINES / CONSTANTS
//-----------------------------------------------------------------------------

#define CSP_SYS_ID 			0b10000000

#define CSP_SYS_AVAIL_BIT 	((uint32_t)0x00000000)


//-----------------------------------------------------------------------------
// FORWARD DECLARATIONS
//-----------------------------------------------------------------------------

static int CSP_SYS_RequestHandler(const struct CS_Request_Struct* request);


//-----------------------------------------------------------------------------
// INTERNAL VARIABLES
//-----------------------------------------------------------------------------

static struct CS_Handle_Struct cs;

static char cs_tx_buffer[CS_MAX_RESPONSE_LENGTH];

static uint16_t cs_prov_response[MAX_DATA_LEN_HW];

static struct CS_Provider_Struct cs_sys_prov = {
		CSP_SYS_ID,
		CSP_SYS_AVAIL_BIT,
		&CSP_SYS_RequestHandler
};

//-----------------------------------------------------------------------------
// FUNCTION DEFINITIONS
//-----------------------------------------------------------------------------

int CS_Init()
{
	int i, errcode;

	// Initialize structure with default values
	cs.provider_cnt = 0;
	for (i = 0; i < CS_MAX_PROVIDER_COUNT; ++i)
	{
		cs.provider[i] = NULL;
	}
	cs.conf_content = NULL;
	cs.conf_content_len = 0;
	cs.conf_page_cnt = 0;

	errcode = CS_PlatformInit(&cs);
	if (errcode != CS_OK)
	{
		CS_SYS_Error("Platform initialization failed.");
		return errcode;
	}

	CS_SYS_Info("Platform initialized.");

	// Add SYS service provider.
	CS_RegisterProvider(&cs_sys_prov);

	return CS_OK;
}

int CS_RegisterProvider(struct CS_Provider_Struct* provider)
{
	if (provider == NULL ||
		provider->request_handler == NULL)
	{
		CS_SYS_Error("Failed to register provider.");
		return CS_ERROR;
	}

	for (int i = 0; i < cs.provider_cnt; ++i)
	{
		if(cs.provider[i]->id == provider->id)
		{
			CS_SYS_Error("Non unique provider ID.");
			return CS_ERROR;
		}
	}

	if (cs.provider_cnt == CS_MAX_PROVIDER_COUNT)
	{
		CS_SYS_Error("Reached maximum provider count.");
		return CS_ERROR;
	}

	cs.provider[cs.provider_cnt] = provider;
	cs.provider_cnt += 1;

	CS_SYS_Info("Registered provider '%u'", cs.provider[cs.provider_cnt - 1]->id);

	return CS_OK;
}

int CS_ProcessRequest(const struct CS_Request_Struct *request)
{
	int errcode, i;
	uint32_t timestamp;
	bool providers_found;

	if (request == NULL)
	{
        return CS_ERROR;
	}

	timestamp = CS_PlatformTime();

#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
    CS_SYS_Info("Received request packet: '" COLORIZE("%u", CYAN, BOLD) "'",
            request);
#else
	CS_SYS_Info("Received request packet: '%u'", request);
#endif

	// Iterate all available providers to find matches.
	for (i = 0; i < cs.provider_cnt; ++i)
	{
		if (request->provider_id & cs.provider[i]->id)
		{
			// Matching provider was found -> pass request
			errcode = cs.provider[i]->request_handler(request);

			if (errcode == CS_OK)
			{
#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
                CS_SYS_Info(
                        "Response packet: '");
#else
				CS_SYS_Info("Response packet: '");
#endif
				for(int j = 0; j < MAX_DATA_LEN_HW; ++j)
				{
#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
                CS_SYS_Info(COLORIZE("%u", MAGENTA, BOLD) " ", cs_prov_response[j]);
#else
				CS_SYS_Info(" %u ", cs_prov_response[j]);
#endif
				}
#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
                CS_SYS_Info(" '");
#else
				CS_SYS_Info(" '");
#endif


				// Send response to platform
				int res_len = sizeof(cs_prov_response);
				if (CS_PlatformWriteBytes(cs_prov_response, res_len, cs.provider[i]->id) == CS_OK)
				{
					timestamp = CS_PlatformTime() - timestamp;
					CS_SYS_Verbose("Request completed in %lu ms.", timestamp);
				}
				else
				{
					CS_SYS_Error("Platform send failed. (errcode=%d)", errcode);
					return CS_ERROR;
				}
			}
			else if (errcode == CS_NO_RESPONSE)
			{
				providers_found = true;
			}
			else
			{
				CS_SYS_Error("Provider request processing error. (errcode=%d)", errcode);
				sprintf(cs_tx_buffer, "%u/e/UNK_ERROR", cs.provider[i]->id);
				errcode = CS_PlatformWriteString(cs_tx_buffer, strlen(cs_tx_buffer), cs.provider[i]->id);
				if (errcode != CS_OK)
				{
					CS_SYS_Error("Platform send failed. (errcode=%d)", errcode);
					return CS_ERROR;
				}
				// Provider failed to process request
				return CS_ERROR;
			}
		}
	}

	if(providers_found) return CS_OK;
	else{
		CS_SYS_Error("No matching providers found for '%u'", request->provider_id);
		sprintf(cs_tx_buffer, "%u/e/UNK_PROV", request->provider_id);
		errcode = CS_PlatformWriteString(cs_tx_buffer, strlen(cs_tx_buffer), CSP_SYS_ID);
		if (errcode != CS_OK)
		{
			CS_SYS_Error("Platform send failed. (errcode=%d)", errcode);
			return CS_ERROR;
		}
	}

	return CS_ERROR;
}

int CS_PollProviders(void)
{
    for (int i = 0; i < cs.provider_cnt; ++i)
    {
        if ((cs.provider[i]->req_token & START) &&
        		cs.provider[i]->poll_handler != NULL)
        {
            cs.provider[i]->poll_handler();
        }
    }

    return CS_OK;
}

int CS_InjectResponse(uint16_t response[MAX_DATA_LEN_HW], uint8_t provider_id, \
		uint8_t response_len)
{
    int errcode;

    // check response length
    if (response_len > CS_MAX_RESPONSE_LENGTH)
    {
        CS_SYS_Error("Attempting to inject too long response packet.");
        return CS_ERROR;
    }

#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
                CS_SYS_Info(
                        "Response packet: '");
#else
				CS_SYS_Info("Response packet: '");
#endif
				for(int i = 0; i < MAX_DATA_LEN_HW; ++i)
				{
#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
                CS_SYS_Info(COLORIZE("%u", MAGENTA, BOLD) " ", response);
#else
				CS_SYS_Info(" %u ", response[i]);
#endif
				}
#if CS_LOG_WITH_ANSI_COLORS != 0 && defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION
                CS_SYS_Info(" '");
#else
				CS_SYS_Info(" '");
#endif

	// Protect data from changes while transmitting
	//int16_t data[MAX_DATA_SIZE_HW];
	//memcpy(data, response, response_len);

	// Send response to platform
    errcode = CS_PlatformWriteBytes(response, response_len, provider_id);
    if (errcode == CS_OK)
    {
        return CS_OK;
    }
    else
    {
        CS_SYS_Error("Platform send failed. (errcode=%d)", errcode);
        return CS_ERROR;
    }
}

int CS_SetPowerMode(enum CS_PowerMode mode)
{
    for (int i = 0; i < cs.provider_cnt; ++i)
    {
        if (cs.provider[i]->power_handler != NULL)
        {
            cs.provider[i]->power_handler(mode);
        }
    }

    return CS_OK;
}

void CS_SetAppConfig(const char* content)
{
	if (content == NULL)
	{
		return;
	}

	cs.conf_content_len = strlen(content);
	if (cs.conf_content_len == 0)
	{
		return;
	}

	cs.conf_page_cnt = (cs.conf_content_len / CS_TEXT_PAGE_LEN);
	if (cs.conf_page_cnt * CS_TEXT_PAGE_LEN < cs.conf_content_len)
	{
		cs.conf_page_cnt += 1;
	}

	cs.conf_content = content;

	CS_SYS_Info("Set app config. %dB long, %d pages.", cs.conf_content_len, cs.conf_page_cnt);
}

void CS_Log(enum CS_Log_Level level, const char* module, const char* fmt, ...)
{

#if defined RTE_DEVICE_BDK_OUTPUT_REDIRECTION && CS_LOG_WITH_ANSI_COLORS != 0
    static const char* log_level_str[] = {
          COLORIZE("ERROR", RED, BOLD),
          COLORIZE("WARN", YELLOW, BOLD),
          COLORIZE("INFO", GREEN),
          "VERBOSE"
      };
#else
    static const char* log_level_str[] = {
        "ERROR",
        "WARN",
        "INFO",
        "VERBOSE"
    };
#endif



	if (module != NULL && fmt != NULL)
	{
		CS_PlatformLogLock();

		CS_PlatformLogPrintf("[CS %s][%s] ", log_level_str[level], module);
		va_list args;
		va_start(args, fmt);
		CS_PlatformLogVprintf(fmt, args);
		va_end(args);
		CS_PlatformLogPrintf("\r\n");

		CS_PlatformLogUnlock();
	}
}

static int CSP_SYS_RequestHandler(const struct CS_Request_Struct* request)
{
	return CS_OK;
}
