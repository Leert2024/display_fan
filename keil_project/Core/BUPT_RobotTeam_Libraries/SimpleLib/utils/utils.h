#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../simplelib_cfg.h"
#ifdef SLIB_USE_UTILS
#include "math.h"
#include <stdint.h>

/**
 * @brief	宏数据处理函数
 * @note	切勿在传入带有副作用的参数.例如 *p++
 */
#define __LIMIT(value, max) \
	if (value > max)        \
		value = max;        \
	else if (value < -max)  \
	value = -max

#define __LIMIT_FROM_TO(value, min, max) \
	{                                    \
		if ((value) > (max))             \
			value = max;                 \
		if ((value) < (min))             \
			value = min;                 \
	}

/**
 * @brief 计算数组和，无需指定数组类型
 * @param result 需要填入double类型，然后在宏外部使用强制类型转换
 */
#define __SUM_OF_AR(AR, N, RESULT)   \
	{                                \
		double MACRO_SUM_TEMP = 0;   \
		for (int i = 0; i < N; i++)  \
		{                            \
			MACRO_SUM_TEMP += AR[i]; \
		}                            \
		RESULT = MACRO_SUM_TEMP;     \
	}

/**
 * @brief 计算数组平均数，无需指定数组类型
 * @param result 需要填入double类型
 */
#define __AVE_OF_AR(AR, N, RESULT)   \
	{                                \
		int MACRO_SUM_TEMP = 0;      \
		for (int i = 0; i < N; i++)  \
		{                            \
			MACRO_SUM_TEMP += AR[i]; \
		}                            \
		RESULT = MACRO_SUM_TEMP / N; \
	}

/**
 * @brief 获取数组中最大的元素
 * @param MAX 结果
 */
#define __MAX_OF_AR(AR, N, MAX)     \
	{                               \
		MAX = AR[0];                \
		for (int i = 0; i < N; i++) \
		{                           \
			if (AR[i] > MAX)        \
			{                       \
				MAX = AR[i];        \
			}                       \
		}                           \
	}

#define __MIN_OF_AR(AR, N, MIN)     \
	{                               \
		MIN = AR[0];                \
		for (int i = 0; i < N; i++) \
		{                           \
			if (AR[i] > MIN)        \
			{                       \
				MIN = AR[i];        \
			}                       \
		}                           \
	}

#define Min(A, B) ((A) <= (B) ? (A) : (B))
#define Max(A, B) ((A) >= (B) ? (A) : (B))
#define PI (3.1415926535898f)
#define SL_OK 0
#define SL_ERROR 1
//角度制转化为弧度制
#define __ANGLE2RAD(x) (((x)*1.0) / 180.0f * PI)
//弧度制转换为角度制
#define __RAD2ANGLE(x) (((x)*1.0) / PI * 180.0f)

/* PID句柄 */
typedef struct{
	float Kp;                   // P参数
	float Ki;                   // I参数
	float Kd;                   // D参数
	float Kdd;				    // D²参数
	float limMin;               // 输出值下限
	float limMax;               // 输出值上限
	float limMinInt;            // 积分(integrator)下限
	float limMaxInt;            // 积分(integrator)上限
	float T;                    // 采样周期
	float tau;                  // 微分项低通滤波器时间常数(值越大，滤波器输出越平滑)
	float integrator;           // 误差累积值
	float prevError;            // 当前误差与前一次误差之差
	float differentiator;       // 一阶微分滤波值
	float prevMeasurement;	    // 前一次测量值
	float prevPrevMeasurement;	// 前两次测量值（二阶微分用）
	float differentiator2;	    // 二阶微分滤波值
	float out;                  // 输出
}PID_t;

void PID_Init(PID_t* pid, float Kp, float Ki, float Kd, float Kdd, float T, float tau, float limMin, float limMax, float limMinInt, float limMaxInt);
float PID_Update(PID_t* pid, float setpoint, float measurement);

typedef struct Point2D_s{
	float x;
	float y;
} Point2D_s; // 二维空间点

typedef struct Point3D_s
{
	float x;
	float y;
	float z;
} Point3D_s;



float AngleLimit180(float angle);
float AngleLimitPI(float angle);
float AngleLimitDiff(float a, float b);
float AngleBetweenPoints(float start_x, float start_y, float end_x, float end_y);
float LaserAngleBetweenPoints(float start_x, float start_y, float end_x, float end_y);
#endif // SLIB_USE_UTILS

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_H */