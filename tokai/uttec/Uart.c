#include <stdlib.h>
#include "usart.h"

uint8_t rxData[256] = {0,};
uint8_t rxDepth = 0;
uint8_t ucHead = 0;
uint8_t ucTail = 0;

void receiveUart(uint8_t* pCh, size_t length)
{
    rxData[ucHead++]= *pCh;
}

uint8_t* getRx()
{
    uint8_t* ucpReturn = &rxData[ucTail];
    ucTail++;
    return ucpReturn;
}

uint8_t isRx()
{
    return abs(ucHead - ucTail);
}

void initUart(void)
{
    // Usart_init(115200, UART_FLOW_CONTROL_NONE);
    Usart_setEnabled(true);
    Usart_enableReceiver(receiveUart);
    Usart_receiverOn();
}

void disableUart(void)
{
    Usart_receiverOff();
    Usart_setEnabled(false);
}

void sendUart(const void * buffer, uint32_t length)
{
    Usart_sendBuffer(buffer, length);
}