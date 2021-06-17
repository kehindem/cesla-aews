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
//!
//!
//-----------------------------------------------------------------------------

#ifndef RTE_APP_CONFIG_H_
#define RTE_APP_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>


// <o> BLE Advertising Interval [ms] <10-1000000>
// <i> Determines how often to send advertising packets.
// <i> Default: 1000 ms
#ifndef RTE_APP_BLE_ADV_INT
#define RTE_APP_BLE_ADV_INT  1000
#endif

// <s.20> BLE Complete Local Name
// <i> Advertised name of this device.
// <i> Default: HB_BLE_Terminal
#ifndef RTE_APP_BLE_COMPLETE_LOCAL_NAME
#define RTE_APP_BLE_COMPLETE_LOCAL_NAME  "CESLA_BLE_Terminal"
#endif

// <o> Advertising Stop Timeout [s] <1-1000>
// <i> Default: 60 s
#ifndef RTE_APP_ADV_DISABLE_TIMEOUT
#define RTE_APP_ADV_DISABLE_TIMEOUT  60
#endif

// <o> Wake-up Button Check Interval [ms] <10-1000000>
// <i> Default: 1000 ms
#ifndef RTE_APP_BTN_CHECK_TIMEOUT
#define RTE_APP_BTN_CHECK_TIMEOUT  1500
#endif

// <o> I2C Bus Speed
// <i> Default: Fast+
// <0=> Standard
// <1=> Fast
// <2=> Fast+
#ifndef RTE_APP_I2C_BUS_SPEED
#define RTE_APP_I2C_BUS_SPEED  2
#endif

// <h> CESLA Custom Service


// <o> Audio Sampling Rate (Hz)
// <i> Of analog microphones.
// <i> Default: 1250Hz
// <0=> 1250Hz
// <1=> 12500Hz
// <2=> 25000Hz
// <3=> 50000Hz
#ifndef RTE_APP_ADC_SAMPLING_RATE
#define RTE_APP_ADC_SAMPLING_RATE  0
#endif

// <e> Left Channel Audio Stream (LCA)
// <i> Provide audio stream from left channel microphone over CCS Custom Service.
// <i> Default: Enabled
#ifndef RTE_APP_CCS_LCA_ENABLED
#define RTE_APP_CCS_LCA_ENABLED  1
#endif

// </e>

// <e> Right Channel Audio Stream (RCA)
// <i> Provide audio stream from right channel microphone over CCS Custom Service.
// <i> Default: Enabled
#ifndef RTE_APP_CCS_RCA_ENABLED
#define RTE_APP_CCS_RCA_ENABLED  1
#endif

// </e>

// <e> Digital Microphone Audio Stream (DMIC)
// <i> Provide audio stream from on-board DMIC over CCS Custom Service.
// <i> Default: Enabled
#ifndef RTE_APP_CCS_DMIC_ENABLED
#define RTE_APP_CCS_DMIC_ENABLED  1
#endif

// </e>

// <e> Left Channel Features (LCF)
// <i> Provide left channel features data over CCS Custom Service.
// <i> Default: Disabled (not yet implemented)
#ifndef RTE_APP_CCS_LCF_ENABLED
#define RTE_APP_CCS_LCF_ENABLED  0
#endif

// </e>

// <e> Right Channel Features (RCF)
// <i> Provide right channel features data and ILD over CCS Custom Service.
// <i> Default: Disabled (not yet implemented)
#ifndef RTE_APP_CCS_RCF_ENABLED
#define RTE_APP_CCS_RCF_ENABLED  0
#endif

// </e>

// </h>


// <<< end of configuration section >>>

#endif /* RTE_APP_CONFIG_H_ */

