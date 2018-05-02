#include "SCCB.h"

/**
 *  @brief      SCCB延迟函数
 *  @param      time    延时时间
*/
static void SCCB_delayus(volatile uint16_t us)
{
    DelayUs(us);
}

//////////////////////////////////////
/**
*	@brief 初始化模拟SCCB接口
* 初始化SDA,SCK引脚
*	@return  状态量
*/
Status SCCB_GPIO_Config(void)
{
    GPIO_QuickInit(SCCB_PORT,  SCCB_SCK,  kGPIO_Mode_OPP);
    GPIO_QuickInit(SCCB_PORT,  SCCB_SDA,  kGPIO_Mode_OPP);
    SCCB_SCK_H();
    SCCB_SDA_H();

    PORT_PinPullConfig(SCCB_PORT, SCCB_SCK, kPullUp);
    PORT_PinPullConfig(SCCB_PORT, SCCB_SDA, kPullUp);

    return OK;
}

//////////////////////////////////////
/**
*	@brief 设置SCCB,SDA推挽输出
*	@return  状态量
*/
Status SCCB_SDA_OUTPUT(void)
{
    GPIO_PinConfig(SCCB_PORT,  SCCB_SDA, kOutput);
    return OK;
}

//////////////////////////////////////
/**
*	@brief 设置SCCB,SDA浮空输入
*	@return  状态量
*/
Status SCCB_SDA_INPUT(void)
{
    GPIO_PinConfig(SCCB_PORT,  SCCB_SDA, kInput);
    return OK;
}

//////////////////////////////////////
/**
*	@brief start命令,SCCB的起始信号
* SCK----
*	SDA--__
*	@return  状态量
*/
Status startSCCB(void)
{
    SCCB_SDA_H();     //数据线高电平
    SCCB_SCK_H();	   //在时钟线高的时候数据线由高至低
    SCCB_delayus(15);
		
		SCCB_SDA_IN;
		if(!SCCB_SDA_STATE){	//启动判忙
			SCCB_SDA_OUT;
			return FALSE;
		}
		SCCB_SDA_OUT;
		
		SCCB_SDA_L();
    SCCB_delayus(15);
    SCCB_SCK_L();	 //数据线恢复低电平，单操作函数必要
		
    return OK;
}

//////////////////////////////////////
/**
*	@brief stop命令,SCCB的停止信号
*	@return  状态量
*/
Status stopSCCB(void)
{
		SCCB_SCK_L();
    SCCB_SDA_L();
    SCCB_delayus(15);

    SCCB_SCK_H();
    SCCB_delayus(15);

    SCCB_SDA_H();
    SCCB_delayus(15);

    return OK;
}

//////////////////////////////////////
/**
*	@brief noAck,用于连续读取中的最后一个结束周期
*	@return  状态量
*/
Status noAck(void)
{
    SCCB_SCK_L();
    SCCB_delayus(15);

    SCCB_SDA_H();
    SCCB_delayus(15);

    SCCB_SCK_H();
    SCCB_delayus(15);

    SCCB_SCK_L();
    SCCB_delayus(15);

    return OK;
}

//////////////////////////////////////
/**
*	@brief Ack
*	@return  状态量
*/
Status Ack(void)
{
    SCCB_SCK_L();
    SCCB_delayus(15);

    SCCB_SDA_L();
    SCCB_delayus(15);

    SCCB_SCK_H();
    SCCB_delayus(15);

    SCCB_SCK_L();
    SCCB_delayus(15);
    return OK;
}

//////////////////////////////////////
/**
*	@brief 写入一个字节的数据到SCCB
*	@parma 写入数据
*	@return  状态量
*/
unsigned char SCCBwriteByte(unsigned char m_data)
{
    unsigned char j, tem;

    for(j = 0; j < 8; j++) //循环8次发送数据
    {
        if(m_data & 0x80)
        {
            SCCB_SDA_H();
        }
        else
        {
            SCCB_SDA_L();
        }
				m_data <<= 1;
        SCCB_delayus(15);
        SCCB_SCK_H();
        SCCB_delayus(15);
        SCCB_SCK_L();
    }			
    SCCB_delayus(5);
    SCCB_SDA_IN;
    SCCB_delayus(10);
    SCCB_SCK_H();
    SCCB_delayus(25);
    if(SCCB_SDA_STATE)  //ACK
    {
        tem = 0;
    }
    else tem = 1;
    SCCB_SCK_L();
    SCCB_delayus(10);
    SCCB_SDA_OUT;
    return(tem);
}

//////////////////////////////////////
/**
*	@brief 一个字节数据读取并且返回
*	@return  读取的数据
*/
unsigned char SCCBreadByte(void)
{
    unsigned char read, j;
    read = 0x00;

    SCCB_SDA_IN;/*设置SDA为输入*/
    SCCB_delayus(10);
    for(j = 8; j > 0; j--) //循环8次接收数据
    {
        read <<= 1;
				SCCB_SCK_L();
				SCCB_delayus(10);
        SCCB_SCK_H();
        SCCB_delayus(10);
        if(SCCB_SDA_STATE)
        {
            read |= 0x01;
        }
        SCCB_delayus(2);
    }
		SCCB_SCK_L();
    SCCB_SDA_OUT;/*设置SDA为输出*/
    return(read);
}

//////////////////////////////////////
/**
*	@brief   连续写2次iic
*	@return  状态量
*/
Status SCCB_WriteRegister(unsigned char regID, unsigned char regDat)
{
    startSCCB();
    if(0 == SCCBwriteByte(DEV_ADDR)) //写地址
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);

    if(0 == SCCBwriteByte(regID)) //寄存器ID
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);

    if(0 == SCCBwriteByte(regDat)) //写数据到积存器
    {
        stopSCCB();
        return 0;
    }

    stopSCCB();//发送SCCB 总线停止传输命令

    return(1);//成功返回
}

//////////////////////////////////////
/**
*	@brief   连续写2次iiC(多次)
*	@return  状态量
*/
Status SCCB_WriteDoubleRegister(unsigned char regID, unsigned char regDat)
{
    int i = 0;
    while(0 == SCCB_WriteRegister(regID, regDat))
    {
        i++;
        if(i == 20)
        {
            return 0 ;
        }
    }
    return 1;
}

/**
*	@brief   连续写2次iic
*	@return  状态量
*/
Status SCCB_ReadRegister(unsigned char* pBuffer, unsigned int Length, unsigned char regID)
{
		unsigned char ReadData=0;
    startSCCB();
		ReadData = SCCBwriteByte(DEV_ADDR);
    if(0 == ReadData) //写地址
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);
		
		ReadData = SCCBwriteByte(regID);
    if(0 == ReadData) //寄存器ID
    {
        stopSCCB();
        return 0;
    }
		stopSCCB();
    DelayUs(20);
		
		startSCCB();
		ReadData = SCCBwriteByte(DEV_ADDR + 1);
    if(0 == ReadData) //读地址
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);
		
		while(Length){
			*pBuffer = SCCBreadByte();
			if(1 == Length){
				noAck();
			}else{
				Ack();
			}
			pBuffer++;
			Length--;
		}
    return(1);//成功返回
}
