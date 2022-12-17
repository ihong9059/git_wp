/* The module name to be printed each lines. */
#define DEBUG_LOG_MODULE_NAME "DBUG APP"
/* The maximum log level printed for this module. */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"
#include "uttec.h"
#include "persistent.h"
#include "Uart.h"

flash_t myFlash = {0,};

bike_t myBike = {0,};

bikeLockStatus_t bikeLockStatus = bikeUnlock;
rackStatus_t rackStatus = empty_unlock;
leafsw_t beforeLeafSw = leafsw_off;

leafsw_t* getBeforeLeafSw(void){
    return &beforeLeafSw;
}

bikeLockStatus_t* getBikeLockStatus(void){
    return &bikeLockStatus;
}

rackStatus_t* getRackStatus(void){
    return &rackStatus;
}

void dipalyRackStatus(void){
    switch(rackStatus){
        case empty_unlock: 
            Printf("empty_unlock.\r\n"); break;
        case empty_lock: 
            Printf("empty_lock?.\r\n"); break;
        case occupy_unlock: 
            Printf("occupy_unlock, wait Locking\r\n"); break;
        case occupy_lock: 
            Printf("occupy_lock, pay......\r\n"); break;
    }
}

void initUttec(void)
{
    nrf_gpio_cfg_output(OP_LED);
    nrf_gpio_cfg_output(MSG_LED);
    nrf_gpio_cfg_output(LED_A);
    nrf_gpio_pin_clear(LED_A);    //LED_A ON

    // nrf_gpio_cfg_output(SOLENOID0);
    // nrf_gpio_cfg_output(SOLENOID1);
    nrf_gpio_cfg_output(POWER);

    nrf_gpio_cfg_input(LEAF_SW, NRF_GPIO_PIN_PULLUP);

    nrf_gpio_cfg_input(SOLENOID0, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SOLENOID1, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SOLENOID00, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SOLENOID11, NRF_GPIO_PIN_NOPULL);

    nrf_gpio_pin_clear(SOLENOID0);
    nrf_gpio_pin_clear(SOLENOID1);
    nrf_gpio_pin_clear(POWER); // power off
    Printf("+++++++++++++++ This is initUttec\r\n");
}

void setLED_A(bool on_off){
    if(on_off){
        nrf_gpio_pin_set(LED_A);
    }
    else{
        nrf_gpio_pin_clear(LED_A);
    }
}

bool isLeafSw(void){
    bool result = leafsw_on;
    if(nrf_gpio_pin_read(LEAF_SW)){//leaf switch off     
        result = leafsw_off;
    }
    return result;
}

void setOpLed(bool state){
    if(state){
        nrf_gpio_pin_clear(OP_LED);
    }else{
        nrf_gpio_pin_set(OP_LED);
    }
}
void toggleOpLed(void){
    nrf_gpio_pin_toggle(OP_LED);
}

void setMsgLed(bool state){
    if(state){
        nrf_gpio_pin_clear(MSG_LED);
    }else{
        nrf_gpio_pin_set(MSG_LED);
    }
}
void toggleMsgLed(void){
    nrf_gpio_pin_toggle(MSG_LED);
}

void onPower(){
    nrf_gpio_cfg_output(SOLENOID0);
    nrf_gpio_cfg_output(SOLENOID1);
    nrf_gpio_pin_set(POWER);
    myBike.onTime = BikeOnTime;
    myBike.stopFlag = true;
}
void offPower(){
    nrf_gpio_cfg_input(SOLENOID0, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SOLENOID1, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_pin_clear(POWER);
}

void parseAddress(char* pAddr){
    char addr[2][5] = {0,};
    int array = 0;
    int count = 0;
    char* pTemp = pAddr;
    for(int i = 0; i < 10; i++){
        char ch = *pTemp;
        if(((ch >= '0')&&(ch <= '9'))||(ch == ',')||(ch == ' ')||(ch == 0)) pTemp++;
        else{
            Printf("Error at data: %d\r\n", i);
            return;
        } 
    }

    for(int i = 0; i < 10; i++){
        if(*pAddr == ','){
            array++; count = 0;
            pAddr++;
        }
        else{
            addr[array][count++] = *pAddr++;
        }
    }
    
    myFlash.flag = 1;
    myFlash.myId = atoi(addr[1]);
    if(myFlash.myId >= 500){
        Printf("MyId Error: %d\r\n", myFlash.myId);
        return;
    } 

    myFlash.areaId = atoi(addr[0]);
    Printf("{\"area\":%d, \"rack\":%d\r\n", myFlash.areaId, myFlash.myId);
    // Printf("\r\n +++++++++++ Area: %d, MyId: %d\r\n", myFlash.areaId, myFlash.myId);
    myFlash.macId = NRF_FICR->DEVICEID[0]&0x7fffffff;
    Mcu_Persistent_write((uint8_t *)&myFlash, 0, sizeof(flash_t));
}

bool isFirst(void)
{

    Mcu_Persistent_read((uint8_t *)&myFlash, 0, sizeof(flash_t));
    
    if(myFlash.macId == (NRF_FICR->DEVICEID[0]&0x7fffffff)){
        Printf("++++++++++++ pass\r\n");
    }
    else{
        myFlash.flag = 1;
        myFlash.myId = 1;
        myFlash.areaId = 1;
        myFlash.macId = NRF_FICR->DEVICEID[0]&0x7fffffff;
        Mcu_Persistent_write((uint8_t *)&myFlash, 0, sizeof(flash_t));
        Printf("++++++++++++ First\r\n");
    }
    return true;
}

void displayDeviceId(void)
{
    Printf("DEVICEID0: %08X\r\n", NRF_FICR->DEVICEID[0]);
    Printf("DEVICEID1: %08X\r\n", NRF_FICR->DEVICEID[1]);
    Printf("DEVICEID0: %ld\r\n", NRF_FICR->DEVICEID[0]&0x7fffffff);
    Printf("persistent size: %d\r\n", Mcu_Persistent_getMaxSize());
    Mcu_Persistent_read((uint8_t *)&myFlash, 0, sizeof(flash_t));
    Printf("flag: %d, myId: %d, mac: %ld\r\n", 
        myFlash.flag, myFlash.myId, myFlash.macId&0x7fffffff);
}

uint16_t getMyId(void)
{
    return myFlash.myId;
}

uint16_t getAreaId(void)
{
    return myFlash.areaId;
}


void setRackStatusByLock(bool lock, bool allFlag){
    Printf("--------- setRackStatusByLock\r\n");
    if(lock){
        setLED_A(false);
        switch(rackStatus){
            case empty_unlock:
                rackStatus = empty_lock; 
            break;
            case empty_lock:
                rackStatus = empty_lock; 
            break;
            case occupy_unlock:
                rackStatus = occupy_lock; // bike lock
            break;
            case occupy_lock:
                rackStatus = occupy_lock; 
            break;
        }
    }
    else{
        switch(rackStatus){
            case empty_unlock:// check status
                rackStatus = empty_unlock; 
                setLED_A(false);
            break;
            case empty_lock:// check status
                rackStatus = empty_unlock; 
                setLED_A(false);
            break;
            case occupy_unlock:
                rackStatus = occupy_unlock; 
                setLED_A(false);
            break;
            case occupy_lock:
                rackStatus = occupy_unlock;// bike unlock 
                setLED_A(true); //2022.10.06 LED display
            break;
        }
        // setUnlockDelayTime(); //set Unlock delay time
    }
    if(!allFlag)
        send_uttec_response_msg(0);
}

void lockBike(void)
{    
    nrf_gpio_pin_set(SOLENOID0);
    nrf_gpio_pin_clear(SOLENOID1);
}

void unlockBike(void)
{
    nrf_gpio_pin_clear(SOLENOID0);
    nrf_gpio_pin_set(SOLENOID1);
}

void stopBike(void)
{
    offPower();
    if(myBike.stopFlag){
        Printf("stopBike\r\n");
    } 
    myBike.stopFlag = false;
    nrf_gpio_pin_clear(SOLENOID0);
    nrf_gpio_pin_clear(SOLENOID1);
}

bool isEndOfOnTime(void)
{
    // checkBike();
    if(myBike.onTime){
        myBike.onTime--;
        return false;
    } 
    else{
        return true;
    }
}

bike_t* getMyBike(void)
{
    return &myBike;
}

flash_t* getFlash(void){
    return &myFlash;
}

// static uint16_t occupy_unlock_delay = 0; //2022.08.02 delete direct locking function
// void setUnLockDealyTimeAtUnlock(uint16_t delayTime){
//     occupy_unlock_delay = delayTime;
// }
uint8_t reSendTime = 0;
bool reSendFlag = false;

bool isChangedSwitch(void){
    static uint8_t ucDelayTime = 0;
    static bool bDelayFlag = false;

    bool result = false;
    bool nowStatus = isLeafSw();

    if(nowStatus != *getBeforeLeafSw()){
        if(!bDelayFlag){//first check
            bDelayFlag = true;
            ucDelayTime = 2;
            return result;
        }
        else{
            if(ucDelayTime){
                ucDelayTime--;        
                return result;
            } 
            else{
                bDelayFlag = false;
                //finally checked change
            }
        }

        *getBeforeLeafSw() = nowStatus;
        if(nowStatus == leafsw_on){// move from switch off to switch on(bike in)
            Printf("leaf switch on\r\n");
            switch(rackStatus){
                case empty_unlock:
                    // occupy_unlock_delay = UNLOCK_DELAY_TIMEOUT;
                    rackStatus = occupy_unlock; 
                break;
                case empty_lock:// check status
                    rackStatus = occupy_lock; 
                break;
                case occupy_unlock:
                    // occupy_unlock_delay = UNLOCK_DELAY_TIMEOUT;
                    rackStatus = occupy_unlock; 
                break;
                case occupy_lock:// check status
                    rackStatus = occupy_lock; 
                break;
            }
        }
        else{// move from switch on to switch off(bike out)
            setLED_A(false);
            Printf("leaf switch off\r\n");
            switch(rackStatus){
                case empty_unlock:// check status
                    rackStatus = empty_unlock; 
                break;
                case empty_lock:// check status
                    rackStatus = empty_lock; 
                break;
                case occupy_unlock:
                    rackStatus = empty_unlock; 
                break;
                case occupy_lock:
                    rackStatus = empty_lock;// bike out 
                break;
            }
        }
        send_uttec_response_msg(0);
        reSendFlag = true;
        reSendTime = 10;
        Printf("~~~~~~~~~ changed leaf switch\r\n");
        result = true;
    }

    // dipalyRackStatus();
    return result;
}

void reSend(void){
    if(reSendTime) reSendTime--;

    if(reSendFlag && !reSendTime){
        send_uttec_response_msg(0);
        reSendFlag = false;
    }
}
// void setUnlockDelayTime(void){
//     occupy_unlock_delay = UNLOCK_DELAY_TIMEOUT;
// }
