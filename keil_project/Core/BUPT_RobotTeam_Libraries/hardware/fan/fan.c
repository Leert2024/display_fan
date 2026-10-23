# include "fan.h"
# include "pattern.h"
# include "hardware/ws2812/ws2812.h"
# include "MotorLib/M2006.h"
# include "SimpleLib/core/soft_timer.h"
# include <math.h>

# define FAN_MOTOR_ID 2 // 风扇电机ID
# define PATTERN_VEL 10	// 图片旋转速度(rpm)
# define SWITCH_PERIOD  4000    // 图片切换间隔
# define SWITCH_TIME    1000    // 图片切换耗时

static uint16_t offset = 0;
static uint32_t switch_start_tick = 0;	// 切换开始时刻
static uint8_t pattern_index = 0;	// 当前在播放哪一张图案

static uint8_t rotate(uint16_t cycle_count){
    offset = PATTERN_VEL > 0 ? (offset + 1) % DIVIDE_NUM : (offset + DIVIDE_NUM - 1) % DIVIDE_NUM;
    return 0;
}

static uint8_t switch_pattern(uint16_t cycle_count){
    switch_start_tick = HAL_GetTick();
	pattern_index ++;
    return 0;
}

void Fan_Init(void){
	M2006_Init(FAN_MOTOR_ID);

    uint32_t rotate_period = (uint32_t)(60000.0 / (float)(PATTERN_VEL * DIVIDE_NUM));
    SoftTimer_Start(rotate_period, rotate);
	SoftTimer_Start(SWITCH_PERIOD, switch_pattern);
}

static void frame_to_WS2812(uint16_t now_frame){
	// TODO: 图片切换
	
	for(uint8_t i = 0; i < LED_NUM; i++){
        WS2812_SetSingle(
            LED_NUM - 1 - i, 
            pattern[now_frame][i][0], 
            pattern[now_frame][i][1], 
            pattern[now_frame][i][2]
        );
    }
    for(uint8_t i = 0; i < LED_NUM; i++){
        WS2812_SetSingle(
            19 + i, 
            pattern[(now_frame + DIVIDE_NUM / 2) % DIVIDE_NUM][i][0], 
            pattern[(now_frame + DIVIDE_NUM / 2) % DIVIDE_NUM][i][1], 
            pattern[(now_frame + DIVIDE_NUM / 2) % DIVIDE_NUM][i][2]
        );
    }
    WS2812_Refresh();
}

void Fan_Update(void){
    static uint16_t last_frame = 0;

    uint32_t pos = M2006_GetPos(FAN_MOTOR_ID);
    uint16_t now_frame = pos / (294912 / DIVIDE_NUM);
    if(now_frame == last_frame)return;
    frame_to_WS2812((now_frame + offset) % DIVIDE_NUM);
	last_frame = now_frame;
}
