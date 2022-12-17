#ifndef UTTEC_UART_H__
#define UTTEC_UART_H__

void initUart();
void sendUart(const void * buffer, uint32_t length);

uint8_t* getRx();
uint8_t isRx();
void disableUart(void);


#endif

