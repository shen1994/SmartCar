/**
*     COPYRIGHT NOTICE
*     All rights reserved.
*
* @file         key.c
* @brief        按键GUI
*
*
* @version 1.0
* @author  hejun
* @date    2015年1月3日
*
*
*     修订说明：最初版本
*/

#include "key.h"
//勿动
CUR_SET Cur_Set;
static char Cur_Now = Back_Cur_Num;
static char Comfirm_Times = 0;
unsigned char RecX=55;
signed char Cur_Count=Back_Cur_Num;

//一般在60左右，可以取57
unsigned char Threshold = 62;

/**
* @breif 按键按下去之后延长一段时间
**/
static void DelayKeyPress(void){
	for(int i=0;i<1600;i++)
		for(int j=0;j<2650;j++);
}

/**
*	@brief 按键扫描
*	@return 按键值
*/
uint8_t Key_Scan(void)
{
    uint8_t key_num = 0;
	  if(GPIO_ReadBit(KEY_PORT,KEY_UP))
			key_num = 2;
		else if(GPIO_ReadBit(KEY_PORT,KEY_DOWN))
			key_num = 8;
		else if(GPIO_ReadBit(KEY_PORT,KEY_LEFT))
			key_num = 4;
		else if(GPIO_ReadBit(KEY_PORT,KEY_RIGHT))
			key_num = 6;
		else if(GPIO_ReadBit(KEY_PORT,KEY_PRESS))
			key_num = 5;
		DelayKeyPress();
    return key_num;
}

/**
*	@brief  响应按键扫描
* @parma[in] 扫描的按键值
*	@return 状态值
*/
Status Key_Ack(uint8_t Key_Num)
{
    switch(Key_Num)
    {
    case 2: if(Cur_Now != Back_Cur_Num)Last_Parma();
        break;
    case 4: if(Cur_Now == Back_Cur_Num)//选择参数界面
            {
              if(Cur_Count == Back_Cur_Num)
                Cur_Count=1;
                Cur_Count--;
              if(Cur_Count<0)Cur_Count=0;
              if(Cur_Count==Speed_Cur_Num)Speed_Cur();
              else if(Cur_Count==Dirction_Cur_Num)Dirction_Cur();
              else if(Cur_Count==Camera_Cur_Num)Camera_Cur();
            }
            else
            {
              Dec_Value();
            }
        break;
    case 5:
        Confirm_Set();
        break;
    case 6: if(Cur_Now == Back_Cur_Num)
            {
              if(Cur_Count == Back_Cur_Num)
                Cur_Count=-1;
              Cur_Count++;
              if(Cur_Count>=3)Cur_Count=3;
              if(Cur_Count==Speed_Cur_Num)Speed_Cur();
              else if(Cur_Count==Dirction_Cur_Num)Dirction_Cur();
              else if(Cur_Count==Camera_Cur_Num)Camera_Cur();
              else if(Cur_Count==Stop_Cur_Num)Stop_Cur();
            }
            else
            {
             Add_Value();
            }
        break;
    case 8: if(Cur_Now != Back_Cur_Num)Next_Parma();
        break;
    default:
        break;
    }
    return OK;
}
/**
*	@brief  调参界面初始化
*	@return 状态值
*/
Status Cur_Set_Init(void)
{
		GPIO_QuickInit (KEY_PORT, KEY_GND,   kGPIO_Mode_OPP);
    GPIO_QuickInit (KEY_PORT, KEY_UP,    kGPIO_Mode_IPD);
    GPIO_QuickInit (KEY_PORT, KEY_DOWN,   kGPIO_Mode_IPD);
    GPIO_QuickInit (KEY_PORT, KEY_LEFT,    kGPIO_Mode_IPD);
		GPIO_QuickInit (KEY_PORT, KEY_RIGHT,   kGPIO_Mode_IPD);
    GPIO_QuickInit (KEY_PORT, KEY_PRESS,    kGPIO_Mode_IPD);
    GPIO_WriteBit(KEY_PORT, KEY_GND,1);
		
    ///速度方向PID界面初始化
    Cur_Set[Speed_Cur_Num].parma_turn = 0;
    Cur_Set[Speed_Cur_Num].parma_name[0] = "SpeedST";
    Cur_Set[Speed_Cur_Num].parma_name[1] = "GoRamp";
    Cur_Set[Speed_Cur_Num].parma_name[2] = "DiffKp";
    Cur_Set[Speed_Cur_Num].parma_name[3] = "DiffKd";

    Cur_Set[Speed_Cur_Num].parma_site[0] = 0;
    Cur_Set[Speed_Cur_Num].parma_site[1] = 2;
    Cur_Set[Speed_Cur_Num].parma_site[2] = 4;
    Cur_Set[Speed_Cur_Num].parma_site[3] = 6;

    Cur_Set[Speed_Cur_Num].parma_step[0] = 10;
    Cur_Set[Speed_Cur_Num].parma_step[1] = 10;
    Cur_Set[Speed_Cur_Num].parma_step[2] = 0.1;
    Cur_Set[Speed_Cur_Num].parma_step[3] = 0.1;

    Cur_Set[Speed_Cur_Num].parma_step_Change[0] = 10;
    Cur_Set[Speed_Cur_Num].parma_step_Change[1] = 10;
    Cur_Set[Speed_Cur_Num].parma_step_Change[2] = 0.1;
    Cur_Set[Speed_Cur_Num].parma_step_Change[3] = 0.1;

    Cur_Set[Speed_Cur_Num].parma[0] = SpeedSetStraight;
    Cur_Set[Speed_Cur_Num].parma[1] = SpeedSetRamp;
    Cur_Set[Speed_Cur_Num].parma[2] = Direction_P;
    Cur_Set[Speed_Cur_Num].parma[3] = Direction_D;
    ///方向界面初始化
    Cur_Set[Dirction_Cur_Num].parma_turn = 0;
    Cur_Set[Dirction_Cur_Num].parma_name[0] = "SurvoCen";
    Cur_Set[Dirction_Cur_Num].parma_name[1] = "SpeedKp";
    Cur_Set[Dirction_Cur_Num].parma_name[2] = "SpeedKi";
    Cur_Set[Dirction_Cur_Num].parma_name[3] = "Thres";

    Cur_Set[Dirction_Cur_Num].parma_site[0] = 0;
    Cur_Set[Dirction_Cur_Num].parma_site[1] = 2;
    Cur_Set[Dirction_Cur_Num].parma_site[2] = 4;
    Cur_Set[Dirction_Cur_Num].parma_site[3] = 6;

    Cur_Set[Dirction_Cur_Num].parma_step[0] = 1;
    Cur_Set[Dirction_Cur_Num].parma_step[1] = 0.2;
    Cur_Set[Dirction_Cur_Num].parma_step[2] = 0.1;
    Cur_Set[Dirction_Cur_Num].parma_step[3] = 2;

    Cur_Set[Dirction_Cur_Num].parma_step_Change[0] = 1;
    Cur_Set[Dirction_Cur_Num].parma_step_Change[1] = 0.1;
    Cur_Set[Dirction_Cur_Num].parma_step_Change[2] = 0.1;
    Cur_Set[Dirction_Cur_Num].parma_step_Change[3] = 1;

    Cur_Set[Dirction_Cur_Num].parma[0] = SurvoCenter;
    Cur_Set[Dirction_Cur_Num].parma[1] = SpeedKp;
    Cur_Set[Dirction_Cur_Num].parma[2] = SpeedKi;
    Cur_Set[Dirction_Cur_Num].parma[3] = Threshold;

    ///摄像头界面初始化
    Cur_Set[Camera_Cur_Num].parma_turn = 0;
    Cur_Set[Camera_Cur_Num].parma_name[0] = "DiffKk";
    Cur_Set[Camera_Cur_Num].parma_name[1] = "RoadSTK";
    Cur_Set[Camera_Cur_Num].parma_name[2] = "RoadCOK";
    Cur_Set[Camera_Cur_Num].parma_name[3] = "IsBackS";

    Cur_Set[Camera_Cur_Num].parma_site[0] = 0;
    Cur_Set[Camera_Cur_Num].parma_site[1] = 2;
    Cur_Set[Camera_Cur_Num].parma_site[2] = 4;
    Cur_Set[Camera_Cur_Num].parma_site[3] = 6;

    Cur_Set[Camera_Cur_Num].parma_step[0] = 0.1;
    Cur_Set[Camera_Cur_Num].parma_step[1] = 5;
    Cur_Set[Camera_Cur_Num].parma_step[2] = 5;
    Cur_Set[Camera_Cur_Num].parma_step[3] = 1;

    Cur_Set[Camera_Cur_Num].parma_step_Change[0] = 0.1;
    Cur_Set[Camera_Cur_Num].parma_step_Change[1] = 10;
    Cur_Set[Camera_Cur_Num].parma_step_Change[2] = 10;
    Cur_Set[Camera_Cur_Num].parma_step_Change[3] = 1;

    Cur_Set[Camera_Cur_Num].parma[0] = Direction_K;
    Cur_Set[Camera_Cur_Num].parma[1] = 0;
    Cur_Set[Camera_Cur_Num].parma[2] = 0;
    Cur_Set[Camera_Cur_Num].parma[3] = isBackStop;

    return OK;
}

/**
*	@brief  速度调参界面
*	@return 状态值
*/
Status Speed_Cur(void)
{
    LED_CLS();
    //Cur_Now = Speed_Cur_Num;
    Comfirm_Times = 0;
    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[0], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[0]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[0], Cur_Set[Speed_Cur_Num].parma[0], 2);

    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[1], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[1]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[1], Cur_Set[Speed_Cur_Num].parma[1], 2);

    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[2], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[2]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[2], Cur_Set[Speed_Cur_Num].parma[2], 2);

    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[3], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[3]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[3], Cur_Set[Speed_Cur_Num].parma[3], 2);
    
    //LCD_Rectangle(RecX,Cur_Set[Speed_Cur_Num].parma_site[Cur_Set[Speed_Cur_Num].parma_turn]);
    return OK;
}

/**
*	@brief  方向调参界面
*	@return 状态值
*/
Status Dirction_Cur(void)
{
    LED_CLS();
    //Cur_Now = Dirction_Cur_Num;
    Comfirm_Times = 0;

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[0], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[0]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[0], Cur_Set[Dirction_Cur_Num].parma[0], 2);

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[1], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[1]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[1], Cur_Set[Dirction_Cur_Num].parma[1], 2);

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[2], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[2]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[2], Cur_Set[Dirction_Cur_Num].parma[2], 2);

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[3], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[3]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[3], Cur_Set[Dirction_Cur_Num].parma[3], 2);
    
    //LCD_Rectangle(RecX,Cur_Set[Dirction_Cur_Num].parma_site[Cur_Set[Dirction_Cur_Num].parma_turn]);
    return OK;
}

/**
*	@brief  摄像头调参界面
*	@return 状态值
*/
Status Camera_Cur(void)
{
    LED_CLS();
    //Cur_Now = Camera_Cur_Num;
    Comfirm_Times = 0;

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[0], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[0]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[0], Cur_Set[Camera_Cur_Num].parma[0], 2);

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[1], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[1]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[1], Cur_Set[Camera_Cur_Num].parma[1], 2);

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[2], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[2]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[2], Cur_Set[Camera_Cur_Num].parma[2], 2);

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[3], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[3]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[3], Cur_Set[Camera_Cur_Num].parma[3], 2);
    
    //LCD_Rectangle(RecX,Cur_Set[Camera_Cur_Num].parma_site[Cur_Set[Camera_Cur_Num].parma_turn]);
    return OK;
}
/**
*	@brief  停止调参界面
*	@return 状态值
*/
Status Stop_Cur(void)
{
    LED_CLS();
    Comfirm_Times = 0;
    LED_P8x16Str(20, 4, (uint8_t *)"Skip!");
    return OK;
}
/**
*	@brief  参数值增加
*	@return 状态值
*/
Status Add_Value(void)
{
    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn] += Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn];
    LED_PrintValueF(64, Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn],
                    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn], 2);
    return OK;
}

/**
*	@brief  参数值减少
*	@return 状态值
*/
Status Dec_Value(void)
{
    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn] -= Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn];
    LED_PrintValueF(64, Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn],
                    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn], 2);
    return OK;
}

/**
*	@brief  参数步长增加
*	@return 状态值
*/
Status Add_Step(void)
{
    Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] += Cur_Set[Cur_Now].parma_step_Change[Cur_Set[Cur_Now].parma_turn];
    return OK;
}

/**
*	@brief  参数步长减少
*	@return 状态值
*/
Status Dec_Step(void)
{
    Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] -= Cur_Set[Cur_Now].parma_step_Change[Cur_Set[Cur_Now].parma_turn];
    if(Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] < 0)
        Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] = 0;
    return OK;
}

/**
*	@brief  上一个参数
*	@return 状态值
*/
Status Next_Parma(void)
{
    unsigned RecTurn=0,ClearTurn=0;
    ClearTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];
    Cur_Set[Cur_Now].parma_turn += 1;
    if(Cur_Set[Cur_Now].parma_turn > Parma_Num - 1)
        Cur_Set[Cur_Now].parma_turn = Parma_Num - 1;
    RecTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];
    if(RecTurn!=ClearTurn)
    {
      LCD_Rectangle(RecX,RecTurn);
      LCD_ClearRectangle(RecX,ClearTurn); 
    }
    return OK;
}

/**
*	@brief  下一个参数
*	@return 状态值
*/
Status Last_Parma(void)
{
    unsigned RecTurn=0,ClearTurn=0;
    ClearTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];  
    if(Cur_Set[Cur_Now].parma_turn < 1)
        Cur_Set[Cur_Now].parma_turn = 1;
    Cur_Set[Cur_Now].parma_turn -= 1;
     RecTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];
    if(RecTurn!=ClearTurn)
    {
      LCD_Rectangle(RecX,RecTurn);
      LCD_ClearRectangle(RecX,ClearTurn); 
    }
    return OK;
}

/**
*	@brief  确定按钮
*	@return 状态值
*/
Status Confirm_Set(void)
{
    unsigned RecTurn=0;
    if(Cur_Count>=0&&Cur_Count<3)
    RecTurn=Cur_Set[Cur_Count].parma_site[Cur_Set[Cur_Count].parma_turn];
    if(Cur_Now != Back_Cur_Num)
    {
      //退出缓存的界面
      Cur_Now = Back_Cur_Num;
      LCD_ClearRectangle(RecX,RecTurn);
    }
    else if(Cur_Count!=Back_Cur_Num&&Cur_Count!=Stop_Cur_Num)
    {
      ///进入缓存的界面（除去基础起始界面和退出界面）
      Cur_Now = Cur_Count;
      LCD_Rectangle(RecX,RecTurn);
    }
    if(Cur_Count==Stop_Cur_Num)
    Comfirm_Times++;
    if(Comfirm_Times>20)
    {
      LED_CLS();
      Cur_Now = Back_Cur_Num;
      Back_Cur(Comfirm_Times);
    }
    return OK;
}

/**
*	@brief  背景
*	@return 状态值
*/
Status Back_Cur(char Comfirm)
{
    if(Comfirm_Times <20)
        LED_P8x16Str(20, 4, (uint8_t *)"System Set!");
    else if(Comfirm_Times >= 40)
        LED_P8x16Str(25, 4, (uint8_t *)"Ready go!");
    return OK;
}

/**
*	@brief  按键工作
*	@return 状态值
*/
Status Key_Work(void)
{
    uint8_t key = 0;
    Cur_Set_Init();
    Back_Cur(Comfirm_Times);
    while(1)
    {
        key = Key_Scan();
        Key_Ack(key);
        if(Comfirm_Times >= 4)
        {
            LED_CLS();
            FinishSet();
            break;
        }
    }
    return OK;
}

/**
*	@brief   根据按键值写入
*	@return 状态值
*/
Status FinishSet(void)
{
    SpeedSetStraight = Cur_Set[Speed_Cur_Num].parma[0];
    SpeedSetRamp = Cur_Set[Speed_Cur_Num].parma[1];
    Direction_P = Cur_Set[Speed_Cur_Num].parma[2];
    Direction_D = Cur_Set[Speed_Cur_Num].parma[3];

    SurvoCenter=Cur_Set[Dirction_Cur_Num].parma[0];
    SpeedKp=Cur_Set[Dirction_Cur_Num].parma[1];
    SpeedKi=Cur_Set[Dirction_Cur_Num].parma[2];
    Threshold=(unsigned char)Cur_Set[Dirction_Cur_Num].parma[3]; 
   
    Direction_K = Cur_Set[Camera_Cur_Num].parma[0];
		//RoadStraight_K = (int)Cur_Set[Camera_Cur_Num].parma[1];
    //RoadCorner_K = (int)Cur_Set[Camera_Cur_Num].parma[2];
		isBackStop = (int)Cur_Set[Camera_Cur_Num].parma[3];
    return OK;
}
