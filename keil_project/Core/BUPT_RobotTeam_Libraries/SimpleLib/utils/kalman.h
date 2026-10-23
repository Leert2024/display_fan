/*
 * @Author       : Zybz
 * @Date         : 2022-07-09 20:13:54
 * @LastEditors  : Zybz
 * @LastEditTime : 2022-07-09 20:34:31
 * @FilePath     : \robocon-r2_oslib\oslib\utils\kalman.h
 * @Description  :
 *
 * Copyright (c) 2022 by BUPT RobotTeam, All Rights Reserved.
 */
#ifndef __KALMAN_H
#define __KALMAN_H

/*Struct Area*/
typedef struct Kalman_s
{
  float A;       //一般为1
  float B;       //一般为0
  float Q;       //系统过程噪声的协方差
  float R;       //测量噪声的协方差
  float kal_out; //上一次卡尔曼的输出
  float cov;     //上一次卡尔曼的输出的协方差
} Kalman_s;

/*Function Area*/
float Kalman(Kalman_s *kal, float now_value, float uk);

#endif