#include "pid.h"

//control相关
Pid_Set SpeedPid;
Pid_Set DirectionPid;

/**
*	@brief 计算微分项
*   由PID_Get_Value()调用
*   @param[in]  微分系数,微分类型
*	@return 结果值
*/
float Differential_Value(Pid_Set *Pid, Differential_Type D_type, PIDType PID_Type)
{
	float Value = 0;
	if (D_type == NORMAL_D)
	{
		float Multiplier = 0;
		if (PID_Type == LOCATION)
			Multiplier = Pid->err - Pid->errback;
		else
			Multiplier = Pid->err + Pid->errbackLast - 2 * Pid->errback;
		Value = Pid->kd * Multiplier;
	}//传统微分
	else if (D_type == NOT_COMPLETE)
	{
		float  Aifa = 0;
		Aifa = Pid->aifa;
		if (Aifa > 1)
		{
			Aifa = 1;
		}
		if (Aifa < 0)
		{
			Aifa = 0;
		}
		Value = Pid->kd * (1 - Aifa) * (Pid->err - Pid->errback) + Aifa * Pid->errback;
	}//不完全微分
	else if (D_type == DIFF_HEAD)
	{
		Value = Pid->kd * (Pid->err_head - Pid->errback_head);
	}//微分先行
	return Value;
}
///////////////////////////////////////////////////////
/**
*	@brief 计算积分项
*   由PID_Get_Value()调用
*   @param[in] 积分系数,积分类型
*	@return 结果值
*/
float Inertial_Value(Pid_Set *Pid, Inertial_Type I_type, PIDType PID_Type)
{
	float Value = 0;

	if (I_type == NORMAL_I)
	{
		float Multiplier = 0;
		if (PID_Type == LOCATION)
			Multiplier = Pid->integral;
		else
			Multiplier = Pid->err;
		Value = Pid->ki * Multiplier;
		if (PID_Type == LOCATION)
			Pid->integral += Pid->err;
	}//传统积分
	else if (I_type == CHANGE_INER)
	{
		float adj_fi = 0;
		float abs_err = 0;
		abs_err = fabs(Pid->err);
		if (abs_err <= Pid->min_i)
		{
			adj_fi = 1;
		}
		else if (abs_err <= Pid->major_i)
		{
			adj_fi = (Pid->major_i - abs_err) / (Pid->major_i - Pid->min_i);
		}
		else
		{
			adj_fi = 0;
		}
		if (PID_Type == LOCATION)
			Value = Pid->ki * Pid->integral;
		else
			Value = Pid->ki * (adj_fi*Pid->err);
		Pid->integral += adj_fi * Pid->err;
	}//变积分
	else if (I_type == REGISTER_OVER)
	{
		Value = Pid->ki * Pid->integral;
		if (Pid->integral > Pid->threshold)
		{
			if (Pid->err < 0)
				Pid->integral += Pid->err;
		}
		else if (Pid->integral < (-Pid->threshold))
		{
			if (Pid->err > 0)
				Pid->integral += Pid->err;
		}
		else
		{
			Pid->integral += Pid->err;
		}
	}//抗积分饱和
	return  Value;
}
///////////////////////////////////////////////////////
/**
* @brief 误差界限值设定---经典PID必须设置
**/
void SetPID_ErrLimit(Pid_Set *Pid, float errlimit){
	Pid->errlimit = errlimit;
}
///////////////////////////////////////////////////////
/**
* @brief 设定目标值---经典PID必须设置
**/
void SetPID_AimValue(Pid_Set *Pid, float AimValue){
	Pid->AimValue = AimValue;
}
///////////////////////////////////////////////////////
/**
* @brief 设定PID的Kp/Ki/Kd参数---经典PID必须设置
**/
void SetPID_Kpid(Pid_Set *Pid, float kp, float ki, float kd){
	Pid->kp = kp;
	Pid->ki = ki;
	Pid->kd = kd;
}
///////////////////////////////////////////////////////
/**
* @brief 设定PID的Kp参数
**/
void SetPID_Kp(Pid_Set *Pid,float kp){
	Pid->kp = kp;
}
///////////////////////////////////////////////////////
/**
* @brief 设定PID的Ki参数
**/
void SetPID_Ki(Pid_Set *Pid, float ki){
	Pid->ki = ki;
}
///////////////////////////////////////////////////////
/**
* @brief 设定PID的Kd参数
**/
void SetPID_Kd(Pid_Set *Pid, float kd){
	Pid->kd = kd;
}
///////////////////////////////////////////////////////
/**
* @brief 不完全微分比例设定
**/
void SetPID_Aifa(Pid_Set *Pid, float aifa){
	Pid->aifa = aifa;
}
///////////////////////////////////////////////////////
/**
* @brief 变限积分参数设定
**/
void SetPID_LimitIntegral(Pid_Set *Pid, float min_i, float major_i){
	Pid->min_i = min_i;
	Pid->major_i = major_i;
}
///////////////////////////////////////////////////////
/**
* @brief 抗饱和积分阈值设定
**/
void SetPID_Threshold(Pid_Set *Pid, float threshold){
	Pid->threshold = threshold;
}
///////////////////////////////////////////////////////
/**
*	@brief 计算PID控制值
*
* @param[in] pid设定值,给定值,实际值,微分类型,积分类型
*	@return 结果值
*/
float PID_Get_Value(Pid_Set *Pid, float Give_Value, Differential_Type D_type, Inertial_Type I_type, PIDType PID_Type)
{
	float P_value = 0;
	float D_value = 0;
	float I_value = 0;
	float Value = 0;
	Pid->err = Give_Value - Pid->AimValue;
	I_value = Inertial_Value(Pid, I_type, PID_Type);//增量式积分分离
	/*if (Pid->err>Pid->errlimit)
	{
		Pid->err = Pid->errlimit;
	}
	if (Pid->err<-Pid->errlimit)
	{
		Pid->err = -Pid->errlimit;
	}*/
	Pid->err_head = Pid->err_head_first - Pid->AimValue;
	if (PID_Type == LOCATION)
		P_value = Pid->kp * Pid->err;
	else
		P_value = Pid->kp * Pid->err - Pid->kp * Pid->errback;
	D_value = Differential_Value(Pid, D_type, PID_Type);
	Value = P_value + D_value + I_value;

	Pid->errbackLast = Pid->errback;
	Pid->errback = Pid->err;

	Pid->errback_head = Pid->err_head;
	if (PID_Type == LOCATION)
		Pid->OutValue = Value;
	else
		Pid->OutValue += Value;
	return Pid->OutValue;
}
///////////////////////////////////////////////////////
/**
*	@brief 智能位置式PI控制器
*        速度控制
*   @param[in] pi结构体
*	@return 结果值
*/
float IntelligentPI(Pid_Set *Pid, Intelligent_PI *Intelligent, float Give_Value, float Actual_Value)
{
	float P_value = 0;
	float I_value = 0;
	Pid->err = Give_Value - Actual_Value;
	Pid->integral += Pid->err;
	Intelligent->kp[Intelligent->Index] = Pid->kp;
	Intelligent->ki[Intelligent->Index] = Pid->ki;
	if (fabs(Pid->err)<120)
	{
		if ((Pid->err*Pid->errback)<0)//偏差正向趋近于0
		{
			Intelligent->kp[Intelligent->Index + 1] =
				Intelligent->kp[Intelligent->Index] - Intelligent->kprate;

			if ((Pid->err*Pid->integral)>0)//加速消除偏差
				Intelligent->ki[Intelligent->Index + 1] =
				Intelligent->ki[Intelligent->Index] + Intelligent->kirate;
			else
				Intelligent->ki[Intelligent->Index + 1] =
				Intelligent->ki[Intelligent->Index] - Intelligent->kirate;
		}
		else if ((Pid->err*Pid->errback)>0)
		{
			Intelligent->kp[Intelligent->Index + 1] =
				Intelligent->kp[Intelligent->Index] + Intelligent->kprate;

			if ((Pid->err*Pid->integral)>0)
				Intelligent->ki[Intelligent->Index + 1] =
				Intelligent->ki[Intelligent->Index] + Intelligent->kirate;
			else
				Intelligent->ki[Intelligent->Index + 1] =
				Intelligent->ki[Intelligent->Index] - Intelligent->kirate;
		}
		else
		{
			Intelligent->kp[Intelligent->Index + 1] =
				Intelligent->kp[Intelligent->Index];
			Intelligent->ki[Intelligent->Index + 1] =
				Intelligent->ki[Intelligent->Index];
		}
		Pid->kp = Intelligent->kp[Intelligent->Index + 1];
		Pid->ki = Intelligent->ki[Intelligent->Index + 1];
	}
	else
	{
		Pid->kp = 2000 / fabs(Pid->err);
		Pid->integral = 0;
	}

	if (Pid->kp<Intelligent->kpmin)
		Pid->kp = Intelligent->kpmin;
	if (Pid->ki<Intelligent->kimin)
		Pid->ki = Intelligent->kimin;
	if (Pid->kp>Intelligent->kpmax)
		Pid->kp = Intelligent->kpmax;
	if (Pid->ki>Intelligent->kimax)
		Pid->ki = Intelligent->kimax;

	//	P_value=Pid->kp*(Pid->err- Pid->errback);
	//	I_value=Pid->ki*Pid->err;
	P_value = Pid->kp*Pid->err;
	I_value = Pid->ki*Pid->integral;
	Pid->OutValue = P_value + I_value;
	Pid->errback = Pid->err;
	Intelligent->Index++;
	if (Intelligent->Index>2)
		Intelligent->Index = 0;
	return Pid->OutValue;
}
///////////////////////////////////////////////////////
/**
* @brief 清除PID相关参数
**/
void InitPIDVaribles(Pid_Set *Pid)
{
	Pid->AimValue = 0.0f;

	Pid->kp = 0.0f;
	Pid->ki = 0.0f;
	Pid->kd = 0.0f;
	Pid->err = 0.0f;
	Pid->errback = 0.0f;
	Pid->errbackLast = 0.0f;
	Pid->integral = 0.0f;

	Pid->errlimit = 0.0f;

	Pid->aifa = 0.0f;
	Pid->err_head = 0.0f;
	Pid->errback_head = 0.0f;
	Pid->err_head_first = 0.0f;

	Pid->major_i = 0.0f;
	Pid->min_i = 0.0f;
	Pid->threshold = 0.0f;

	Pid->OutValue = 0.0f;
}
