//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//! \file BLE_CCS.h
//! \version v1.0.0
//!
//! \addtogroup BDK_GRP
//! \{
//! \addtogroup BLE_GRP
//! \{
//! \addtogroup CESLA Custom Service Profile - Remote Microphone and Feature Extraction
//!
//! \brief BLE profile for bidirectional communication between BDK and client
//! device.
//!
//! Three characteristics are provided to act as stream control point for receiving requests
//! from client device, and raw audio and sound features lines for providing responses.<br>
//! Size of message that can be send and received over these characteristics is
//! limited to 20 bytes.<br>
//! Transmitted data are application specific and can be in binary form or as
//! readable AT commands.
//!
//! \warning Custom Service Profile uses message handlers registered under
//! application task (TASK_APP) to communicate with GATTM and GATTC tasks which
//! manage attribute database and active connections.
//! Therefore there cannot be second user defined profile handled by
//! application task at the same time as they would collide.<br>
//! This concerns only profiles that are not directly supported by RSL10 BLE
//! stack which have assigned their own tasks by event kernel.<br>
//! Following message handlers will be added to application task:
//!     * GATTM_ADD_SVC_RSP
//!     * GATTC_READ_REQ_IND
//!     * GATTC_WRITE_REQ_IND
//!     * GATTC_ATT_INFO_REQ_IND
//!     * GATTC_CMP_EVT
//!
//! \b Example: \n
//! Minimal code example which uses Custom Service.
//! It echoes all data written to RX characteristic back to client by sending
//! notifications over TX characteristic with the same  data.
//! \include ics_example.c
//! \{
//-----------------------------------------------------------------------------

#ifndef BLE_CCS_H
#define BLE_CCS_H

#include "BLE_PeripheralServer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief CESLA RMFE Custom Service UUID */
#define CCS_SERVICE_UUID                	{ 0x24, 0xdc, 0x0e, 0x6e, 0x01, 0x40, \
                                          	0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xb5, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service Stream Control Point Characteristic UUID */
#define CCS_SCP_CHARACTERISTIC_UUID  		{ 0x24, 0xdc, 0x0e, 0x6e, 0x02, 0x40, \
                                          	0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xb6, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service DMIC Raw Audio Characteristic UUID */
#define CCS_DMIC_CHARACTERISTIC_UUID     	{ 0x24, 0xdc, 0x0e, 0x6e, 0x04, 0x40, \
											0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xb7, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service Left Channel Raw Audio Characteristic UUID */
#define CCS_LCA_CHARACTERISTIC_UUID      	{ 0x24, 0xdc, 0x0e, 0x6e, 0x04, 0x40, \
											0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xb8, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service Right Channel Raw Audio Characteristic UUID */
#define CCS_RCA_CHARACTERISTIC_UUID      	{ 0x24, 0xdc, 0x0e, 0x6e, 0x04, 0x40, \
											0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xb9, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service Left Channel Sound Features Characteristic UUID */
#define CCS_LCF_CHARACTERISTIC_UUID      	{ 0x24, 0xdc, 0x0e, 0x6e, 0x03, 0x40, \
											0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xba, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service Right Channel Sound Features Characteristic UUID */
#define CCS_RCF_CHARACTERISTIC_UUID      	{ 0x24, 0xdc, 0x0e, 0x6e, 0x03, 0x40, \
											0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xbb, 0xf3, 0x93, 0xe0 }

/** \brief CESLA RMFE Service Alert Speech Control Point Characteristic UUID */
#define CCS_ASCP_CHARACTERISTIC_UUID      	{ 0x24, 0xdc, 0x0e, 0x6e, 0x03, 0x40, \
											0xca, 0x9e, 0xe5, 0xa9, 0xa3, 0x00, \
											0xbc, 0xf3, 0x93, 0xe0 }

/** \brief Human readable Stream Control Point characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of SCP
 * characteristic.
 */
#define CCS_SCP_CHARACTERISTIC_NAME      "STREAM_CONTROL_POINT - Write - Start/Stop Audio/Features"

/** \brief Human readable Right Channel Sound Features characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of RCF
 * characteristic.
 */
#define CCS_RCF_CHARACTERISTIC_NAME      "SOUND_FEATURES (Right Channel) - Notification"

/** \brief Human readable Left Channel Sound Features characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of LCF
 * characteristic.
 */
#define CCS_LCF_CHARACTERISTIC_NAME      "SOUND_FEATURES (Left Channel) - Notification"

/** \brief Human readable Right Channel Audio characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of RCA
 * characteristic.
 */
#define CCS_RCA_CHARACTERISTIC_NAME      "AUDIO (Right Channel) - Notification"

/** \brief Human readable Left Channel Audio characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of LCA
 * characteristic.
 */
#define CCS_LCA_CHARACTERISTIC_NAME      "AUDIO (Left Channel) - Notification"


/** \brief Human readable DMIC Audio characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of DMIC
 * characteristic.
 */
#define CCS_DMIC_CHARACTERISTIC_NAME	 "DMIC_AUDIO - Notification"

/** \brief Human readable Alert Speech Control Point characteristic description.
 *
 * Can be read from <i>Characteristic User Description</i> of ASCP
 * characteristic.
 */
#define CCS_ASCP_CHARACTERISTIC_NAME	 "Request to ALERT user of direction"

#define CCS_SCP_CHARACTERISTIC_NAME_LEN  (sizeof(CCS_SCP_CHARACTERISTIC_NAME) - 1)

#define CCS_RCF_CHARACTERISTIC_NAME_LEN  (sizeof(CCS_RCF_CHARACTERISTIC_NAME) - 1)

#define CCS_RCA_CHARACTERISTIC_NAME_LEN  (sizeof(CCS_RCA_CHARACTERISTIC_NAME) - 1)

#define CCS_LCF_CHARACTERISTIC_NAME_LEN  (sizeof(CCS_LCF_CHARACTERISTIC_NAME) - 1)

#define CCS_LCA_CHARACTERISTIC_NAME_LEN  (sizeof(CCS_LCA_CHARACTERISTIC_NAME) - 1)

#define CCS_DMIC_CHARACTERISTIC_NAME_LEN (sizeof(CCS_DMIC_CHARACTERISTIC_NAME) - 1)

#define CCS_ASCP_CHARACTERISTIC_NAME_LEN (sizeof(CCS_ASCP_CHARACTERISTIC_NAME) - 1)

/** \brief Maximum amount of data that can be either received from RX
 * characteristic or send over TX characteristic.
 *
 * This is limited to 20 characters to accommodate for maximal allowed
 * notification length.
 */
#define CCS_CHARACTERISTIC_VALUE_LENGTH (20)

/** \brief Attribute database indexes of CCS characteristics. */
typedef enum
{
    /* SCP Characteristic */
    CCS_IDX_SCP_VALUE_CHAR,
    CCS_IDX_SCP_VALUE_VAL,
    CCS_IDX_SCP_VALUE_CCC,
    CCS_IDX_SCP_VALUE_USR_DSCP,

	/* DMIC Characteristic */
    CCS_IDX_DMIC_VALUE_CHAR,
    CCS_IDX_DMIC_VALUE_VAL,
    CCS_IDX_DMIC_VALUE_CCC,
    CCS_IDX_DMIC_VALUE_USR_DSCP,

    /* LCA Characteristic */
    CCS_IDX_LCA_VALUE_CHAR,
    CCS_IDX_LCA_VALUE_VAL,
    CCS_IDX_LCA_VALUE_CCC,
    CCS_IDX_LCA_VALUE_USR_DSCP,

	/* RCA Characteristic */
    CCS_IDX_RCA_VALUE_CHAR,
    CCS_IDX_RCA_VALUE_VAL,
    CCS_IDX_RCA_VALUE_CCC,
    CCS_IDX_RCA_VALUE_USR_DSCP,

	/* LCF Characteristic */
    CCS_IDX_LCF_VALUE_CHAR,
    CCS_IDX_LCF_VALUE_VAL,
    CCS_IDX_LCF_VALUE_CCC,
    CCS_IDX_LCF_VALUE_USR_DSCP,

	/* RCF Characteristic */
    CCS_IDX_RCF_VALUE_CHAR,
    CCS_IDX_RCF_VALUE_VAL,
    CCS_IDX_RCF_VALUE_CCC,
    CCS_IDX_RCF_VALUE_USR_DSCP,

	/* ASCP Characteristic */
    CCS_IDX_ASCP_VALUE_CHAR,
    CCS_IDX_ASCP_VALUE_VAL,
    CCS_IDX_ASCP_VALUE_CCC,
    CCS_IDX_ASCP_VALUE_USR_DSCP,

    /* Max number of characteristics */
    CCS_IDX_NB,
} BLE_CCS_AttributeIndex;

/** \brief Initialization state of CCS library. */
enum BLE_CCS_State
{
    BLE_CCS_OFF = 0, /**< Initial state after device power up. */
    BLE_CCS_CREATE_DB, /**< Waiting for attribute database creation confirmation. */
    BLE_CCS_READY, /**< Waiting for client device to connect.  */
    BLE_CCS_CONNECTED /**< Client device connected. Notifications can be send. */
};

/** \brief Data structure passed to application specific Write Indication
 * callback handler.
 */
struct BLE_CCS_RxIndData
{
    /** \brief Stores data received in last write to RX characteristic. */
    uint8_t data[CCS_CHARACTERISTIC_VALUE_LENGTH];

    /** \brief Number of valid data bytes in \ref data array. */
    uint8_t data_len;
};

/** \brief Callback type for handling of RX Write indication events. */
typedef void (*BLE_CCS_RxIndHandler)(struct BLE_CCS_RxIndData *ind);

/** \brief Stores internal state CCS Profile. */
struct BLE_CCS_Resources
{
    /** \brief Stores current initialization and connection state. */
    uint8_t state;

    /** \brief Index of first CCS characteristic in attribute database. */
    uint16_t start_hdl;

    /** \brief Application specific handler for RX characteristic write
     * indication events.
     */
    BLE_CCS_RxIndHandler rx_write_handler;

    uint8_t scp_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t scp_value_length;
    uint16_t scp_cccd_value;

    uint8_t ascp_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t ascp_value_length;
    uint16_t ascp_cccd_value;

    uint8_t dmic_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t dmic_value_length;
    uint16_t dmic_cccd_value;

    uint8_t lca_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t lca_value_length;
    uint16_t lca_cccd_value;

    uint8_t rca_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t rca_value_length;
    uint16_t rca_cccd_value;

    uint8_t lcf_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t lcf_value_length;
    uint16_t lcf_cccd_value;

    uint8_t rcf_value[CCS_CHARACTERISTIC_VALUE_LENGTH];
    uint8_t rcf_value_length;
    uint16_t rcf_cccd_value;
};

/** \brief Adds CESLA Custom Service into BDK BLE stack.
 *
 * Calling this function will add Custom Service to BLE attribute database.
 *
 * This function does the following initialization steps:
 *     1. Initialize BDK BLE stack by calling BDK_BLE_Initalize.
 *     2. Add message handler callbacks to application task.<br>
 *        These handlers are needed to communicate with GATTM and GATTC tasks.
 *     3. Adds service initialization & enable callbacks that will be executed
 *        during attribute database generation.
 *
 * \pre This functions must be called before Application task is started and
 * before Event Kernel messaging is started, e.g. before entering main loop
 * and calling \ref BDK_Schedule .
 *
 * \param rx_ind_handler
 */
extern void BLE_CCS_Initialize(BLE_CCS_RxIndHandler rx_ind_handler);

/** \brief Send out a notification over corresponding characteristic.
 *
 * TX characteristic will be updated with new data and connected device will
 * receive notification of this change.
 *
 * \param data
 * Data that will be sent in notification packet and stored in the caller's
 * characteristic.
 *
 * \param data_len
 * Length of given data.
 *
 * \returns Operation status code.
 * | Code | Description                                              |
 * | ---- | -------------------------------------------------------- |
 * | 0    | On success.                                              |
 * | 1    | If there is no BLE client device connected.              |
 * | 2    | If length of data is bigger than allowed maximum length. |
 *
 */
extern uint32_t BLE_CCS_Notify(uint8_t *data, uint8_t data_len, BLE_CCS_AttributeIndex idx);

/*
 * Write more than 20 bytes to a characteristic.
 * Notifies first 20 bytes only however stores all bytes in caller's characteristic.
 */
extern uint32_t BLE_CCS_WriteNotify(uint8_t *data, uint16_t data_len, BLE_CCS_AttributeIndex idx);

#ifdef __cplusplus
}
#endif

#endif /* BLE_CCS_H */

//! \}
//! \}
//! \}
