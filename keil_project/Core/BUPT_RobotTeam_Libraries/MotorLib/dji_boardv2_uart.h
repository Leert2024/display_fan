# ifndef __DJI_BOARDV2_UART_H__
# define __DJI_BOARDV2_UART_H__

# include <stdint.h>

uint8_t dji_uart_Init(void);
uint8_t dji_uart_SetMotor(uint8_t motorID, uint8_t motorType, uint8_t ctrlMode, uint32_t maxVel);
uint8_t dji_uart_MultiPosCtrl(uint8_t motorID, int32_t angle, uint8_t isQueue);
uint8_t dji_uart_VelCtrl(uint8_t motorID, int32_t vel, uint8_t isQueue);

# endif
