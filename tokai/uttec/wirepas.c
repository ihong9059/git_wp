#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "shared_appconfig.h"
#include "api.h"
#include "node_configuration.h"

// #include "button.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "voltage.h"
#include "uttec.h"

#include "wirepas.h"
#include "mesh.h"

#include "debug_log.h"

uint16_t m_filter_id;
uint32_t m_period_ms;

/** Unicast & broadcast messages handling callback */

/** Unicast messages filter */
shared_data_item_t unicast_packets_filter =
{
    .cb = unicast_broadcast_data_received_cb,
    .filter = {
        .mode = SHARED_DATA_NET_MODE_UNICAST,
        /* Filtering by source endpoint. */
        .src_endpoint = DATA_EP,
        /* Filtering by destination endpoint. */
        .dest_endpoint = DATA_EP,
        .multicast_cb = NULL
    }
};

/** Broadcast messages filter */
shared_data_item_t broadcast_packets_filter =
{
    // .cb = unicast_broadcast_data_received_cb,
    .cb = uttec_broadcast_data_received_cb,
    .filter = {
        .mode = SHARED_DATA_NET_MODE_BROADCAST,
        /* Filtering by source endpoint. */
        .src_endpoint = DATA_EP,
        /* Filtering by destination endpoint. */
        .dest_endpoint = DATA_EP,
        .multicast_cb = NULL
    }
};

app_lib_data_send_res_e send_uplink_msg(message_id_e id,
                                               uint8_t * payload)
{
    static uint32_t ulSendCount = 0;

    msg_t msg; /* Create uplink message structure. */
    size_t msg_byte_size = sizeof(msg.id); /* Message to send byte size. */

    msg.id = (uint8_t)id;
    ulSendCount++;
    switch (msg.id)
    {
        case MSG_ID_PERIODIC_MSG:
            memcpy(&msg.payload.periodic,
                   (payload_periodic_t *)payload,
                   sizeof(payload_periodic_t));

            msg_byte_size += sizeof(payload_periodic_t);
            // Printf("MSG_ID_PERIODIC_MSG, area: %d, myId: %d\r\n",
            Printf("_PERIODIC_MSG, area: %d, myId: %d\r\n",
                msg.payload.periodic.test1, msg.payload.periodic.myId);
            // Printf("test by hong\r\n");    
            break;

        case MSG_ID_ECHO_RESPONSE_MSG:
            msg.id = MSG_ID_PERIODIC_MSG;
            memcpy(&msg.payload.periodic,
                   (payload_periodic_t *)payload,
                   sizeof(payload_periodic_t));
            msg.payload.periodic.counter = ulSendCount;
            msg.payload.periodic.sensor = diaper.sensor;
            msg.payload.periodic.battery = diaper.battery;
            msg.payload.periodic.status = diaper.status;
            msg.payload.periodic.myId = getMyId();
            msg.payload.periodic.test1 = 2;
            msg.payload.periodic.solenoid = 3;

            msg_byte_size += sizeof(payload_periodic_t);

            Printf("MSG_ID_ECHO_RESPONSE_MSG\r\n");
            break;

        case MSG_ID_UTTEC_COMMAND_MSG:
            msg.id = MSG_ID_UTTEC_COMMAND_MSG;
            memcpy(&msg.payload.uttec,
                   (payload_uttec_t *)payload,
                   sizeof(payload_periodic_t));
            msg.payload.periodic.counter = ulSendCount;

            msg_byte_size += sizeof(payload_uttec_t);

            Printf("MSG_ID_UTTEC_COMMAND_MSG\r\n");
            break;

        default:
            Printf("default+++++++++++++\r\n");
            /* Invalid message ID given : send only invalid msg ID. */
            break;
    }
    // msg_byte_size = 13;
    /* Create a data packet to send. */
    app_lib_data_to_send_t data_to_send;
    data_to_send.bytes = (const uint8_t *) &msg;
    data_to_send.num_bytes = msg_byte_size;
    data_to_send.dest_address = APP_ADDR_ANYSINK;
    data_to_send.src_endpoint = DATA_EP;
    data_to_send.dest_endpoint = DATA_EP;
    data_to_send.qos = APP_LIB_DATA_QOS_HIGH;
    data_to_send.delay = 0;
    data_to_send.flags = APP_LIB_DATA_SEND_FLAG_NONE;
    data_to_send.tracking_id = APP_LIB_DATA_NO_TRACKING_ID;

    // toggleMsgLed();
    /* Send the data packet. */
    // static uint32_t upCount = 0;
    // Printf("uplink: %d, size: %d\r\n", upCount++, msg_byte_size);
    return Shared_Data_sendData(&data_to_send, NULL);
}

void appConfigPeriodReceivedCb(uint16_t type,
                                      uint8_t length,
                                      uint8_t * value_p)
{
    app_config_t * config;

    if (type != CUSTOM_PERIOD_TYPE)
    {
        /* It should never happen as we registered only this type with this cb. */
        LOG(LVL_ERROR, "Wrong app config type");
        return;
    }

    if (length != sizeof(app_config_t))
    {
        /* Wrong size. */
        LOG(LVL_ERROR, "Wrong app config size");
        return;
    }

    config = (app_config_t *) value_p;

    LOG(LVL_INFO,
        "New app configuration  interval_s=%d",
        config->interval);

    /* Set new periodic data transfer interval. */
    set_periodic_msg_period(config->interval*1000);

}

void initSystem(void){
/** uart initial **/
    LOG_INIT();
}
