/**
*     COPYRIGHT NOTICE
*     All rights reserved.
*
* @file         PC_LINK.c
* @brief        上位机通信
*
* 上位机通信
*
* @version 1.0
* @author  hejun
* @date    2014年11月28日
*
*
*     修订说明：最初版本
*/
#include  "PC_LINK.h"
#include "stdlib.h"
#include "app_include.h"

SpeedSettings Speed;
DirectionSettings Direction;
/**
*	@brief 速度方向发送包初始化
* @paema[in] 包指针
*	@return 状态值
*/
Status SettingsInit()
{
    Direction.DiffSpeedx = 0.6;//1.0
    Direction.DoubleFirst = 1.0;
    Direction.DoubleSecond = 0.025;
    Direction.ImageLimit = 80;
    Direction.SpeedBack = 0;
    Direction.SteerLimit = 70;//73//;80;//85;
    Direction.SteerCenter = 662;//659;

    Speed.BangBangThreshold = 0;
    Speed.BangBangValue = 0;
    Speed.BigCrossSpeed = 0;
    Speed.IncreaseMax = 9999;
    Speed.IncreaseMin = -9999;
    Speed.LitterCrossSpeed = 0;
    Speed.StraightCornerSpeed = 325;
    Speed.StraightSpeed = 340;
    Speed.ChangeRate = 1.5;
    Speed.IncreaseDistance = 0;
    return OK;
}
/**
*	@brief 速度控制9
* @parma[in] 速度设定值
*	@return 状态值
*/
Status Control_Pid_Init(void)
{
    Speed_PidL.kp = 20.0; //18/4/0
    Speed_PidL.ki = 1.5;
    Speed_PidL.kd = 1.1;
    Speed_PidL.err = 0;
    Speed_PidL.errback = 0;
    Speed_PidL.errbackLast = 0;
    Speed_PidL.OutValue = 0;
    Speed_PidL.integral = 0;
    Speed_PidL.major_i = 200;
    Speed_PidL.min_i = 150;
    Speed_PidL.errlimit = 170;

    Speed_PidR.kp = 20.0;//18     23
    Speed_PidR.ki = 1.5;//1.4    1.0
    Speed_PidR.kd = 1.1;//1.1    0
    Speed_PidR.err = 0;
    Speed_PidR.errback = 0;
    Speed_PidR.errbackLast = 0;
    Speed_PidR.OutValue = 0;
    Speed_PidR.integral = 0;
    Speed_PidR.major_i = 200;
    Speed_PidR.min_i = 150;
    Speed_PidR.errlimit = 170;

    Dirction_Pid.kp = 1.8;//2.2;//1.9;//1.7   2
    Dirction_Pid.ki = 0;
    Dirction_Pid.kd = 1.6;//1.4  1.6
    Dirction_Pid.err = 0;
    Dirction_Pid.errback = 0;
    Dirction_Pid.errbackLast = 0;
    Dirction_Pid.OutValue = 0;
    Dirction_Pid.errlimit = 200;

    SpeedNeed.LeftQD = 0;
    SpeedNeed.RightQD = 0;
    SpeedNeed.CurrentSpeed = 0;
    SpeedNeed.LastDir = 1;
    SpeedNeed.NowDir = 1;
    SpeedNeed.DiffSpeed = 0;
    SpeedNeed.LastDiffSpeed = 0;

    return OK;
}
