#ifndef __CMD_FUNC_H__
#define __CMD_FUNC_H__

# include <stdint.h>

typedef enum{
    CMD_RET_OK,                 // 成功返回值
    CMD_RET_FAIL,               // 失败返回值
    CMD_RET_INVALID_PARAM_NUM,  // 参数数量非法
    CMD_RET_INVALID_PARAM,      // 参数非法
    CMD_RET_NO_SUCH_CMD,        // 不存在该命令
    CMD_RET_NONE,               // 无需返回值
} CMD_Ret_t;

CMD_Ret_t CMDFunc_djictrl(uint8_t argc, char **argv);
CMD_Ret_t CMDFunc_m2006setvel(uint8_t argc, char **argv);

#endif
