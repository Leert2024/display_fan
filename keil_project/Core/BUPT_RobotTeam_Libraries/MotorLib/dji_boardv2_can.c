/**
 * @file dji_ctr.c
 * @author ActionLab & LJenius
 * @brief 大疆驱动板V2.0CAN通信协议相关
 * @version 0.1
 * @date 2021-07-10
 * @copyright Copyright (c) 2021
 */

# include "dji_boardv2_can.h"
# include "../SimpleLib/simplelib.h"
# include "serial.h"

# define MAX_SPEED 2	// 电机最大转速

int16_t spd[4]={10, 10, 10, 10};
MotorType Motor[8] = {0};
RM_MotorStatus_t RM_MotorStatus[2][4]; // 四个大疆电机的状态结构体

/**
 * @brief   发送CAN报文使能单个电机(通电)
 * @param   boardId 控制板ID(1至3)
 * @param   motorId 电机ID(1至4)
 */
void dji_MotorOn(uint32_t boardId, uint16_t motorId){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = MOTORON;
    msg.in[1] = 0;
    CAN_SendStdMsg(canId, &msg);
    RM_MotorStatus[boardId-1][motorId - 1].Status = MotorStatus_STOP;
}

/**
 * @brief   发送CAN报文失能单个电机(断电)
 * @param   boardId 控制板ID(1至3)
 * @param   motorId 电机ID(1至4)
 */
void dji_MotorOff(uint32_t boardId, uint16_t motorId){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = MOTOROFF;
    msg.in[1] = 0;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
    RM_MotorStatus[boardId-1][motorId - 1].Status = MotorStatus_OFF;
}

/**
 * @brief   发送CAN报文配置电机为速度环模式
 * @param   boardId 控制板ID(1至3)
 * @param   motorId 电机ID(1至4)
 */
void dji_VelCfg(uint32_t boardId, uint16_t motorId){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = VELCFG;
    msg.in[1] = 0;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
}

/**
 * @brief 限速的速度环
 * @param boardId
 * @param motorId 1至4
 * @note 仅支持RM3508 并且参数暂不支持自定义
 */
void dji_LimitVelCfg(uint32_t boardId, uint16_t motorId){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = VELCTRL;
    msg.in[1] = 0;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
}

/**
 * @brief 配置电机位置环
 * @param boardId
 * @param motorId 1至4
 * @param maxPosVel 最大位置环控制速度
 */
void dji_PosCfg(uint32_t boardId, uint16_t motorId, int maxPosVel){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = POSCFG;
    msg.in[1] = maxPosVel;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
}

/**
 * @brief 配置电机位置环，但不丢失当前编码器数据
 * @param boardId
 * @param motorId 1至4
 * @param maxPosVel 最大位置环控制速度
 * @note 仅支持RM3508电机
 */
void dji_NoInitPosCfg(uint32_t boardId, uint16_t motorId, int maxPosVel){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = NOINITPOSCFG;
    msg.in[1] = maxPosVel;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
}

/**
 * @brief 配置电机电流环
 * @param boardId
 * @param motorId 1至4
 */
void dji_CurCfg(uint32_t boardId, uint16_t motorId){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = CURCFG;
    msg.in[1] = 0;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
}

/**
 * @brief 速度环(使用前务必设置为对应模式)
 * @param boardId 目标驱动板ID
 * @param motorId 目标电机ID 1至4
 * @param vel 目标速度
 */
void dji_VelCtrl(uint32_t boardId, uint16_t motorId, int32_t vel){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = VELCTRL;
    msg.in[1] = vel;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
    RM_MotorStatus[boardId-1][motorId - 1].Status = vel ? MotorStatus_VEL : MotorStatus_STOP;
}

/**
 * @brief 位置环(使用前务必设置为对应模式)
 * @param boardId 目标驱动板ID
 * @param motorId 目标电机ID 1至4
 * @param pos 目标位置
 */
void dji_PosCtrl(uint32_t boardId, uint16_t motorId, int32_t pos){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = POSCTRLMUL;
    msg.in[1] = pos;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
    RM_MotorStatus[boardId-1][motorId - 1].Status = MotorStatus_POS;
}

/**
 * @brief 电流环(使用前务必设置为对应模式)
 * @param boardId 目标驱动板ID
 * @param motorId 目标电机ID 1至4
 * @param cur 目标电流
 */
void dji_CurCtrl(uint32_t boardId, uint16_t motorId, int32_t cur){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = CURCTRL;
    msg.in[1] = cur;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
    RM_MotorStatus[boardId-1][motorId - 1].Status = cur ? MotorStatus_CUR : MotorStatus_STOP;
}

static uint32_t encode(int32_t pos, uint16_t cur) {
    uint16_t cur_mask = cur & 0xFFF;
    uint32_t pos_bits = ((uint32_t)pos) & 0xFFFFF;  // 0xFFFFF = 2^20 - 1
    return (pos_bits << 12) | cur_mask;
}

void dji_PosCurCtrl(uint32_t boardId, uint16_t motorId, int32_t pos, uint16_t cur){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = POSCURCTRL;
    msg.in[1] = encode(pos, cur);
    CAN_SendStdMsg(canId, &msg);
    RM_MotorStatus[boardId-1][motorId - 1].Status = cur ? MotorStatus_POSCUR : MotorStatus_STOP;
}

/**
 * @brief 发报让大疆驱动板返回指定电机信息
 *  信息返回canId为DJI_BASE_ID+BOARDID
 * @param boardId 请求的驱动板ID
 * @param motorId 请求的电机ID 1至4
 */
void dji_ReadInfo(uint32_t boardId, uint16_t motorId){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = READINFO;
    msg.in[1] = 0;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);
}

/**
 * @brief 控制四个电机的速度环(使用前务必设置为对应控制模式)
 * @param boardId 目标驱动板id
 * @param vel 长度为4的int16_t数组，代表四个电机目标速度
 */
void dji_VelCtrlAll(uint32_t boardId, int16_t vel[4]){
    uint32_t canId = 0x204 + boardId;
    CAN_Message_u msg;
    for (int i = 0; i < 4; i++){
        msg.i16[i] = vel[i];
    }
    CAN_SendStdMsg(canId, &msg);
    for (int i = 0; i < 4; i++)
        RM_MotorStatus[boardId][i].Status = MotorStatus_VEL;
}

/**
 * @brief   控制四个电机位置环(使用前务必设置为对应控制模式)
 * @param   boardId 目标驱动板id
 * @param   pos 长度为4的int16_t数组，代表四个电机位置。
 * @note    为了弥补数据范围不足，大疆驱动板会将收到的位置信息*100后发送给电机
 */
void dji_PosCtrlAll(uint32_t boardId, int16_t pos[4]){
    uint32_t canId = 0x208 + boardId;
    CAN_Message_u msg;
    for (int i = 0; i < 4; i++){
        msg.i16[i] = pos[i];
    }
    CAN_SendStdMsg(canId, &msg);
    for (int i = 0; i < 4; i++)RM_MotorStatus[boardId][i].Status = MotorStatus_POS;
}

/**
 * @brief   Homing
 * @param   hcan 本机can端口
 * @param   boardId 大疆板子编号
 * @param   motorId 电机编号 1至4
 * @param   vel 找零速度(编码器线/ms) 建议在100以下,正负代表方向
 * @param   cur 找零电流(mA) 建议在0至3000之间
 */
void dji_Homing(uint32_t boardId, uint16_t motorId, int16_t vel, int16_t cur){
    uint32_t canId = DJI_BASE_ID + boardId;
    CAN_Message_u msg;
    msg.ui16[0] = motorId;
    msg.ui16[1] = HOMING;
    msg.i16[2] = vel;
    msg.i16[3] = cur;
    // OSLIB_CAN_SendMessage(&hcan, CAN_ID_STD, canId, &msg);
    CAN_SendStdMsg(canId, &msg);

    RM_MotorStatus[boardId-1][motorId - 1].Status = MotorStatus_HOMING;
}

/**
 * @brief 接受大疆驱动板发回的电机信息
 *  接受到的消息存储在了RM_MotorStatus中
 * CAN_ID=DJI_BASE_ID+BOARDID
 * @param data 收到的CAN消息
 */
void CAN_Callback_DJIv2_ReadInfo(CAN_ConnMessage_s *data){
    int boardid = data->id - DJI_BASE_ID;
    int motorId = (data->payload.ui8[0] >> 6) & 0xff;
    /*
    int vel = ((0x1f & data->payload.ui8[0]) << 8) | data->payload.ui8[1];
    vel |= (((data->payload.ui8[0] & 0x20))*0xffffff00);
    将14位补码变为32位补码，可读性差，换为移码。
    */
    int16_t vel = ((0x3f & data->payload.ui8[0]) << 8) | data->payload.ui8[1];
    vel -= (1 << 13);
    //移码写法(是不是很简单doge)
    int cur = data->payload.i16[1];
    float pos = data->payload.fl[1];
    Serial_TxDataQueuef(1, "[DEBUG] id:%d\tvel:%d\tcur:%d\tpos:%f\r\n", motorId, vel, cur, pos);
    RM_MotorStatus[boardid-1][motorId].pos = pos;
    RM_MotorStatus[boardid-1][motorId].rpm = vel;
    RM_MotorStatus[boardid-1][motorId].current = cur;
    //dji_VelCtrlAll(1,spd);
}

__weak void dji_PosArrive_Callback(uint8_t boardid, uint8_t motorid){
    Serial_TxDataQueuef(1, "[INFO] pos arrive: board: %d motor: %d", boardid, motorid);
}

__weak void dji_PosCurStuck_Callback(uint8_t boardid, uint8_t motorid){
    Serial_TxDataQueuef(1, "[INFO] poscur stuck: board: %d motor: %d", boardid, motorid);
}

/**
 * @brief 接受大疆驱动板发回的位置环到位信号
 * @param data
 */
void CAN_Callback_DJIv2_PosArrival(CAN_ConnMessage_s *data){
    int boardid = data->payload.i16[0];
    int motorid = data->payload.i16[1];
    dji_PosArrive_Callback(boardid, motorid);
    RM_MotorStatus[boardid-1][motorid - 1].Status = MotorStatus_STOP;
}

/**
 * @brief 接受大疆驱动板发回的位置环到位信号
 * @param data
 */
void CAN_Callback_DJIv2_PosCurStuck(CAN_ConnMessage_s *data){
    int boardid = data->payload.i16[0];
    int motorid = data->payload.i16[1];
    dji_PosCurStuck_Callback(boardid, motorid);
    RM_MotorStatus[boardid-1][motorid - 1].Status = MotorStatus_STOP;
}

/**
 * @brief 接受大疆驱动板发回的homing到位信号
 * @param data
 */
void CAN_Callback_DJIv2_HomingOver(CAN_ConnMessage_s *data){
    int boardid = data->payload.i16[0];
    int motorid = data->payload.i16[1];
    Serial_TxDataQueuef(1, "[DEBUG] motorid:%d\t in board:%d HomingOver\r\n", motorid, boardid);
    RM_MotorStatus[boardid-1][motorid - 1].Status = MotorStatus_STOP;
}
