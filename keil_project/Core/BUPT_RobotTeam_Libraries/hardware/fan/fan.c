# include "fan.h"
# include "pattern.h"
# include "hardware/ws2812/ws2812.h"
# include "MotorLib/M2006.h"
# include "SimpleLib/core/soft_timer.h"

# define FAN_MOTOR_ID 2 // 风扇电机ID
# define ACC_TIME 5000	// 加速用时
# define PATTERN_VEL 10	// 图片旋转速度(rpm)
# define SWITCH_PERIOD  (60000 / PATTERN_VEL)	// 图片切换间隔
# define SWITCH_TIME    1000    // 图片切换耗时

uint32_t acc_start_tick = 0;	// 电机加速开始时间
static uint16_t offset = 0;
static uint32_t switch_start_tick = 0;	// 切换开始时刻
static uint8_t pattern_index = 0;	// 当前在播放哪一张图案
static uint8_t old_pattern_index = 0;	// 先前播放的图案

/**
 * @brief	加1取模
 */
static unsigned int mod_inc(unsigned int num, unsigned int mod){
	return (num + 1) % mod;
}

/**
 * @brief	减1取模
 */
static unsigned int mod_dec(unsigned int num, unsigned int mod){
	return (num + mod - 1) % mod;
}

/**
 * @brief	图案旋转定时器中断
 */
static uint8_t rotate(uint16_t cycle_count){
    offset = PATTERN_VEL > 0 ? mod_inc(offset, DIVIDE_NUM) : mod_dec(offset, DIVIDE_NUM);
    return 0;
}

/**
 * @brief	图案切换定时器中断
 */
static uint8_t switch_pattern(uint16_t cycle_count){
	if(HAL_GetTick() - acc_start_tick < ACC_TIME)return 0;	// 加速阶段不切换
    switch_start_tick = HAL_GetTick();
	old_pattern_index = pattern_index;
	pattern_index = mod_inc(pattern_index, PATTERN_NUM);
    return 0;
}

void Fan_Init(void){
	M2006_Init(FAN_MOTOR_ID);
	if(PATTERN_VEL != 0){
		uint32_t rotate_period = (uint32_t)(60000.0 / (float)(PATTERN_VEL * DIVIDE_NUM));
		SoftTimer_Start(rotate_period, rotate);
	}
	SoftTimer_Start(SWITCH_PERIOD, switch_pattern);
	acc_start_tick = HAL_GetTick();
}

static void frame_to_WS2812(uint16_t now_frame){
	/* 切换动画处理 */
	uint8_t display_pattern_index;
	if(pattern_index != old_pattern_index){
		uint16_t new_index;
		uint32_t switch_time = HAL_GetTick() - switch_start_tick;
		if(switch_time < SWITCH_TIME){
			new_index = (uint16_t)((float)switch_time * (DIVIDE_NUM / 2) / SWITCH_TIME);
		}else new_index = DIVIDE_NUM / 2;
		
		if(now_frame < DIVIDE_NUM / 2){
			if(PATTERN_VEL < 0){
				display_pattern_index = now_frame <= new_index ? pattern_index : old_pattern_index;
			}else{
				display_pattern_index = now_frame >= DIVIDE_NUM / 2 - new_index - 1 ? pattern_index : old_pattern_index;
			}
		}else{
			if(PATTERN_VEL < 0){
				display_pattern_index = now_frame <= new_index + DIVIDE_NUM / 2 ? pattern_index : old_pattern_index;
			}else{
				display_pattern_index = now_frame >= DIVIDE_NUM - new_index - 1 ? pattern_index : old_pattern_index;
			}
		}
	}else{
		display_pattern_index = pattern_index;
	}
	
	for(uint8_t i = 0; i < LED_NUM; i++){
        WS2812_SetSingle(
            LED_NUM - 1 - i, 
            pattern[display_pattern_index][now_frame][i][0], 
            pattern[display_pattern_index][now_frame][i][1], 
            pattern[display_pattern_index][now_frame][i][2]
        );
    }
    for(uint8_t i = 0; i < LED_NUM; i++){
        WS2812_SetSingle(
            19 + i, 
            pattern[display_pattern_index][(now_frame + DIVIDE_NUM / 2) % DIVIDE_NUM][i][0], 
            pattern[display_pattern_index][(now_frame + DIVIDE_NUM / 2) % DIVIDE_NUM][i][1], 
            pattern[display_pattern_index][(now_frame + DIVIDE_NUM / 2) % DIVIDE_NUM][i][2]
        );
    }
    WS2812_Refresh();
}

void Fan_Update(void){
	/* 电机加速（仅前ACC_TIME ms内有效） */
	static uint32_t last_tick = 0;
	if(HAL_GetTick() - acc_start_tick <= ACC_TIME){
		if(HAL_GetTick() - last_tick >= 10){
			last_tick = HAL_GetTick();
			M2006_SetVel(2, (float)(HAL_GetTick() - acc_start_tick) * 360.0 / (float)ACC_TIME);
		}
	}
	
	/* 灯带显示切换 */
    static uint16_t last_frame = 0;
    uint32_t pos = M2006_GetPos(FAN_MOTOR_ID);
    uint16_t now_frame = pos / (294912 / DIVIDE_NUM);
    if(now_frame == last_frame)return;
    frame_to_WS2812((now_frame + offset) % DIVIDE_NUM);
	last_frame = now_frame;
}
