# include "soft_timer.h"

# define MAX_TIMER_NUM 10

volatile SoftTimer_t timers[MAX_TIMER_NUM];
volatile uint8_t is_efftive[MAX_TIMER_NUM] = {0};

/**
 * @brief   定时器更新
 * @param   tick 当前时间戳（ms）
 * @note    本函数在SysTick_Handler中调用，用户事件循环中无需调用
 */
void SoftTimer_Update(void){
    for(uint8_t i = 0; i < MAX_TIMER_NUM; i++){
        if(!is_efftive[i])continue;
        SoftTimer_t* timer = &timers[i];
        if(HAL_GetTick() - timer->start_tick - timer->cycle_count * timer->cycle > timer->cycle){
            timer->cycle_count ++;
            if(timer->handler(timer->cycle_count))is_efftive[i] = 0;
        }
    }
}

/**
 * @brief   延时执行某函数
 * @param   cycle 延时周期
 * @param   handler 到时回调函数
 * @note    定时器触发时，handler将被调用，传入参数为周期数。若handler返回1，则定时器将被清除
 */
uint8_t SoftTimer_Start(uint32_t cycle, uint8_t (*handler)(uint16_t cycle_count)){
    uint8_t i = 0;
    for(i = 0; i < MAX_TIMER_NUM; i++){
        if(is_efftive[i])continue;
        is_efftive[i] = 1;
        timers[i].cycle = cycle;
        timers[i].cycle_count = 0;
        timers[i].handler = handler;
        timers[i].start_tick = HAL_GetTick();
        return 0;
    }
    return 1;
}
