# 功能层说明

## 目录说明
SimpleLib  
- 实现了CAN中断接收回调函数的自动管理
- 实现了串口命令行，详见[README](SimpleLib/cmd/README.md)
- 实现了常用外设的封装
- 实现了常用科学计算、PID算法等工具函数

ChassisLib  
- 底盘库，分为[底盘高级控制层](ChassisLib/chassis.h)与[底盘底层控制层](ChassisLib/omni_chassis.h)
- 具体见[底盘库说明](ChassisLib/README.md)

MotorLib  
- 实现了与大疆驱动板通信等相关功能的封装，直接控制电机

hardware  
- [Scancode.h](hardware/Scancode.h)提供了与上位机(ESP32)通信的接口
- [laser.h](hardware/laser.h)提供了处理来自激光测距模块的串口消息的接口
- [player.h](hardware/player.h)提供了控制播放器的接口
- [servos.h](hardware/servos.h)提供了控制机械臂与药箱的接口