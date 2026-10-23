# 命令行开发说明

## 描述
实现了一个串口命令行，用于与上位机通信。
默认使用串口1。
串口收发基于串口库，详见[serial.c](../../../Src/serial.c)和[serial.h](../../../Inc/serial.h)。

## 使用方法
- 必须确保在初始化阶段调用`CMD_Init`函数；
- 必须确保在`RxDataCallback`中调用`CMD_RecvCallback`函数：
    ```c
    uint8_t RxDataCallback(uint8_t index, uint8_t* buf, uint32_t len){
	    CMD_RecvCallback(index, (char*)buf, len);
        // ...
    }
    ```
- 命令必须遵循`<name> [value value ...]`的格式。`<name>`唯一确定一个命令，`<value>`为命令的参数。
- 解析到的**参数个数**和**参数值**将被传递给对应的**命令处理函数**。可在[cmd.c](cmd.c)的`CMD_Init`中通过`CMD_Register`函数注册命令处理函数，例如： 
    ```c
    CMD_Register("help", CMDFunc_help);
    ```
- 绑定函数必须遵循以下接口形式：
    ```c
    void CMDFunc_name(uint8_t argc, char **argv);
    ```
