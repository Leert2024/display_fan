# include <string.h>
# include "M2006.h"
# include "SimpleLib/can/can_utils.h"
# include "serial.h"
# include "SimpleLib/utils/utils.h"

int16_t currents[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int16_t vels[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int16_t target_vels[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint16_t poses[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint32_t final_poses[8] = {0, 0, 0, 0, 0, 0, 0, 0};
PID_t pids[8];

/**
 * @brief	编码器边界回绕处理
 * @param	now 当前帧的值
 * @param	old 上一帧的值
 * @return	正确的差分值
 */
static int16_t limit_4096(int16_t now, int16_t old){
	if(now - old > 4096)now -= 8192;
	if(old - now > 4096)now += 8192;
	return now - old;
}

/**
 * @brief	输出端角度转换为浮点数
 * @param	uint32_t_value 整数值(0~294911)
 * @return	浮点数(0.0~359.9988)
 */
static float final_get_float(uint32_t uint32_t_value){
	return (float)uint32_t_value * 360.0 / 294912.0;
}

static void M2006_CanCallback(CAN_ConnMessage_s *data){
	static uint16_t last_poses[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	if(data->id < 0x201 || data->id > 0x208)return;
	uint8_t id = data->id - 0x200;
	
	memcpy(last_poses, poses, sizeof(poses));
	poses[id - 1] = (data->payload.ui8[0] << 8) + data->payload.ui8[1];
	vels[id - 1] = (data->payload.ui8[2] << 8) + data->payload.ui8[3];
	
	/* 计算输出端角度 */
	for(uint8_t i = 0; i < 8; i++){
		if(last_poses[i] == 0)continue;	// 过滤掉初始时的大差分值
		final_poses[i] += limit_4096(poses[i], last_poses[i]);
		final_poses[i] = (final_poses[i] + 294912) % 294912;
	}
}

void M2006_Init(uint8_t id){
	if(id < 1 || id > 8)return;
	uint16_t can_id = id + 0x200;
	CAN_CallbackAdd(can_id, M2006_CanCallback);
	PID_Init(&pids[id - 1], -8.33, -0.22, 0, 0, 1, 0, -10000, 10000, -2000, 2000);
}

void M2006_Update(void){
	static uint32_t last_tick = 0;
	if(HAL_GetTick() - last_tick < 1)return;
	last_tick = HAL_GetTick();
	
	for(uint8_t i = 0; i < 8; i++){
		currents[i] = PID_Update(&pids[i], target_vels[i], vels[i]);
	}
	CAN_Message_u msg;
	for(uint8_t i = 0; i < 4; i++){
		msg.ui16[i] = __REV16((uint16_t)(int16_t)currents[i]);
	}
	CAN_SendStdMsg(0x200, &msg);
	memset(&msg, 0, sizeof(msg));          // 防止残留
	for(uint8_t i = 4; i < 8; i++){
		msg.ui16[i - 4] = __REV16((uint16_t)(int16_t)currents[i]);
	}
	CAN_SendStdMsg(0x1FF, &msg);
}

/**
 * @brief	获取M2006输出端角度
 * @param	id 电机ID(1~8)
 * @return	输出端角度编码值(0~294911)
 */
uint32_t M2006_GetPos(uint8_t id){
	if(id < 1 || id > 8)return 0;
	return final_poses[id - 1];
}

void M2006_SetVel(uint8_t id, float vel){
	if(id < 1 || id > 8)return;
	target_vels[id - 1] = (int16_t)(vel * 36.0);
}
