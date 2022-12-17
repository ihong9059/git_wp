#ifndef __UTTEC_H_
#define __UTTEC_H_
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"
#include "board.h"

#define OP_LED 18 //for bike
// #define OP_LED 22 //for diaper
#define MSG_LED 23

#define LED_A 15

#define SOLENOID0 11
#define SOLENOID1 13
#define POWER 24 

#define SOLENOID00 8
#define SOLENOID11 22

#define LEAF_SW 20  

#define BikeOnTime 5
// #define UNLOCK_DELAY_TIMEOUT 60
#define UNLOCK_DELAY_TIMEOUT 5 //by tokai, when switch on, locking after 5 Sec

typedef struct{
    uint16_t flag;
    uint16_t myId;
    uint16_t areaId;
    uint32_t macId;
} flash_t;

typedef enum{
    LED_ON = true,
    LED_OFF = false,
} led_t;

typedef struct{
    bool stopFlag;
    uint8_t status;
    uint16_t bikeNo;
    uint16_t onTime;
} bike_t;

typedef struct{
    uint16_t rackNo;
    uint16_t battery;
    uint8_t status;    
    uint8_t solenoid;    
} status_t;

typedef enum{
    bikeUnlock = false,
    bikeLock = true,
} bikeLockStatus_t;

typedef enum{
    empty_unlock = 0,
    empty_lock,
    occupy_unlock,
    occupy_lock,
} rackStatus_t;

typedef enum{
    leafsw_on = true,
    leafsw_off = false,
} leafsw_t;

void setOpLed(bool);
void toggleOpLed(void);

void setMsgLed(bool);
void toggleMsgLed(void);

void initUttec(void);

bool isFirst(void);
void displayDeviceId(void);

uint16_t getMyId(void);
uint16_t getAreaId(void);

bike_t* getMyBike(void);

bool isEndOfOnTime(void);
void stopBike(void);
void lockBike(void);
void unlockBike(void);
void onPower(void);

bool isLeafSw(void);
flash_t* getFlash(void);
void dipalyRackStatus(void);
rackStatus_t* getRackStatus(void);
bikeLockStatus_t* getBikeLockStatus(void);
leafsw_t* getBeforeLeafSw(void);
bool isChangedSwitch(void);

bool* getPowerFlag(void);
bool* getLockFlag(void);
void setRackStatusByLock(bool lock, bool allFlag);

void send_uttec_response_msg(uint32_t delay);

void reSend(void);

void setLED_A(bool);
void parseAddress(char* pAddr);

#endif /* __UTTEC_H_ */
