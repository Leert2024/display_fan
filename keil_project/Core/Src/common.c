# include "main.h"
# include "common.h"		// 任务调度
# include "stm32f407xx.h"	// 寄存器控制
# include "serial.h"		// 串口

# include "MotorLib/dji_boardv2_can.h"
# include "MotorLib/M2006.h"

# include "SimpleLib/simplelib.h"	// CAN通信
# include "SimpleLib/utils/utils.h"	// 数学工具库
# include "SimpleLib/cmd/cmd.h"
# include "SimpleLib/core/soft_timer.h"

# include "hardware/fan/fan.h"

# define ACC_TIME 5000
uint32_t start_tick = 0;

void User_Init(void){
	HAL_Delay(3000);	// 等待电机上电
	
	/* 初始化通信 */
	Serial_TxDataf(2, "[INFO] Start init...\r\n");
	SimpleLib_Init(&hcan1);	// 初始化CAN收发管理
	CMD_Init(2);	// 初始化CLI(注册命令)
	
	/* 初始化各类装置 */
	Serial_TxDataf(2, "[INFO] Start hardwares...\r\n"); HAL_Delay(1);
	Fan_Init();
	Serial_TxDataf(2, "[INFO] Hardwares init ok.\r\n");	HAL_Delay(1);

	/* 初始化完成 */
	Serial_TxDataf(2, "[INFO] All init ok, event loop start.\r\n"); HAL_Delay(1);
	start_tick = HAL_GetTick();
}

void User_Tasks(void){
	SoftTimer_Update();
	SimpleLib_Run();
	CheckUartRxData();
	CheckUartTxData();
	M2006_Update();
	Fan_Update();
	
	static uint32_t last_tick = 0;
	if(HAL_GetTick() - start_tick <= ACC_TIME){
		if(HAL_GetTick() - last_tick < 10)return;
		last_tick = HAL_GetTick();
		M2006_SetVel(2, (float)(HAL_GetTick() - start_tick) * 360.0 / (float)ACC_TIME);
	}
}

uint8_t RxDataCallback(uint8_t index, uint8_t* buf, uint32_t len){
	CMD_RecvCallback(index, (char*)buf, len);
	return 1;
}

void Error_Callback(void){
	__disable_irq();
	Serial_TxData(1, (uint8_t*)"[FATAL] Reach error handler!\r\n", 30);
	while (1){
	}
}
