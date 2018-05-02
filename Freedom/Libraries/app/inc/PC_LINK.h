#ifndef  PC_LINK_H_
#define  PC_LINK_H_
#include "common.h"
#include "uart.h"
#include "nrf24l01.h"
#include "OLED.h"
#include "pid.h"
typedef struct
{
    uint8_t Head[2];//2个字节的字头
    uint8_t DC;//数据/控制
    uint8_t length;//数据长度
    uint8_t *DataBuf;//数据
} PCLinkPack;
#define SpeedPID 0x02
#define DirectionPID 0x03
#define FirstOther 0x04
#define SecondOther 0x05
#define DirctionSet 0x06
#define PowerSetting 0x07
#define ControlDataLength 0x06
#define SpeedGetSet 0x08
#define ImageSet 0x0a
typedef enum 
{
  OnlyControl=0,
	ControlAndWave,
	OnlyWave
}SendType;
typedef struct 
{
  int StraightSpeed;
	int LitterCrossSpeed;
	int BigCrossSpeed;
	int StraightCornerSpeed;
	
	int IncreaseMax;
	int IncreaseMin;
	int BangBangThreshold;
	int BangBangValue;
	
	int IncreaseDistance;
	float ChangeRate;
}SpeedSettings;

typedef struct 
{
  float SpeedBack;
	float DoubleFirst;
	float DoubleSecond;
	int ImageLimit;	
	int SteerLimit;
	float DiffSpeedx;
	
	int SteerCenter;
}DirectionSettings;

Status InitPCLinkPack(PCLinkPack *Pack, uint8_t HeadF, uint8_t HeadS, uint8_t DC, uint8_t length);
void PCLink(PCLinkPack *Pack, float *OutData);
void SendPack(PCLinkPack *ControlPack,PCLinkPack *WavePack,SendType Type);
void AnalysisData(unsigned char *Buf);
Status PID_Callback(float *No, char Flag);
Status Other_Callback(float *No, char Flag);
Status Power_Callback(float *No, char Flag);
Status DirctionSetting_Callback(unsigned char *No, char Flag);
Status Speed_Callback(unsigned char *No, char Flag);
Status SettingsInit(void);
Status AckPackInit(void);
Status AckPackSend(char Flag);
extern uint8_t AckStatus;
extern SpeedSettings Speed;
extern DirectionSettings Direction;
#endif
