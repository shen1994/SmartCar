#include "ov7725_reg.h"

const REG_S OV7725_reg_config[]={
	  //0xC1+0x02---50  0x41+0x00---75   0x81+0x00---112  0xC1+0x00---150
    {OV7725_COM4         , 0xC1},	//pll---0X/0x00-4X/0x40-6X/0x80-8X/0xC0
    {OV7725_CLKRC        , 0x02},	//freq*pll/((CLK+1)*2)  //pclk=12M*8/1=96M
    {OV7725_COM2         , 0x03},	//唤醒，输出驱动能力4X
    {OV7725_COM3         , 0xD0},	//YUV输出
    {OV7725_COM7         , 0x40},	//GBR422 QVGA
		//{OV7725_COM9       , 0xF6},	//丢失行中断和场中断
    {OV7725_HSTART       , 0x3F},	//QVGA-0x3F VGA-0x26
    {OV7725_HSIZE        , 0x50},	//80
    {OV7725_VSTRT        , 0x03},	//QVGA-0x03 VGA-0x07
    {OV7725_VSIZE        , 0x78},	//120
    {OV7725_HREF         , 0x00},
    {OV7725_SCAL0        , 0x0A},	//1/4HSample 1/4VSample 
    {OV7725_AWB_Ctrl0    , 0xE0},	//白平衡使能，光线变化
    {OV7725_DSPAuto      , 0xff},	//白平衡阈值自动控制
    {OV7725_DSP_Ctrl2    , 0x0C},
    {OV7725_DSP_Ctrl3    , 0x00},
    {OV7725_DSP_Ctrl4    , 0x00},	

//分辨率，目前只支持80*60 320*240
#if (OV7725_CAMERA_W == 80)
    {OV7725_HOutSize     , 0x14},		//20/80
#elif (OV7725_CAMERA_W == 160)
    {OV7725_HOutSize     , 0x28},		//40/80
#elif (OV7725_CAMERA_W == 240)
    {OV7725_HOutSize     , 0x3c},		//60/80
#elif (OV7725_CAMERA_W == 320)
    {OV7725_HOutSize     , 0x50},		//80/80
#else

#endif

#if (OV7725_CAMERA_H == 60 )
    {OV7725_VOutSize     , 0x1E},	//30/120
#elif (OV7725_CAMERA_H == 120 )
    {OV7725_VOutSize     , 0x3c},	//60/120
#elif (OV7725_CAMERA_H == 180 )
    {OV7725_VOutSize     , 0x5a},	//90/120
#elif (OV7725_CAMERA_H == 240 )
    {OV7725_VOutSize     , 0x78},	//120/120
#else

#endif

    {OV7725_EXHCH        , 0x00},
    {OV7725_GAM1         , 0x0c},
    {OV7725_GAM2         , 0x16},
    {OV7725_GAM3         , 0x2a},
    {OV7725_GAM4         , 0x4e},
    {OV7725_GAM5         , 0x61},
    {OV7725_GAM6         , 0x6f},
    {OV7725_GAM7         , 0x7b},
    {OV7725_GAM8         , 0x86},
    {OV7725_GAM9         , 0x8e},
    {OV7725_GAM10        , 0x97},
    {OV7725_GAM11        , 0xa4},
    {OV7725_GAM12        , 0xaf},
    {OV7725_GAM13        , 0xc5},
    {OV7725_GAM14        , 0xd7},
    {OV7725_GAM15        , 0xe8},
    {OV7725_SLOP         , 0x20},
    {OV7725_LC_RADI      , 0x00},
    {OV7725_LC_COEF      , 0x13},
    {OV7725_LC_XC        , 0x08},
    {OV7725_LC_COEFB     , 0x14},
    {OV7725_LC_COEFR     , 0x17},
    {OV7725_LC_CTR       , 0x05},
    {OV7725_BDBase       , 0x99},
    {OV7725_BDMStep      , 0x03},
    {OV7725_SDE          , 0x04},
    {OV7725_BRIGHT       , 0x00},
		//{OV7725_CNST         ,60},	//图像阈值60
    {OV7725_SIGN         , 0x06},
    {OV7725_UVADJ0       , 0x11},
    {OV7725_UVADJ1       , 0x02}
};

//////////////////////////////////////
/**
* @breif 获取结构体数组大小
**/
#define GetArraySize(StructName) (sizeof((StructName)) / sizeof((StructName)[0]))

//////////////////////////////////////
unsigned char RegSize = GetArraySize(OV7725_reg_config);
