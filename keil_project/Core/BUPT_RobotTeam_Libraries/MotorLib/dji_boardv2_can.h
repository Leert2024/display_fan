#ifndef __DJI_BOARDV2_CAN_H__
#define __DJI_BOARDV2_CAN_H__

#include "stm32f4xx.h"
#include "main.h"
#include "../SimpleLib/can/can_utils.h"

#define DJI_BASE_ID (0x200)	//大疆驱动板接收ID基址

/* 电机种类 */
typedef enum{
 	RM_3508 = 1,
 	M_2006 = 2,
 	NONE = 3 //none表示没有接电机
}MotorType_TypeDef;

typedef enum{
 	MotorStatus_OFF = 0,
 	MotorStatus_POS,
	MotorStatus_VEL,
	MotorStatus_CUR,
	MotorStatus_POSCUR,
	MotorStatus_HOMING,
	MotorStatus_STOP
}RunningStatus;

/* Robomaster Motor 3508/2006 type structure definition */
typedef struct{
 	int32_t pos, posLast;
 	MotorType_TypeDef type;
 	int32_t vel; //电机返回的速度，单位: rpm
 	int16_t cur; //电机返回的电流值
 	int8_t temp; //电机的温度
}MotorType;

/* 配合东大驱动板使用的状态结构体 */
typedef struct RM_MotorStatus_t{
	float pos; // rad
	float rpm;
	float current;
	uint8_t Status;
} RM_MotorStatus_t;

extern RM_MotorStatus_t RM_MotorStatus[2][4];
extern MotorType Motor[8];

/* CAN消息种类 */
typedef enum{
	MOTORON = 1,	// 使能
	MOTOROFF,		// 失能
	VELCFG = 3,		// 配置为速度环模式
	POSCFG = 5,		// 配置为单圈角度环模式
	CURCFG,			// 配置为电流环模式
	VELCTRL = 7,	// 控制速度
	POSCTRL = 8,	// 控制单圈角度
	POSCTRLMUL = 9,
	CURCTRL = 10,	// 控制电流
	READINFO = 11,
	VELCTRLALL = 12,
	POSCTRLALL = 13,
	HOMING = 14,
	LIMITVELCFG = 15,
	NOINITPOSCFG = 16,
	POSCURCFG = 17,
	POSCURCTRL = 18,
}CAN_OPTION;

/* 以下为可用的接口 */

/* 电机使能与失能 */

void dji_MotorOn(uint32_t boradId, uint16_t motorId);
void dji_MotorOff(uint32_t boradId, uint16_t motorId);

/* 控制模式设置 */

void dji_VelCfg(uint32_t boradId, uint16_t motorId);
void dji_LimitVelCfg(uint32_t boradId, uint16_t motorId);
void dji_PosCfg(uint32_t boradId, uint16_t motorId, int maxPosVel);
void dji_NoInitPosCfg(uint32_t boradId, uint16_t motorId, int maxPosVel);
void dji_CurCfg(uint32_t boradId, uint16_t motorId);

/* 单个电机状态(速度/角度/电流)控制(须提前设置为相应控制模式) */

void dji_VelCtrl(uint32_t boradId, uint16_t motorId, int32_t vel);
void dji_PosCtrl(uint32_t boradId, uint16_t motorId, int32_t pos);
void dji_CurCtrl(uint32_t boradId, uint16_t motorId, int32_t cur);
void dji_PosCurCtrl(uint32_t boardId, uint16_t motorId, int32_t pos, uint16_t cur);

/* 全体电机状态(速度/角度)控制(须提前设置为相应控制模式) */

void dji_VelCtrlAll(uint32_t boradId, int16_t vel[4]);
void dji_PosCtrlAll(uint32_t boradId, int16_t pos[4]);

/* 杂项 */

void dji_ReadInfo(uint32_t boardId, uint16_t motorId);
void dji_Homing(uint32_t boardId, uint16_t motorId, int16_t vel, int16_t cur);

/* CAN接收中断回调函数 */

void CAN_Callback_DJIv2_ReadInfo(CAN_ConnMessage_s *data);
void CAN_Callback_DJIv2_PosArrival(CAN_ConnMessage_s *data);
void CAN_Callback_DJIv2_HomingOver(CAN_ConnMessage_s *data);

#endif
