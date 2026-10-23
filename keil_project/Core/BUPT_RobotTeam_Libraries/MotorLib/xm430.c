# include "serial.h"
# include "xm430.h"
# include "crc_calc.h"

/**
 * @brief   初始化XM430电机(开启扭矩)
 */
void XM430_Init(uint8_t ID){
    Serial_Start(XM430_UART);
    uint8_t cmd[] = {
        0xFF, 0xFF, 0xFD, 0x00,	// 消息头
        0x00,					
        0x0B, 0x00,				// 长度
        0x83,
        0x40, 0x00,
        0x01, 0x00,
        0x01,
        0x01,
        0x02,
        0x01,
        0x00, 0x00				// CRC校验
    };
	cmd[4] = ID;
	uint16_t crc = update_crc(0, cmd, sizeof(cmd) - 2);
	cmd[16] = crc & 0xFF;
	cmd[17] = crc >> 8;

    Serial_TxData(XM430_UART, cmd, sizeof(cmd));
}

/**
 * @brief 控制单个XM430电机位置
 * @param ID 电机ID（如 0x01）
 * @param position 位置(0~4095)
 */
uint8_t XM430_ControlPosition(uint8_t ID, int32_t position){
    uint8_t command[14] = {
        0xFF, 0xFF, 0xFD, 0x00,
        0x01,
        0x09, 0x00,
        0x03,
        0x74, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    command[4] = ID;

    command[10] = (uint8_t)(position & 0xFF);
    command[11] = (uint8_t)((position >> 8) & 0xFF);
    command[12] = (uint8_t)((position >> 16) & 0xFF);
    command[13] = (uint8_t)((position >> 24) & 0xFF);

    uint16_t crc = update_crc(0, command, sizeof(command));

    uint8_t txBuf[16];  // 多两个字节存放 CRC
    memcpy(txBuf, command, sizeof(command));
    txBuf[14] = crc & 0xFF;
    txBuf[15] = crc >> 8;

    return !Serial_TxData(XM430_UART, txBuf, sizeof(txBuf));  // 发送完整命令
}

/**
 * @brief 控制单个XM430电机角度
 * @param ID 电机ID（如 0x01）
 * @param angle 角度
 */
uint8_t XM430_ControlAngle(uint8_t ID, float angle){
    uint32_t position = (uint32_t)(angle * 4095.0f / 360.0f);
	return XM430_ControlPosition(ID, position);
}
