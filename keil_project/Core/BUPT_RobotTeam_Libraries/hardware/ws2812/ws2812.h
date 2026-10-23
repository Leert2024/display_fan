# ifndef __MY_WS2812_H
# define __MY_WS2812_H	 

# include <stdint.h>

void WS2812_SetBuf(uint16_t* content);
void WS2812_SetSingle(uint16_t num, uint8_t rv, uint8_t gv, uint8_t bv);
void WS2812_AllOff(void);
void WS2812_AllOn(void);
void WS2812_Init(void);
void WS2812_Refresh(void);

# endif
