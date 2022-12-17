#ifndef __MEASH_H_
#define __MEASH_H_

// 0:p2, 1:p3, 2:p4, 3:p5, 4:p28, 5:p29, 6:p30, 7:p31
#define DIAPER_CH 0
#define BATTERY_CH 2

typedef struct{
    uint32_t counter;
    uint8_t status;
    short sensor;
    short battery;
} diaper_t;

extern diaper_t diaper;


uint32_t task_send_periodic_msg(void);

uint32_t task_uttec(void);

void appConfigPeriodReceivedCb(uint16_t type, 
                                      uint8_t length,
                                      uint8_t * value_p);

void initSystem(void);

void set_periodic_msg_period(uint32_t new_period_ms);

app_lib_data_receive_res_e unicast_broadcast_data_received_cb(
        const shared_data_item_t * item,
        const app_lib_data_received_t * data);

app_lib_data_receive_res_e uttec_broadcast_data_received_cb(
        const shared_data_item_t * item,
        const app_lib_data_received_t * data);

void setPowerFlag(bool);
void setLockFlag(bool);

#endif /* __INITIAL_H_ */
