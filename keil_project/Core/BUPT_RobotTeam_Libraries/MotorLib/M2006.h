# ifndef __M2006_H__
# define __M2006_H__

# include <stdint.h>

void M2006_Init(uint8_t id);
void M2006_Update(void);
uint32_t M2006_GetPos(uint8_t id);
void M2006_SetVel(uint8_t id, float vel);

# endif
