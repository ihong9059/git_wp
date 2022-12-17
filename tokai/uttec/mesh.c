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
#include "Uart.h"

#include "debug_log.h"
#include "persistent.h"

diaper_t diaper = {0, 0, 0, 0};

uint32_t task_send_periodic_msg(void)
{

    static bool first = true;

    payload_periodic_t payload; /* Message payload data. */

    payload.counter = diaper.counter;
    payload.sensor = diaper.sensor;
    payload.battery = diaper.battery;
    payload.status = *getRackStatus();
    payload.myId = getMyId();
    payload.test1 = getAreaId();
    payload.solenoid = diaper.status;

    if(first){
        first = false;
        payload.sensor = payload.battery = 1280;
        Printf("************ first sensor: %d\r\n", payload.sensor);
        Printf("************ first battery: %d\r\n", payload.battery);
    }

    if (send_uplink_msg(MSG_ID_PERIODIC_MSG,
                        (uint8_t *)&payload) != APP_LIB_DATA_SEND_RES_SUCCESS)
    {
    }
    return m_period_ms;
}

short averageDiaper(short diaper){
    static int total = 1024 * 10;
    total -= total/10;
    total += diaper;
    return total/10;
}
short averageBattery(short diaper){
    static int total = 1024 * 10;
    total -= total/10;
    total += diaper;
    return total/10;
}

#define BeepDelay 2
bool powerFlag = false;
bool lockFlag = false;
bool allFlag = false;

bool* getPowerFlag(void){
    return &powerFlag;
}

bool* getLockFlag(void){
    return &lockFlag;
}

#include <stdlib.h>

#define SOH '{'
#define EOT '}'
#define OK_FLAG '@'
#define ADDR_SET_TIMEOUT 100 // 10 Seconds time out 
#define ADDR_BUF_LENGTH   10
void setAddress(void){
    static char addrBuf[ADDR_BUF_LENGTH] = {0,};
    static uint16_t uiUartTimeout = 0;
    static uint8_t ucAddressStatus = 0;
    static uint8_t ucAddrCount = 0;
    
    if(uiUartTimeout) uiUartTimeout--; //check address setting timeout
    else{ // when timeout, reset factor
        setOpLed(LED_OFF);
        setMsgLed(LED_OFF);
        ucAddrCount = ucAddressStatus = 0;
        for(int i = 0; i < ADDR_BUF_LENGTH; i++) addrBuf[i] = 0;
    }

    if(isRx()){
        uint8_t rx = *getRx();
        if(rx == SOH){
            setOpLed(LED_OFF);
            setMsgLed(LED_OFF);
            uiUartTimeout = ADDR_SET_TIMEOUT;
            // Printf("00000000 address ready\r\n");
            setMsgLed(LED_ON);
            ucAddressStatus = 1; ucAddrCount = 0;
            for(int i = 0; i < ADDR_BUF_LENGTH; i++) addrBuf[i] = 0;
        }
        else if((rx == EOT)&&(ucAddressStatus == 1)){
            // Printf("11111111 address end\r\n");
            setOpLed(LED_ON);
            ucAddressStatus = 2;
        }
        else if((rx == OK_FLAG)&&(ucAddressStatus == 2)){
            // Printf("{\"rack\":%d}\r\n", rx);
            setOpLed(LED_OFF);
            setMsgLed(LED_OFF);
            parseAddress(addrBuf);
        }
        else{
            if(ucAddressStatus == 1){
                addrBuf[ucAddrCount++] = rx;
            }
        }
        // Printf("++++++ input: %c\r\n", rx);
    }
}

void actFirst(void){
    if(isLeafSw()){// bike occupied
    // occupy_unlock, occupy_lock
        // *getRackStatus() = occupy_lock;
        *getRackStatus() = occupy_unlock;
        *getBikeLockStatus() = bikeLock;
        *getBeforeLeafSw() = leafsw_on;
        // setPowerFlag(true);
        // setLockFlag(true);
        Printf("initial bike occupied and locking\r\n");
    }
    else{// bike empty
        *getRackStatus() = empty_unlock;
        *getBikeLockStatus() = bikeUnlock;
        *getBeforeLeafSw() = leafsw_off;
        // setPowerFlag(true);
        // setLockFlag(false);
        Printf("initial bike empty and unlocking\r\n");
    }
    send_uttec_response_msg(0);
}

uint32_t task_uttec(void)
{
    uint32_t m_uttec_time = 100;

    static uint32_t ulUttecCount = 0;
    static bool toggle = true;

    setAddress(); //2022.07.18, set address

    if(isEndOfOnTime()){
        stopBike();
    }   

    if(powerFlag){
        onPower();
        powerFlag = 0;
        setRackStatusByLock(lockFlag, allFlag);
    }
    else{
        if(lockFlag){
            lockBike();
        }
        else{
            unlockBike();
        }
    }    

    if(!(ulUttecCount++%10)){
    // if(!(ulUttecCount++%1)){
        static bool first = true;
        static uint32_t ulCount = 0;
        ulCount++;

        if(first){
            actFirst();
            first = false;
        }

        short value = (short)Mcu_voltageGet();
        if(value < 0) value = 0;
        Printf("$%d -> ",  ulCount);
        Printf("{\"area\":%d, \"rack\":%d} \r\n",getAreaId(), getMyId());

        if(getAdcChannel() == 0){
            diaper.battery = averageBattery((value*1024)/2850); // 3.7V
        }
        else{
            diaper.sensor =  averageDiaper((value*1024)/2850); // 7.4V
        }    

        toggle = !toggle;
        if(toggle){
            setAdcChannel(DIAPER_CH);
        }
        else{
            // setAdcChannel(7);
            setAdcChannel(BATTERY_CH);
        }
        diaper.counter = ulUttecCount;
        diaper.status = ulUttecCount;    

        if(isChangedSwitch()){
            if(isLeafSw())
                Printf("Leaf Sw on\r\n");
            else
                Printf("Leaf Sw off\r\n");
            // NVIC_SystemReset();    // 2022.10.17 for Low power application
        }
        reSend();
    }
    return m_uttec_time;    //resetting task period:
}

void send_echo_response_msg(uint32_t delay)
{
    payload_uttec_t payload; 
    send_uplink_msg(MSG_ID_ECHO_RESPONSE_MSG,(uint8_t *)&payload);
}

void send_uttec_response_msg(uint32_t delay)
{
    payload_uttec_t payload; 
    payload.counter = diaper.counter;
    payload.sensor = diaper.sensor;
    payload.battery = diaper.battery;
    payload.status = *getRackStatus();
    payload.myId = getMyId();
    payload.test1 = getAreaId(); //2022.10.07 change
    payload.solenoid = diaper.status;
    
    /* Send message. */
    Printf("~~~~~~~~~~ send_uplink_msg\r\n");
    send_uplink_msg(MSG_ID_UTTEC_COMMAND_MSG,(uint8_t *)&payload);
}

void set_periodic_msg_period(uint32_t new_period_ms)
{
    if ((new_period_ms >= PERIODIC_MSG_PERIOD_SET_MIN_VAL_MS) &&
            (new_period_ms <= PERIODIC_MSG_PERIOD_SET_MAX_VAL_MS))
    {
        m_period_ms = new_period_ms;

        /* Reschedule task to apply new period value. */
        App_Scheduler_addTask_execTime(task_send_periodic_msg,
                                       APP_SCHEDULER_SCHEDULE_ASAP,
                                       PERIODIC_WORK_EXECUTION_TIME_US);
    }
}

typedef struct __attribute__((packed))
{
    uint8_t cmd;
    uint32_t period;
    uint32_t address;
    uint8_t control;
} temp_t;

app_lib_data_receive_res_e uttec_broadcast_data_received_cb(
        const shared_data_item_t * item,
        const app_lib_data_received_t * data)
{
    Printf("+++++++++++++++++ uttec_broadcast_data_received_cb ++++++++++++++++\r\n");

    temp_t ctr = *((temp_t *)data->bytes);
    allFlag = false;
    if(getAreaId() != ctr.period){
        Printf("------------------ Area Error\r\n");
        return APP_LIB_DATA_RECEIVE_RES_HANDLED;
    }

    if((ctr.cmd == MSG_ID_UTTEC_COMMAND_MSG)&&(ctr.address == 999)){
        if(ctr.control == 3){
            Printf("33333333333333 rack all lock\r\n");
            powerFlag = true;
            lockFlag = true;
            allFlag = true;
        }
        else if(ctr.control == 4){
            Printf("44444444444444 rack all unlock\r\n");
            powerFlag = true;
            lockFlag = false;
            allFlag = true;
        }
        else{
            Printf("========= error control: %d\r\n", ctr.control);
        }        
    }
    else if(getMyId() == ctr.address){
        Printf("id: %d\r\n", ctr.cmd);
        Printf("address: %d\r\n", ctr.address);
        Printf("control: %d\r\n", ctr.control);
        allFlag = false;
        if(ctr.control == 1){
            Printf("rack lock\r\n");
            powerFlag = true;
            lockFlag = true;
        }
        else if(ctr.control == 2){
            // setUnLockDealyTimeAtUnlock(200);
            Printf("rack unlock\r\n");
            powerFlag = true;
            lockFlag = false;
        }
        else{
            Printf("error control\r\n");
        }
    }
    else{
        Printf("not my address: %d\r\n", ctr.address);
    }

    return APP_LIB_DATA_RECEIVE_RES_HANDLED;
}

app_lib_data_receive_res_e unicast_broadcast_data_received_cb(
        const shared_data_item_t * item,
        const app_lib_data_received_t * data)
{
    Printf(" ------- uttec_unicast_data_received_cb ------- \r\n");
    temp_t ctr = *((temp_t *)data->bytes);
    Printf("cmd: %d\r\n", ctr.cmd);
    Printf("area: %d\r\n", ctr.period);
    Printf("address: %d\r\n", ctr.address);
    Printf("control: %d\r\n", ctr.control);
    allFlag = false;

    if(getAreaId() != ctr.period){
        Printf("------------------ Area Error\r\n");
        return APP_LIB_DATA_RECEIVE_RES_HANDLED;
    }

    switch (ctr.cmd)
    {
        case MSG_ID_UTTEC_COMMAND_MSG:
            Printf("------ MSG_ID_UTTEC_COMMAND_MSG:\r\n");
            if(getMyId() == ctr.address){
                if(ctr.control == 1){
                    Printf("rack lock\r\n");
                    powerFlag = true;
                    lockFlag = true;
                }
                else if(ctr.control == 2){
                    // setUnLockDealyTimeAtUnlock(200);
                    Printf("rack unlock\r\n");
                    powerFlag = true;
                    lockFlag = false;
                }
                else{
                    Printf("error control\r\n");
                }
            }
            break;

        default:    /* Unknown message ID : do nothing. */
            Printf("Unknown message ID : %d\r\n", ctr.cmd);
            break;

    }
    return APP_LIB_DATA_RECEIVE_RES_HANDLED;
}

void setPowerFlag(bool flag){
    powerFlag = flag;
}
void setLockFlag(bool flag){
    lockFlag = flag;
}




