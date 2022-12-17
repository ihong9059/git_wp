#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "shared_appconfig.h"
#include "api.h"
#include "node_configuration.h"

#include "shared_data.h"
#include "app_scheduler.h"
#include "mesh.h"

#include "uttec.h"
#include "wirepas.h"
#include "debug_log.h"
#include "Uart.h"

#define ENABLE_LOW_LATENCY_MODE

void App_init(const app_global_functions_t * functions)
{
    initUttec();
    initSystem();
#ifdef ENABLE_LOW_LATENCY_MODE
    Printf("\r\n++++++++++ Toky bike real Test. 2022.07.28 12:00 ENABLE_LOW_LATENCY_MODE ++++++++++++++\r\n");
#else 
    Printf("\r\n++++++++++ Tokai Encryption type. 2022.12.02 10:55 LATENCY_MODE ++++++++++++++\r\n");
#endif
    
    // Printf("default_network_channel: 11\r\n");
    setOpLed(LED_OFF);
    setMsgLed(LED_OFF);
    // setOpLed(LED_ON);
    // setMsgLed(LED_ON);
    // while(1);

    displayDeviceId();

    // getFlash()->myId = 7; //for test 2022.06.10
    initUart();
    isFirst();

    shared_app_config_filter_t app_config_period_filter;

    /* App config. */
    Shared_Appconfig_init();
    /* Prepare the app_config filter for measurement rate. */
    app_config_period_filter.type = CUSTOM_PERIOD_TYPE;
    app_config_period_filter.cb = appConfigPeriodReceivedCb;
    Shared_Appconfig_addFilter(&app_config_period_filter, &m_filter_id);

    Printf("this is my printf: %d\r\n", 4321);
    if (configureNodeFromBuildParameters() != APP_RES_OK)
    {
        return;
    }

#ifdef ENABLE_LOW_LATENCY_MODE
    lib_settings->setNodeRole(
            app_lib_settings_create_role(APP_LIB_SETTINGS_ROLE_HEADNODE,
                                        APP_LIB_SETTINGS_ROLE_FLAG_LL |
                                        APP_LIB_SETTINGS_ROLE_FLAG_AUTOROLE));
    Printf("+++++++++++++++++  ENABLE_LOW_LATENCY_MODE\r\n");
    // if(isLeafSw()){ // low latency test 2022.10.15
    //     lib_settings->setNodeRole(
    //             app_lib_settings_create_role(APP_LIB_SETTINGS_ROLE_HEADNODE,
    //                                         APP_LIB_SETTINGS_ROLE_FLAG_LL |
    //                                         // APP_LIB_SETTINGS_ROLE_FLAG_LE |
    //                                         APP_LIB_SETTINGS_ROLE_FLAG_AUTOROLE));
    //     Printf("+++++++++++++++++  ENABLE_LOW_LATENCY_MODE\r\n");
    // }
    // else{
    //     lib_settings->setNodeRole(
    //             app_lib_settings_create_role(APP_LIB_SETTINGS_ROLE_HEADNODE,
    //                                         APP_LIB_SETTINGS_ROLE_FLAG_AUTOROLE));
    //     Printf("-----------------  LATENCY_MODE\r\n");
    // }
#endif
    Shared_Data_init();
    App_Scheduler_init();
    m_period_ms = DEFAULT_PERIOD_MS;
    App_Scheduler_addTask_execTime(task_send_periodic_msg,
                                   APP_SCHEDULER_SCHEDULE_ASAP,
                                   PERIODIC_WORK_EXECUTION_TIME_US);

    App_Scheduler_addTask_execTime(task_uttec,
                                   APP_SCHEDULER_SCHEDULE_ASAP,
                                   PERIODIC_WORK_EXECUTION_TIME_US);

    Printf("PERIODIC_WORK_EXECUTION_TIME_US: %d\r\n", PERIODIC_WORK_EXECUTION_TIME_US);
    /* Set unicast & broadcast received messages callback. */
    Shared_Data_addDataReceivedCb(&unicast_packets_filter);
    Shared_Data_addDataReceivedCb(&broadcast_packets_filter);

    lib_state->startStack();

}
