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
#include <stdio.h>

#include "app.h"
#include "RTE_app_config.h"

enum App_StateStruct app_state = APP_STATE_INIT;
struct stimer app_state_timer;

int main(void)
{
    Device_Initialize();

    /* Indication - Initialization complete. */
    LED_On(LED_GREEN);
    HAL_Delay(250);
    LED_Off(LED_GREEN);

    Main_Loop();
}

static void App_StateMachine(void)
{
    switch (app_state)
    {
    case APP_STATE_INIT:
        TRACE_PRINTF("State: Init\r\n");

        stimer_init(&app_state_timer, Timer_GetContext());
        /* no break */
    case APP_STATE_START_ADVERTISING:
        TRACE_PRINTF("State: Advertising start\r\n");

        // Set timeout to disable advertising if not connection is established
        // for RTE_APP_ADV_DISABLE_TIMEOUT seconds.
        stimer_expire_from_now_s(&app_state_timer, RTE_APP_ADV_DISABLE_TIMEOUT);

        // Start BLE advertising
        BDK_BLE_AdvertisingStart();

        // Signal to user.
        LED_On(LED_GREEN);
        HAL_Delay(APP_STATE_IND_LED_INTERVAL_MS);
        LED_Off(LED_GREEN);

        app_state = APP_STATE_ADVERTISING;
        break;

    case APP_STATE_ADVERTISING:
        TRACE_PRINTF("State: Advertising\r\n");

        // Check if advertisement stop timeout has elapsed.
        if (stimer_is_expired(&app_state_timer) == true)
        {
            // Set timeout to periodically check button state while advertising
            // is disabled.
            stimer_expire_from_now_ms(&app_state_timer,
                    RTE_APP_BTN_CHECK_TIMEOUT);

            // Stop advertising
            BDK_BLE_AdvertisingStop();

            LED_On(LED_RED);
            HAL_Delay(APP_STATE_IND_LED_INTERVAL_MS);
            LED_Off(LED_RED);

            // Enter sleep state
            app_state = APP_STATE_SLEEP;
        }
        break;

    case APP_STATE_SLEEP:
        TRACE_PRINTF("State: Sleep\r\n");

        // Schedule next wake up with the same button check interval
        stimer_advance(&app_state_timer);

        BTN_Initialize(BUTTON0);

        // Check button state
        if (BTN_Read(BUTTON0) == BTN_PRESSED)
        {
            // Set timeout to disable advertising if not connection is
            // established for RTE_APP_ADV_DISABLE_TIMEOUT seconds.
            stimer_expire_from_now_s(&app_state_timer,
                    RTE_APP_ADV_DISABLE_TIMEOUT);

            // Start BLE advertising
            BDK_BLE_AdvertisingStart();

            // Signal wake up event to user.
            LED_On(LED_GREEN);
            HAL_Delay(APP_STATE_IND_LED_INTERVAL_MS);
            LED_Off(LED_GREEN);

            app_state = APP_STATE_ADVERTISING;
        }
        break;

    case APP_STATE_START_CONNECTION:
        TRACE_PRINTF("State: Start connection\r\n");

        // Stop advertising timeout timer.
        stimer_stop(&app_state_timer);

        // Switch to connected state.
        app_state = APP_STATE_CONNECTED;
        break;

    case APP_STATE_CONNECTED:
        TRACE_PRINTF("State: Connected\r\n");
        // Nothing to do, wait for peer device to disconnect.
        // Next state change will be from peer device disconnect hook.
        break;
    }
}

void Main_Loop(void)
{
    bool sleep_allowed = false;

    TRACE_PRINTF("Main_Loop: Enter\r\n");

    while (1)
    {
        /* Refresh watchdog timer. */
        Sys_Watchdog_Refresh();

        CS_PollProviders();

        /* Execute any events that have occurred. */
        Kernel_Schedule();

        /* Application stuff follows here. */
        App_StateMachine();

        /* Set RTC wake up event to nearest timer. */
        if ( Timer_SetWakeupAtNextEvent() != APP_TIMER_ALARM_NOW)
        {
            /* Prepare device for entering deep sleep mode. */
            trace_deinit();
            HAL_I2C_DeInit();

            /* Attempt to enter deep sleep mode.
             * BLE_Power_Mode_Enter will not return when deep sleep is
             * entered. */
            __disable_irq();
            sleep_allowed = BLE_Power_Mode_Enter(&sleep_mode_env,
                    POWER_MODE_SLEEP);
            __enable_irq();

            /* Re-enable basic functionality needed by main loop until device is
             * ready to enter deep sleep. */
            trace_init();
            if (sleep_allowed == false)
            {
                TRACE_PRINTF("Main_Loop: sleep not allowed\r\n");
            }
        }

        /* Enter sleep mode until an interrupt occurs. */
        SYS_WAIT_FOR_INTERRUPT;
    }
}
