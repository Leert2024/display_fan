# include <stdlib.h>
# include "cmd_func.h"
# include "serial.h"
# include "common.h"
# include "MotorLib/dji_boardv2_can.h"
# include "MotorLib/M2006.h"

CMD_Ret_t CMDFunc_djictrl(uint8_t argc, char **argv){
	uint8_t board_id = atoi(argv[1]);
	uint8_t motor_id = atoi(argv[2]);
	uint8_t mode = atoi(argv[3]);
	int16_t value = atoi(argv[4]);
	CMD_Ret_t ret = CMD_RET_FAIL;
	
	switch(mode){
		case 1: dji_CurCtrl(board_id, motor_id, value); ret = CMD_RET_OK; break;
		case 2: dji_VelCtrl(board_id, motor_id, value); ret = CMD_RET_OK; break;
		case 3: dji_PosCtrl(board_id, motor_id, value); ret = CMD_RET_OK; break;
		default: break;
	}
	return ret;
}

CMD_Ret_t CMDFunc_m2006setvel(uint8_t argc, char **argv){
	uint8_t id = atoi(argv[1]);
	float vel = atof(argv[2]);
	M2006_SetVel(id, vel);
	return CMD_RET_OK;
}
