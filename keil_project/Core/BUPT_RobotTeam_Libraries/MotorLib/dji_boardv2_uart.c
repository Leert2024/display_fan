# include "dji_boardv2_uart.h"
# include "serial.h"
# include <stdint.h>

# define DJI_BOARDV2_UART_INDEX 2

/**
 * @brief   初始化与大疆板通信的串口
 */
uint8_t dji_uart_Init(void){
    return Serial_Start(DJI_BOARDV2_UART_INDEX);
}

/**
 * @brief   通过串口控制大疆驱动板，设置电机工作方式
 * @param   motorID 电机ID(1至4)
 * @param   motorType 电机类型(1为RM3508，2为M2006)
 * @param   ctrlMode 控制模式(1为速度环，2为单圈角度环，3为多圈角度环，4为电流环)
 * @param   maxVel 最大速度(单位:rpm)
 * @note	本函数为阻塞的，约耗时0.1s，因此应在初始化阶段调用
 */
uint8_t dji_uart_SetMotor(uint8_t motorID, uint8_t motorType, uint8_t ctrlMode, uint32_t maxVel){
    uint8_t ret = Serial_TxDataf(DJI_BOARDV2_UART_INDEX, "setmotor %d %d %d %d", motorID, motorType, ctrlMode, maxVel);
    HAL_Delay(100);
	return ret;
}

/**
 * @brief   串口控制大疆驱动板设置电机角度(多圈角度环)
 * @param   motorID 电机ID(1至4)
 * @param   angle 目标角度(单位:度)
 * @param   isQueue 是否异步发送
 * @retval	成功返回1，失败返回0
 * @note    需保证电机当前工作模式为多圈角度环模式
 */
uint8_t dji_uart_MultiPosCtrl(uint8_t motorID, int32_t angle, uint8_t isQueue){
	uint8_t ret = 0;
	if(!isQueue){
		ret = Serial_TxDataf(DJI_BOARDV2_UART_INDEX, "multiposctrl %d %d", motorID, angle);
		HAL_Delay(1);
	}else{
		ret = Serial_TxDataQueuef(DJI_BOARDV2_UART_INDEX, "multiposctrl %d %d", motorID, angle);
	}
	return ret;
}

/**
 * @brief   串口控制大疆驱动板设置电机速度(速度环)
 * @param   motorID 电机ID(1至4)
 * @param   vel 目标速度(单位:rpm)
 * @param   isQueue 是否异步发送
 * @retval	成功返回1，失败返回0
 * @note    需保证电机当前工作模式为速度环模式
 */
uint8_t dji_uart_VelCtrl(uint8_t motorID, int32_t vel, uint8_t isQueue){
	uint8_t ret = 0;
    if(!isQueue){
		ret = Serial_TxDataf(DJI_BOARDV2_UART_INDEX, "velctrl %d %d\r\n", motorID, vel);
		HAL_Delay(1);
	}else{
		ret = Serial_TxDataQueuef(DJI_BOARDV2_UART_INDEX, "velctrl %d %d\r\n", motorID, vel);
	}
	return ret;
}
