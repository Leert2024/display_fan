// XM340控制逻辑

# ifndef __XM430_H__
# define __XM430_H__

# include <stdint.h>

# define XM430_UART 3

void XM430_Init(uint8_t ID);
uint8_t XM430_ControlPosition(uint8_t ID, int32_t position);
uint8_t XM430_ControlAngle(uint8_t ID, float angle);

# endif
