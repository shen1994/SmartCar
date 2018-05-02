#ifndef _CONTROL_H_
#define _CONTROL_H_
#include "app_include.h"
#include "Status.h"
#include "math.h"
//#include "PID.h"
#include "CusTomPID.h"
#include "Fuzzy.h"
#include "TargetSpeed.h"

Status Motor_Init(void);
Status SteerInvalidAgain(void);
Status Get_Speed(int32_t *Speed);
Status Speed_Control(int Speed_Give);
Status Dirction_Control(int Dirction_Give);
Status TimeToControlSteer(void);
Status TimeToControlMotor(void);
Status TimeToSleep(void);
Status TimeToStop(void);

extern int32_t MotorQD;
extern int CenterControl;
extern int   SpeedSetStraight,SpeedSetRamp; 
extern float SpeedKp;
extern float SpeedKi;
extern float SpeedKd;
extern float Direction_P;
extern float Direction_D;
extern float Direction_K;
extern int SurvoCenter;
extern int isBackStop;

#endif
