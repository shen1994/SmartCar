#include "SystemInit.h"
//#include "PID.h"
#include "CusTomPID.h"
#include "Fuzzy.h"

uint8_t isCamera=0;
uint8_t isRun = 1;
uint8_t BeepLoopTime = 0;
/////////////////////////////////////////////////////
/**
* @breif 机械调按键扰动延时
**/
Status DelaySysTime(void){
		for(int i=0;i<800;i++)
			for(int j=5000;j>0;j--);
	return OK;
}
/////////////////////////////////////////////////////
/**
*	@brief 中断优先级设置
*
*	@return 结果状态
*/
Status NVIC_SET(void)
{
#if (defined USE_USART3) || (defined USE_OV7725)
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
#endif

#ifdef USE_OV7725
    NVIC_SetPriority(OV7725_VSYNC_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_2, 1, 2));     //其次场中断
    NVIC_SetPriority(OV7725_HREFC_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_2, 1, 1));     //其次行中断
    NVIC_SetPriority(OV7725_DMA_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_2, 1, 0));      //然后像素传输
#endif

#ifdef USE_USART3 
    NVIC_SetPriority(PORTC_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, 2, 1));           //USART3中断
#endif

#ifdef USE_PIT0
		NVIC_SetPriority(PIT0_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_2, 2, 0));	//PIT0定时器
#endif

    return OK;
}
/////////////////////////////////////////////////////
/**
*	@brief 系统初始化
*	@return 结果状态
*/
Status System_Init(void)
{
    //禁用NMI中断
    GPIO_QuickInit(HW_GPIOA, 4, kGPIO_Mode_OPP);
    GPIO_WriteBit(HW_GPIOA, 4, 1);
		
    DisableInterrupts();
		
#ifdef USE_OLED
		LED_Init(); 
#endif

#ifdef USE_BUZZER
    Buzzer_Init;
    Buzzer_Off;
		GPIO_QuickInit(HW_GPIOE, 10, kGPIO_Mode_OPP);	//测试使用
#endif		

#ifdef USE_MOTOR
    Motor_Init();
#endif

		NVIC_SET();	    //设置中断向量优先级
		
#ifdef USE_PIT0
		PIT_QuickInit(HW_PIT_CH0,4);	//5ms
		PIT_CallbackInstall(HW_PIT_CH0,PIT_DoCallBack);	
		PIT_ITDMAConfig(HW_PIT_CH0, kPIT_IT_TOF, false);		
#endif
		
#ifdef USE_USART3
		UART_QuickInit(UART3_RX_PC16_TX_PC17, 115200);
		UART_ITDMAConfig(HW_UART3,kUART_IT_Rx);
		UART_CallbackRxInstall(HW_UART3,Uart_ISR);
#endif		

#ifdef USE_FIVE_KEY
		Key_Work();
		DelaySysTime();
#endif

#ifdef USE_MOTOR
		SteerInvalidAgain();
#endif

#ifdef USE_PID
		PID_Init();
		PID_SetKpidGo(SpeedKp,SpeedKi,SpeedKd);
		PID_SetPointGo(SpeedSetStraight);
/*
		InitPIDVaribles(&SpeedPid);
		InitPIDVaribles(&DirectionPid);
		SetPID_Kpid(&SpeedPid, SpeedKp, SpeedKi, SpeedKd);
		SetPID_Kpid(&DirectionPid, Direction_P, Direction_I, Direction_D);
		SetPID_AimValue(&SpeedPid, (float)SpeedSetStraight);
		SetPID_AimValue(&DirectionPid, 0);
*/

#endif

#ifdef USE_FUZZY
		Fuzzy_init();
		TargetSpeed_FuzzyInit(&TartgetSpeed_Reference);	//放进初始化
#endif

#ifdef USE_OV7725
		OV7725_Init();
#endif

    EnableInterrupts();
		
    return OK;
}
/**------------图像采集相关-----------------------**/
/**
* @breif 图像采集
**/
static void Uart_ISR(uint16_t DataRec)
{
	uint8_t Rxdata=0;
	Rxdata=DataRec&0xff;
	if(Rxdata=='S') isCamera=1;
	//if(Rxdata == 'R') isRun = 1;
	//if(Rxdata == 'T') isRun = 0;
}
/////////////////////////////////////////////////////
/**
*	@brief   图像采集完后ToPC---处理
*	@return  状态
*/
Status ImageCapture(void){
	  int  i=0,j=0;
    uint8_t *PHOTO_BUFFPoint = NULL;
		
    for(i=0;i<60;i++)
    {
        for(j=0;j<80;j++)
        {
            PHOTO_BUFFPoint=OV7725_IMAGE[i]+j;
            UART_WriteByte(HW_UART3,*PHOTO_BUFFPoint);				
        }
    }
		return OK;
}
/////////////////////////////////////////////////////
/**
* @breif 定时器中断函数回调
**/
void PIT_DoCallBack(void){
	return;
}
/////////////////////////////////////////////////////
