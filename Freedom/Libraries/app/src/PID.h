#ifndef __PID__H__
#define __PID__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"
#include "stdio.h"

#ifdef _MSC_VER
#pragma pack(push,4)
#endif
typedef struct
{
	float AimValue;	//目标值

	float kp;
	float ki;
	float kd;
	float err;
	float errback;
	float errbackLast;
	float integral;

	float errlimit;//误差设定限制

	float aifa;   //不完全微分
	float err_head; //微分先行
	float errback_head;
	float err_head_first;

	float min_i;  //变积分
	float major_i;
	float threshold;   //抗积分饱和

	float OutValue;
} Pid_Set;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push,4)
#endif
typedef struct
{
	float kp[4];
	float ki[4];
	float kprate;
	float kirate;
	float kpmax;
	float kpmin;
	float kimax;
	float kimin;
	int  Index;
}Intelligent_PI;
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

typedef enum
{
	NORMAL_D,
	NOT_COMPLETE,     //不完全微分
	DIFF_HEAD        //微分先行
} Differential_Type;

typedef enum
{
	NORMAL_I,
	CHANGE_INER,      //变积分
	REGISTER_OVER    //抗饱和或遇限消弱积分
} Inertial_Type;

typedef enum
{
	LOCATION,
	INCREASE
}PIDType;

extern float Differential_Value(Pid_Set *Pid, Differential_Type D_type, PIDType PID_Type);
extern float Inertial_Value(Pid_Set *Pid, Inertial_Type I_type, PIDType PID_Type);
extern void SetPID_ErrLimit(Pid_Set *Pid, float errlimit);
extern void SetPID_AimValue(Pid_Set *Pid, float AimValue);
extern void SetPID_Kpid(Pid_Set *Pid, float kp, float ki, float kd);
extern void SetPID_Kp(Pid_Set *Pid, float kp);
extern void SetPID_Ki(Pid_Set *Pid, float ki);
extern void SetPID_Kd(Pid_Set *Pid, float kd);
extern void SetPID_Aifa(Pid_Set *Pid, float aifa);
extern void SetPID_LimitIntegral(Pid_Set *Pid, float min_i, float major_i);
extern void SetPID_Threshold(Pid_Set *Pid, float threshold);
extern float PID_Get_Value(Pid_Set *Pid, float Give_Value, Differential_Type D_type, Inertial_Type I_type, PIDType PID_Type);
extern float IntelligentPI(Pid_Set *Pid, Intelligent_PI *Intelligent, float Give_Value, float Actual_Value);
extern void InitPIDVaribles(Pid_Set *Pid);


//control相关
extern Pid_Set SpeedPid;
extern Pid_Set DirectionPid;

#ifdef __cplusplus
}
#endif

#endif
