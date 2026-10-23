/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		simplelib.c
 * Author:			ZeroVoid
 * Description:	None
 * Bug:				None
 * Version:			0.2.1
 * Data:			2019/09/19 Thu 19:35
 * Todo:			None
 *******************************************************************************/

#include "simplelib.h"
#include "gpio.h"

/**
 * @brief	初始化配置
 * @param	hcan CAN通信句柄
 * @return	无
 */
void SimpleLib_Init(CAN_HandleTypeDef *hcan){
#ifdef SLIB_USE_UART_DMA

#endif // SLIB_USE_UART_DMA

#ifdef SLIB_USE_CMD
   
#endif // SLIB_USE_CMD

#ifdef SLIB_USE_CAN
    if (hcan != NULL)
    {
        CAN_Init(hcan);
        
        CAN_FuncInit();
    } 

#endif // SLIB_USE_CAN

}

/**
 * @brief	CAN通信相关前台任务函数
 * @note	本函数应置于事件循环中
 */
void SimpleLib_Run(void){
# ifdef SLIB_USE_CMD
    
# endif
# ifdef SLIB_USE_CAN
    if (CAN_ExeCallback_Flag){
        CAN_CallbackExe();
        CAN_ExeCallback_Flag = 0;
    }
# endif
}
