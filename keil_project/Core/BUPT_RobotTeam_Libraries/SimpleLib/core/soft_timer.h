# ifndef __SOFT_TIMER_H__
# define __SOFT_TIMER_H__

# include <stdint.h>

typedef struct{
    uint32_t start_tick;    // 起始时间戳
    uint32_t cycle;         // 定时时长（毫秒）
    uint16_t cycle_count;   // 循环次数
    uint8_t (*handler)(uint16_t cycle_count);
}SoftTimer_t;

void SoftTimer_Update(void);
uint8_t SoftTimer_Start(uint32_t cycle, uint8_t (*handler)(uint16_t cycle_count));

# endif
