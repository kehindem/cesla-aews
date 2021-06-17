// ----------------------------------------------------------------------------
// Copyright (c) 2021 McMaster University
// CS_Providers.h
// Author: 	CESLA 
// ----------------------------------------------------------------------------

#ifndef _CS_PROVIDERS_H_
#define _CS_PROVIDERS_H_

#include "RTE_Components.h"
#include <stdbool.h>

//-----------------------------------------------------------------------------
// PROVIDER ID
//-----------------------------------------------------------------------------
#define CSP_DMIC_ID 	0b00001
#define CSP_LCA_ID		0b00010
#define CSP_RCA_ID		0b00100
#define CSP_LCF_ID		0b01000
#define CSP_RCF_ID		0b10000

//-----------------------------------------------------------------------------
// PROVIDER AVAILABILITY BIT
//-----------------------------------------------------------------------------
#define CSP_DMIC_AVAIL_BIT ((uint32_t)0x00000000)
#define CSP_LCA_AVAIL_BIT ((uint32_t)0x00000000)
#define CSP_RCA_AVAIL_BIT ((uint32_t)0x00000000)
#define CSP_LCF_AVAIL_BIT ((uint32_t)0x00000000)
#define CSP_RCF_AVAIL_BIT ((uint32_t)0x00000000)

//-----------------------------------------------------------------------------
// LOGGING
//-----------------------------------------------------------------------------
// Shortcut macros for logging of DMIC provider messages.
#define CSP_DMIC_Error(...) CS_LogError("DMIC", __VA_ARGS__)
#define CSP_DMIC_Warn(...) CS_LogWarning("DMIC", __VA_ARGS__)
#define CSP_DMIC_Info(...) CS_LogInfo("DMIC", __VA_ARGS__)
#define CSP_DMIC_Verbose(...) CS_LogVerbose("DMIC", __VA_ARGS__)


//-----------------------------------------------------------------------------
// PROVIDERS
//-----------------------------------------------------------------------------
#include <ccs/providers/CSP_LP_DMIC.h>
#include <ccs/providers/CSP_LP_LCA.h>
#include <ccs/providers/CSP_LP_RCA.h>

#endif /* CS_PROVIDERS_H_ */
