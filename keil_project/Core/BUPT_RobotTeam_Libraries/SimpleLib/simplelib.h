#ifndef __SIMPLELIB_H
#define __SIMPLELIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "simplelib_cfg.h"

#ifdef SLIB_USE_CMD
#include "usart.h"
#endif

#ifdef SLIB_USE_CAN
#include "../BUPT_RobotTeam_Libraries/SimpleLib/can/can_utils.h"
#include "../BUPT_RobotTeam_Libraries/SimpleLib/can/can_func.h"
#endif

#ifdef SLIB_USE_NRF
#include "nrf24l01.h"
#endif

void SimpleLib_Init(CAN_HandleTypeDef *hcan);
void SimpleLib_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLELIB_H */
