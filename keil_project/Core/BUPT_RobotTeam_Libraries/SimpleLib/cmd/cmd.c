#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
# include "serial.h"
# include "cmd.h"

uint8_t CMD_UART = 1;
static uint8_t cmd_count = 0;
static CMD_Ret_t CMDFunc_help(uint8_t argc, char **argv);

/* 命令列表 */
static CMD_Command_t commands[16] = {
/* USER CODE BEGIN COMMAND */
    {"help",
        "Show this help message.",
        0,
		{NULL},
        CMDFunc_help
    },
    {"djictrl",
        "ctrl motor using dji_boardv2_can module",
        4,
        {
            {"board_id", "ID of dji board"},
            {"motor_id", "ID of motor on that dji board"},
            {"mode", "1 for cur, 2 for vel, 3 for pos"},
            {"value", "cur, vel or pos"}
        },
        CMDFunc_djictrl
    },
	{"m2006setvel",
        "set vel of M2006 motor",
        2,
        {
            {"id", "ID of M2006"},
            {"vel", "vel(rpm)"},
        },
        CMDFunc_m2006setvel
    },
/* USER CODE END COMMAND */
};

static CMD_Ret_t CMDFunc_help(uint8_t argc, char **argv){
    for (int i = 0; i < cmd_count; i++) {
        Serial_TxDataf(CMD_UART, "- %s: %s\r\n", commands[i].name, commands[i].help);
        for (int j = 0; j < commands[i].argc; j++) {
            Serial_TxDataf(CMD_UART, "\t<%s>: %s\r\n", commands[i].args[j][0], commands[i].args[j][1]);
        }
    }
	return CMD_RET_OK;
}

static void CMD_InvalidParamNumReply(CMD_Command_t cmd){
    char reply[UART_TX_BUF_SIZE];
    uint8_t reply_len = 0;

    memcpy(reply, "[ERROR] Usage: ", 15);   
    reply_len += 15;

    memcpy(reply + reply_len, cmd.name, strlen(cmd.name)); 
    reply_len += strlen(cmd.name);

    reply[reply_len] = ' ';
    reply_len++;
    
    for (int i = 0; i < cmd.argc; i++) {
		reply[reply_len] = '<';
        reply_len++;
        memcpy(reply + reply_len, cmd.args[i][0], strlen(cmd.args[i][0]));
        reply_len += strlen(cmd.args[i][0]);
		reply[reply_len] = '>';
        reply[reply_len + 1] = ' ';
        reply_len += 2;
    }

    reply[reply_len] = '\r';
	reply[reply_len + 1] = '\n';
	reply_len += 2;
	
    Serial_TxData(CMD_UART, reply, reply_len);
}

static uint8_t CMD_Ack(CMD_Command_t* cmd, CMD_Ret_t ret){
    switch(ret){
        case CMD_RET_OK: Serial_TxDataf(CMD_UART, "[OK]\r\n"); break;
        case CMD_RET_FAIL: Serial_TxDataf(CMD_UART, "[ERROR] fail\r\n"); break;
        case CMD_RET_INVALID_PARAM_NUM: CMD_InvalidParamNumReply(*cmd); break;
        case CMD_RET_INVALID_PARAM: CMD_InvalidParamNumReply(*cmd); break;
        case CMD_RET_NO_SUCH_CMD: Serial_TxDataf(CMD_UART, "[ERROR] no such cmd\r\n"); break;
        case CMD_RET_NONE: break;
        default: Serial_TxDataf(CMD_UART, "[ERROR] unknown error:%d\r\n", ret); break;
    }
}

/**
 * @brief	初始化命令行
 * @param	index 串口索引
 * @note	需要在初始化阶段被调用
 */
void CMD_Init(uint8_t index){
    CMD_UART = index;
    Serial_Start(index);
    cmd_count = 0;
    for (int i = 0; commands[i].handler != NULL; i++) {
        cmd_count++;
    }
}

/**
 * @brief	解析并执行命令
 * @note	需要在RxDataCallback中调用
 */
void CMD_RecvCallback(uint8_t index, char *input, uint16_t len) {
    if(index != CMD_UART)return;

    // 拷贝输入字符串（因为strtok会修改原字符串）
    char buffer[UART_RX_BUF_SIZE + 1];
    strncpy(buffer, input, len);
    buffer[len] = '\0';

    // 第一阶段：按 \r\n 分割为多个命令行
    // （strtok 不可重入，必须先收集所有行，再逐行按空格分割）
    char *lines[MAX_ARGS];
    int line_count = 0;
    char *line = strtok(buffer, "\r\n");
    while (line != NULL && line_count < MAX_ARGS) {
        // 跳过行首空格
        while (*line == ' ') line++;
        if (*line != '\0') {
            lines[line_count++] = line;
        }
        line = strtok(NULL, "\r\n");
    }

    /* 如果没有有效命令行 */
    if (line_count == 0) {
        Serial_TxDataf(CMD_UART, "[ERROR] no cmd\r\n");
        return;
    }

    /* 第二阶段：逐条解析并执行每个命令 */
    for (int i = 0; i < line_count; i++) {
        // 拷贝当前行（内层 strtok 会覆盖外层状态，必须用独立缓冲区）
        char line_buf[UART_RX_BUF_SIZE + 1];
        strncpy(line_buf, lines[i], UART_RX_BUF_SIZE);
        line_buf[UART_RX_BUF_SIZE] = '\0';
        
        /* 按空格分割参数 */
        char *argv[MAX_ARGS];
        int argc = 0;
        char *token = strtok(line_buf, " ");
        while (token != NULL && argc < MAX_ARGS) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }

        if (argc == 0) {
            CMD_Ack(NULL, CMD_RET_NO_SUCH_CMD);
            continue;
        }

        /* 查找匹配的命令 */
        bool found = false;
        for (int j = 0; j < cmd_count; j++) {
            if (strcmp(argv[0], commands[j].name) == 0) {
                if(argc != commands[j].argc + 1){
                    CMD_Ack(&commands[j], CMD_RET_INVALID_PARAM_NUM);
                    return;
                }
                uint8_t ret = commands[j].handler(argc, argv);
                CMD_Ack(&commands[j], ret);
                found = true;
                break;
            }
        }

        if (!found) {
            CMD_Ack(NULL, CMD_RET_NO_SUCH_CMD);
        }
    }
}
