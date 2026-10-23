#include "utils.h"

/**
 * @brief   初始化PID_t结构体
 * @param   pid PID_t结构体的地址
 * @param   Kp P参数
 * @param   Ki I参数
 * @param   Kd D参数
 * @param   Kdd D²参数
 * @param   T 采样周期
 * @param   tau 微分项低通滤波器时间常数(值越大，滤波器输出越平滑，为0则不滤波)
 * @param   limMin 输出值下限
 * @param   limMax 输出值上限
 * @param   limMinInt 积分下限
 * @param   limMaxInt 积分上限
 * @retval  无
 */
void  PID_Init(PID_t* pid, float Kp, float Ki, float Kd, float Kdd, float T, float tau, float limMin, float limMax, float limMinInt, float limMaxInt){
	pid->integrator           = 0.0f;
	pid->prevError            = 0.0f;
	pid->differentiator       = 0.0f;
	pid->prevMeasurement      = 0.0f;
	pid->prevPrevMeasurement  = 0.0f;
	pid->differentiator2      = 0.0f;
	pid->out                  = 0.0f;
	pid->Kp                 = Kp;
	pid->Ki                 = Ki;
	pid->Kd                 = Kd;
	pid->Kdd                = Kdd;
	pid->tau                = tau;
	pid->T                  = T;
	pid->limMax             = limMax;
	pid->limMin             = limMin;
	pid->limMaxInt          = limMaxInt;
	pid->limMinInt          = limMinInt;
}

/**
 * @brief   获取PID算法输出值
 * @param   pid PID_t结构体的地址
 * @param   setpoint 预期值
 * @param   measurement 测量值
 * @retval  PID算法输出值
 */
float PID_Update(PID_t* pid, float setpoint, float measurement) {
    float error = measurement - setpoint;   // 误差 = 测量值 - 预期值

    /* 计算比例项 */
    float proportional = pid->Kp * error;   // 比例项 = Kp * 误差

	/* 计算积分项(梯形积分法) */
    pid->integrator += pid->Ki * pid->T * (error + pid->prevError) / 2.0f;

	/* 积分项限幅 */
    if (pid->integrator > pid->limMaxInt)pid->integrator = pid->limMaxInt;
    else if (pid->integrator < pid->limMinInt)pid->integrator = pid->limMinInt;

    /* 计算微分项(一阶低通滤波) */
    float alpha = pid->tau/(pid->T + pid->tau);
    float errDif = measurement - pid->prevMeasurement;
    pid->differentiator = pid->Kd*(errDif*(1.0f - alpha) + pid->differentiator*alpha);

    /* 计算二阶微分项(一阶低通滤波，微分在测量值上) */
    float errDif2 = measurement - 2.0f*pid->prevMeasurement + pid->prevPrevMeasurement;
    pid->differentiator2 = pid->Kdd*(errDif2*(1.0f - alpha) + pid->differentiator2*alpha);

    /* 计算输出值并限幅 */
    pid->out = proportional + pid->integrator + pid->differentiator + pid->differentiator2;
    if (pid->out > pid->limMax)pid->out = pid->limMax;
    else if (pid->out < pid->limMin)pid->out = pid->limMin;

    /* 保存误差值与本次测量值 */
    pid->prevError           = error;
    pid->prevPrevMeasurement = pid->prevMeasurement;
    pid->prevMeasurement     = measurement;

    return pid->out;    // 输出结果
}

float AngleLimit180(float angle){
  while (angle > 180)
  {
    angle -= 360;
  }
  while (angle <= -180)
  {
    angle += 360;
  }
  return angle;
}

float AngleLimitPI(float angle)
{
  while (angle > PI)
  {
    angle -= 2 * PI;
  }
  while (angle <= -PI)
  {
    angle += 2 * PI;
  }
  return angle;
}

/**
 * @brief 两角差值，限制在[-PI, PI]
 */
float AngleLimitDiff(float a, float b){
  float out = a - b;
  return AngleLimitPI(out);
}

/**
 * @brief   计算两点连线的方向角
 * @param   start_x 起点x坐标
 * @param   start_y 起点y坐标
 * @param   end_x 终点x坐标
 * @param   end_y 终点y坐标
 * @return  方向角
 */
float AngleBetweenPoints(float start_x, float start_y, float end_x, float end_y){
  float angle = 0;
  if (fabs(start_x - end_x) < 1e-4){
    if (fabs(start_y - end_y) < 1e-4)
      angle = 0;
    else if (start_y < end_y)
      angle = PI / 2;
    else //(start_y > end_y)
      angle = -PI / 2;
  }
  else{
    angle = atan2f(end_y - start_y, end_x - start_x);
  }
  return angle;
}

float LaserAngleBetweenPoints(float start_x, float start_y, float end_x, float end_y){
  float angle = 0;
  if (fabs(start_x - end_x) < 1e-4) // 浮点数不能直接比较大小
  {
    if (fabs(start_y - end_y) < 1e-4)
      angle = 0;
    else if (start_y > end_y)
      angle = PI / 2;
    else //(start_y < end_y)
      angle = -PI / 2;
  }
  else{
      angle= atan2f(start_y-end_y, end_x-start_x);
  }
  return angle;
}
