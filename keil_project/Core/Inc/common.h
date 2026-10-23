# ifndef __COMMON_H__
# define __COMMON_H__

/* 以下为向main.c提供的接口 */

void User_Init(void);               // 用户初始化
void User_Tasks(void);              // 事件循环任务
void Error_Callback(void);          // 错误中断回调函数

# endif
