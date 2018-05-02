#ifndef __USB_H_
#define	__USB_H_
#include "common.h"
#include "message_manage.h"
#include <stdio.h>


/*选择当前USB设备的类型*/
#define USB_DEVICE_CLASS USB_DEVICE_CLASS_HID

//常用操作宏定义
#define BIT_SET(BitNumber, Register)        (Register |=(1<<BitNumber))
#define BIT_CLR(BitNumber, Register)        (Register &=~(1<<BitNumber))
#define BIT_CHK(BitNumber, Register)        (Register & (1<<BitNumber))

//本构件使用的输出输入节点
//输出使用端点3
#define EP_OUT          (3)
//输入使用端点2
#define EP_IN           (2)

//USB分频因子
#define USB_FARCTIONAL_VALUE    0x02

//EP0缓冲区设置
#define EP0_SIZE            32

//EP1设置
#define EP1_VALUE           _EP_IN
#define EP1_TYPE            INTERRUPT
#define EP1_SIZE            32
#define EP1_BUFF_OFFSET     0x18

//EP2设置
#define EP2_VALUE           _EP_IN
#define EP2_TYPE            BULK
#define EP2_SIZE            32
#define EP2_BUFF_OFFSET     0x20

//EP3设置
#define EP3_VALUE           _EP_OUT
#define EP3_TYPE            BULK
#define EP3_SIZE            32
#define EP3_BUFF_OFFSET     0x28

//EP4设置
#define EP4_VALUE           DISABLE
#define EP4_SIZE            1
#define EP4_BUFF_OFFSET     0x08

//EP5设置
#define EP5_VALUE           DISABLE
#define EP5_SIZE            1
#define EP5_BUFF_OFFSET     0x08

//EP6设置
#define EP6_VALUE           DISABLE
#define EP6_SIZE            1
#define EP6_BUFF_OFFSET     0x08


//设置使用的输入 输出 端点
#define _EP_IN      USB_ENDPT_EPTXEN_MASK //时能该端点的输入传输 //USB所有的输入输出针对于主机来说
#define _EP_OUT     USB_ENDPT_EPRXEN_MASK //时能该端点的输出传输 //USB所有的输入输出针对于主机来说

#define DISABLE 0


// BDT状态
//DBT由MCU控制
#define kMCU      0x00
//DBT由SIE(USB)模块控制
#define kSIE      0x80

//BDT缓冲区DATA0格式
#define kUDATA0   0x88
//BDT缓冲区DATA1格式
#define kUDATA1   0xC8

//USB事务中，包的PID类型
#define mSETUP_TOKEN    0x0D
#define mOUT_TOKEN      0x01
#define mIN_TOKEN       0x09


/***********************************************************************************************
//标准的SETUP请求命令  在标准请求的8个字节中的第二个
************************************************************************************************/
#define mGET_STATUS           0
#define mCLR_FEATURE          1
#define mSET_FEATURE          3
#define mSET_ADDRESS          5
#define mGET_DESC             6
#define mSET_DESC             7
#define mGET_CONFIG           8
#define mSET_CONFIG           9
#define mGET_INTF             10
#define mSET_INTF             11
#define mSYNC_FRAME           12
#define	mGET_MAXLUN	          0xFE

//获取描述符
#define DEVICE_DESCRIPTOR         1
#define CONFIGURATION_DESCRIPTOR  2
#define STRING_DESCRIPTOR         3
#define INTERFACE_DESCRIPTOR      4
#define ENDPOINT_DESCRIPTOR       5
#define REPORT_DESCRIPTOR         0x22

/*根据USB2.0标准*/
/*定义USB设备的类型*/
#define USB_DEVICE_CLASS_AUDIO        1
#define USB_DEVICE_CLASS_CDC          2
#define USB_DEVICE_CLASS_HID          3
#define USB_DEVICE_CLASS_PHY          4
#define USB_DEVICE_CLASS_IMAGE        5
#define USB_DEVICE_CLASS_MASS_STORAGE 6
#define USB_DEVICE_CLASS_HUB          7
#define USB_DEVICE_CLASS_CDC_DATA     8
#define USB_DEVICE_CLASS_SMARTCARD    9
//.......
/***********************************************************************************************
// SETUP请求类型 在USB标准请求结构的 bmRequestType 中
************************************************************************************************/
enum
{
    uSETUP,
    uDATA
};

enum
{
    EP0,
    EP1,
    EP2,
    EP3,
    EP4,
    EP5,
    DUMMY,
    LOADER

};

enum
{
    uPOWER,
    uENUMERATED,
    uENABLED,
    uADDRESS,
    uREADY
};
enum
{
    fIN,
    fOUT
};

enum
{
    bEP0OUT_ODD,
    bEP0OUT_EVEN,
    bEP0IN_ODD,
    bEP0IN_EVEN,
    bEP1OUT_ODD,
    bEP1OUT_EVEN,
    bEP1IN_ODD,
    bEP1IN_EVEN,
    bEP2OUT_ODD,
    bEP2OUT_EVEN,
    bEP2IN_ODD,
    bEP2IN_EVEN,
    bEP3OUT_ODD,
    bEP3OUT_EVEN,
    bEP3IN_ODD,
    bEP3IN_EVEN
};
/***********************************************************************************************
 缓冲区描述符表(BDT)结构体
 每个端点2个BDT(一个用于微控制器，一个用于USB模块) 每个 BDT 8字节
************************************************************************************************/
typedef union _tBDT_STAT
{
    uint8_t _byte;
    //发送的MCU控制字段
    struct
    {
        uint8_t : 1;
        uint8_t : 1;
        uint8_t BSTALL: 1;             //OTG模块发出一个握手协议
        uint8_t DTS: 1;                //
        uint8_t NINC: 1;               //访问数据缓冲区时，DMA引擎不会增加它的地址
        uint8_t KEEP: 1;               //
        uint8_t DATA: 1;               //发送或接受了DATA0/DATA1包，USB模块不改变该位
        uint8_t UOWN: 1;               //BDT所有权 UOWN=1 USB模块拥有，UOWN=0 微处理器拥有
    } McuCtlBit;
    //接受控制字段
    struct
    {
        uint8_t    : 2;
        uint8_t PID: 4;                //包标志
        uint8_t    : 2;
    } RecPid;
} tBDT_STAT, *ptBDT_STAT;                           //缓冲区描述符表结构体
//BDT：缓冲区描述符表
typedef struct _tBDT
{
    tBDT_STAT Stat;
    uint8_t  dummy;
    uint16_t Cnt;     //接受到的字节数
    uint32_t Addr;    //缓冲区地址
} tBDT, *ptBDT;

/***********************************************************************************************
 SETUP包结构体
************************************************************************************************/
typedef struct _tUSB_Setup
{
    uint8_t bmRequestType; //D7:传输方向 D[6:5]类型 D[4:0]接收端
    uint8_t bRequest;      //特定请求
    uint8_t wValue_l;      //字大小字段,根据请求的不同而不同
    uint8_t wValue_h;
    uint8_t wIndex_l;      //字大小字段,根据请求的不同而不同,通常是传递索引和位移量
    uint8_t wIndex_h;
    uint8_t wLength_l;     //如果有数据阶段，该域表示所要传输的字节大小
    uint8_t wLength_h;
} tUSB_Setup;

//BDT 缓存描述符
extern tBDT tBDTtable[16];													//内部SRAM内存池

//本构件实现的接口函数
void USB_WaitDeviceEnumed(void);
uint8_t USB_IsDeviceEnumed(void);
void USB_EP_IN_Transfer(uint8_t uint8_tEP, uint8_t *puint8_tDataPointer, uint8_t uint8_tDataSize);
uint16_t USB_EP_OUT_SizeCheck(uint8_t uint8_tEP);
void USB_EnableInterface(void);
void USB_GetDescHandler(void);
uint8_t USB_Init(void);
void USB_DisConnect(void);
void USB_Connect(void);

#endif

