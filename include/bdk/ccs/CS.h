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
//
// 2021 McMaster University
// CESLA
// March 11, 2021
// ----------------------------------------------------------------------------

#ifndef _CS_H_
#define _CS_H_

//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------

#include <ccs/CS_Log.h>
#include <ccs/CS_Providers.h>
#include <stddef.h>
#include <stdint.h>

#include "RTE_CS_Feature.h"


#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------

/** Maximum number of data bytes that can be fit inside of a response packet. */
#define CS_MAX_RESPONSE_LENGTH         	(20)

/** Maximum number of half words that can be fit inside of a response packet. */
#define MAX_DATA_LEN_HW 				( CS_MAX_RESPONSE_LENGTH/sizeof(uint16_t) )


//-----------------------------------------------------------------------------
// EXPORTED DATA TYPES DEFINITION
//-----------------------------------------------------------------------------
typedef uint8_t CS_Reserved;
typedef uint8_t CS_OpCode;
typedef uint8_t CS_Provider;

enum CS_PowerMode
{
    /** \brief Enter normal operation mode.
     *
     * Provider does not need to implement this if it can be activated on-demand
     * when request handler is called.
     */
    CS_POWER_MODE_NORMAL = 0,

    /** \brief Enter low power mode.
     *
     * Disable all external sensors, etc.
     * Providers will be usually set to sleep mode when connected BLE central device
     * disconnects.
     *
     * Providers have to be able to automatically exit sleep mode when its request
     * handler is called.
     */
    CS_POWER_MODE_SLEEP  = 1,
};


/** \brief Structure passed to providers via request handler.
 *
 * Request handler must not modify values of any member pointers.
 * Note: system is little-endian. Future use includes:
 *  i.e. initiate transmission of configuration packets
 *  		from central device
 *
 *    7    6    5    4    3    2    1    0
 * +----+----+----+----+----+----+----+----+
 * | R  |    OP   |           PID          |
 * +----+----+----+----+----+----+----+----+
 *
 * Bit [7]    : Reserved		        (for future use)
 * Bit [5-6]  : Op Code 				(0 = STOP, 2 = START_RELEASE, 3 = START_DEBUG)
 * Bit [0-4]  : Provider ID		        (1 = DMIC, 2 = LCA, 4 = RCA, 8 = LCF, 16 = RCF)
 *
 */
// TODO: Make provision for changing sampling rate remotely over BLE
struct CS_Request_Struct
{
	/** \brief Bits containing provider ID bit mask.
	 * DMIC raw audio						 	xxx00001
	 * Left channel microphone audio	 		xxx00010
	 * Right channel microphone audio	 		xxx00100
	 * Left channel microphone sound features	xxx01000
	 * Right channel microphone sound features	xxx10000
	 */
	const CS_Provider provider_id :5;

	/** \brief Bits containing the op code bit mask.
	 * <<Start>> 								1xxxxxxx
	 * <<Stop>> 								0xxxxxxx
	 * <<Debug>> 								x1xxxxxx
	 * <<Release>>	 							x0xxxxxx
	 * */
	const CS_OpCode op_code : 2;

	/** \brief Reserved bit.
	 * Reserved for future use
	 * Default									xx0xxxxx
	 * Configure								xx1xxxxx
	 * */
	const CS_Reserved reserved : 1;
};

typedef enum
{
	STOP = 0b00,
	START = 0b10
} CS_StreamCommand;

/** Bitmask encoding for development mode.
 *
 */
typedef enum
{
	RELEASE = 0b00,
	DEBUG 	= 0b01
} CS_DevMode;

/** Bitmask encoding for operational code.
 *
 *    1    0
 * +----+----+
 * |S/S |D/R |
 * +----+----+
 *
 * Bit [1]	  : Start/Stop Streaming   		(0: Stop, 1 = Start)
 * Bit [0]	  : Debug/Release	       		(0: Release, 1 = Debug)
 *
 */
enum CS_OpCode
{
	STOP_STREAMING 				=		STOP,
	START_STREAMING_RELEASE 	= 		START | RELEASE,
	START_STREAMING_DEBUG 		= 		START | DEBUG,
};



enum CS_Reserved
{
	DEFAULT = 0,
	N_A = 1
};


/** Bitmask encoding for stream provider.
 *
 *    4    3    2    1    0
 * +----+----+----+----+----+
 * |RCF |LCF | RCA|LCA | D  |
 * +----+----+----+----+----+
 *
 * Bit [0]	  : DMIC	        			(0: Disable, 1 = Enable)
 * Bit [1]	  : Left Channel Audio	       	(0: Disable, 1 = Enable)
 * Bit [2]	  : Right Channel Audio	        (0: Disable, 1 = Enable)
 * Bit [3]	  : Left Channel Features	    (0: Disable, 1 = Enable)
 * Bit [4]	  : Right Channel Features	    (0: Disable, 1 = Enable)
 *
 */
enum CS_Provider
{
	DMIC_AUDIO 				= 		CSP_DMIC_ID,
	LEFT_CHNL_AUDIO 		= 		CSP_LCA_ID,
	RIGHT_CHNL_AUDIO 		= 		CSP_RCA_ID,
	LEFT_CHNL_FEATURES 		= 		CSP_LCF_ID,
	RIGHT_CHNL_FEATURES 	= 		CSP_RCF_ID,
};


/** \brief Function prototype for provider request handler.
 *
 * \param[in] request
 * Details of the request passed to the provider.
 *
 * \param[out] response
 * Pointer to array where provider can print its response to request.
 * It does not contain request token.
 * Maximum number of bytes that can be written to this array is
 * \ref CS_MAX_RESPONSE_LENGTH excluding the terminating character.
 *
 * \returns CS_OK when request was successfully processed and response contains
 * valid zero terminated string.
 * \returns CS_ERROR when request processing failed or response does not contain
 * valid string.
 * \returns CS_NO_RESPONSE when request was processed but no response is to be
 * send back to connected device.
 */
typedef int (*CS_RequestHandler)(const struct CS_Request_Struct* request);

/** \brief Function prototype for provider power manager function.
 *
 * \param[in] mode
 * New desired power mode level.
 *
 * \returns CS_OK on success.
 */
typedef int (*CS_PowerModeHandler)(enum CS_PowerMode dev_mode);

/** \brief Function prototype for provider poll function that is to be executed
 * from main loop.
 */
typedef void (*CS_PollHandler)(void);

struct CS_Provider_Struct
{
	CS_Provider id;

	uint32_t req_token;

	/** \brief Function that will be called when a request addressed to this
	 * provider is received.
	 */
	CS_RequestHandler request_handler;

	/** \brief Function for managing of provider power management.
	 *
	 * It is used to put provider to sleep when no peer device is connected.
	 */
	CS_PowerModeHandler power_handler;

	/** \brief Polling function that is called from main loop to handle internal
	 * processes of the provider.
	 *
	 * This function is optional and can be set to NULL if not required.
	 */
	CS_PollHandler poll_handler;
};

struct CS_Handle_Struct
{
	int provider_cnt;
	struct CS_Provider_Struct* provider[CS_MAX_PROVIDER_COUNT];

	const char* conf_content;
	int conf_content_len;
	int conf_page_cnt;
};

enum CS_ErrorCodes
{
	CS_OK          = 0, /* \brief Generic success return value. */
	CS_ERROR       = 1, /* \brief Generic error return value. */
	CS_TIMEOUT     = 2,
	CS_NO_RESPONSE = 3
};


//-----------------------------------------------------------------------------
// EXPORTED FUNCTION DECLARATIONS
//-----------------------------------------------------------------------------

extern int CS_Init();

extern int CS_RegisterProvider(struct CS_Provider_Struct* provider);

/**
 *
 * \param[in] request
 * Pointer to integer that contains request payload.
 */
extern int CS_ProcessRequest(const struct CS_Request_Struct *request);

/** */
extern int CS_PollProviders(void);

/**
 *
 * \param response
 * Pointer to buffer containing the whole response packet to
 * be sent.
 */
extern int CS_InjectResponse(uint16_t response[MAX_DATA_LEN_HW], uint8_t provider_id, \
		uint8_t response_len);

extern int CS_SetPowerMode(enum CS_PowerMode mode);

//extern void CS_SetAppConfig(const char* content);

#ifdef __cplusplus
}
#endif


#endif /* _CS_H_ */
