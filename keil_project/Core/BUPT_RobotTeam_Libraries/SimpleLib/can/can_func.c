
/**
 * @file can_func.c
 * @author BUPT Robot Team 19th
 * @brief 对外接口，添加CAN中断回调函数
 * @version 0.1
 * @date 
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* 在此处添加包含CAN回调函数的头文件----------------------------------------------------*/
// #include "handle.h"
// #include "dji_board_can_v1.h"
// #include "base_chassis.h"

# include "can_func.h"
# include "can_utils.h"
# include "../BUPT_RobotTeam_Libraries/SimpleLib/utils/utils.h"
# include "serial.h"

#ifdef SLIB_USE_CAN // 使能CAN

// 以下函数引自dji_boardv2_can.h
extern void CAN_Callback_DJIv2_PosArrival(CAN_ConnMessage_s *data);
extern void CAN_Callback_DJIv2_PosCurStuck(CAN_ConnMessage_s *data);

/* 希望接收的ID，与回调函数ID对应 */
uint32_t CAN_AcceptID_Std[10] = {
	0x286, 0x287,
	0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208
};

/**
 * @brief 注册CAN回调函数
 */
void CAN_FuncInit(){
     CAN_CallbackAdd(0x286, &CAN_Callback_DJIv2_PosArrival);
     CAN_CallbackAdd(0x287, &CAN_Callback_DJIv2_PosCurStuck);
}

#endif // SLIB_USE_CAN