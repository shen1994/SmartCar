#ifndef KEY_H__
#define KEY_H__
#include "common.h"
#include "gpio.h"
#include "Status.h"
#include "OLED.h"
#include "Control.h"
#include "ov7725.h"

#define KEY_PORT    HW_GPIOC
#define KEY_GND    8
#define KEY_UP     6
#define KEY_DOWN   9
#define KEY_LEFT   7
#define KEY_RIGHT  11
#define KEY_PRESS  4


#define Parma_Num 4//参数个数
#define Cur_Num 3 ///可供调参的界面数

#define Back_Cur_Num    127
#define Speed_Cur_Num     0
#define Dirction_Cur_Num  1
#define Camera_Cur_Num    2
#define Stop_Cur_Num      3
typedef struct
{
    uint8_t parma_turn;//当前调参位置
    char *parma_name[Parma_Num];//最多分配5个参数
    uint8_t parma_site[Parma_Num];//参数位置
    float   parma_step_Change[Parma_Num];//step的调整值
    float   parma_step[Parma_Num]; //参数的step初始值
    float   parma[Parma_Num];//参数值
} CUR_SET[Cur_Num];

Status Cur_Set_Init(void);
uint8_t Key_Scan(void);
Status Key_Ack(uint8_t Key_Num);
Status Speed_Cur(void);
Status Dirction_Cur(void);
Status Camera_Cur(void);
Status Stop_Cur(void);
Status Add_Value(void);
Status Dec_Value(void);
Status Add_Step(void);
Status Dec_Step(void);
Status Last_Parma(void);
Status Next_Parma(void);
Status Confirm_Set(void);
Status Back_Cur(char Comfirm);
Status Key_Work(void);
Status Key_Save_Init(void);
Status FinishSet(void);

extern unsigned char Threshold;
#endif
