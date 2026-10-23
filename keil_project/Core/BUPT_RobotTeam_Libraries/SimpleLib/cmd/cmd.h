#ifndef __CMD_H__
#define __CMD_H__

#define MAX_ARGS 10
#define MAX_CMD_NAME 32

# include <stdint.h>
# include "cmd_func.h"

typedef struct {
    char name[MAX_CMD_NAME];        // 命令名称
    const char* help;               // 命令帮助
    uint8_t argc;                   // 命令参数数量
    const char* args[MAX_ARGS][2];  // 命令参数数组({{参数名, 参数描述}})
    CMD_Ret_t (*handler)(uint8_t argc, char **argv);  // 处理函数
} CMD_Command_t;

void CMD_Init(uint8_t index);
void CMD_RecvCallback(uint8_t index, char *input, uint16_t len);

#endif
