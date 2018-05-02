#include "ov5116.h"
#include "Save_Runway.h"
#include "Key.h"
uint8_t ov5116_pic[HIGH_O][WIDTH_O];
static int OV_HrefCount = 0;
//uint8_t Image_Flag = 0; //图像采集是否完成
/**
*	@brief 行消隐延时
*	@parma[in] 通过执行指令延时
*	@return  无
*/
void ov5116_Delay(uint16_t usTime)
{
    uint16_t i;
    for (i = usTime; i > 0; i--)
    {
    }
    return;
}

/**
*	@brief 场中断处理函数
*	@return  无
*/
static void Vref_NVIC(uint32_t array)
{
    if(array & (1 << Vref_PIN))
    {
			OV_HrefCount = 0;
			ov5116_Delay(24000);//场消隐
			NVIC_DisableIRQ(OV_VsyncIRQ);
			NVIC_EnableIRQ(OV_HrefIRQ);
		}
}


/**
*	@brief 行中断处理函数
*	@return  无
*/
static void Href_NVIC(uint32_t array)
{
		char i=0;
    if(array & (1 << Href_PIN))
    {			
				if(OV_HrefCount % 4 == 0)
				{
						 ov5116_Delay(125);
						 ov5116_pic[OV_HrefCount / 4][0]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][1]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][2]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][3]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][4]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][5]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][6]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][7]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][8]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][9]=(PDin(8));OVHrefDelay;
    
             ov5116_pic[OV_HrefCount / 4][10]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][11]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][12]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][13]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][14]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][15]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][16]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][17]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][18]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][19]=(PDin(8));OVHrefDelay;
           
             ov5116_pic[OV_HrefCount / 4][20]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][21]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][22]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][23]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][24]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][25]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][26]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][27]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][28]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][29]=(PDin(8));OVHrefDelay;
   
             ov5116_pic[OV_HrefCount / 4][30]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][31]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][32]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][33]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][34]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][35]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][36]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][37]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][38]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][39]=(PDin(8));OVHrefDelay;
           
             ov5116_pic[OV_HrefCount / 4][40]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][41]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][42]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][43]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][44]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][45]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][46]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][47]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][48]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][49]=(PDin(8));OVHrefDelay;
            
             ov5116_pic[OV_HrefCount / 4][50]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][51]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][52]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][53]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][54]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][55]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][56]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][57]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][58]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][59]=(PDin(8));OVHrefDelay;

             ov5116_pic[OV_HrefCount / 4][60]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][61]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][62]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][63]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][64]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][65]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][66]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][67]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][68]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][69]=(PDin(8));OVHrefDelay;
  
             ov5116_pic[OV_HrefCount / 4][70]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][71]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][72]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][73]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][74]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][75]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][76]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][77]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][78]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][79]=(PDin(8));OVHrefDelay;
             
             ov5116_pic[OV_HrefCount / 4][80]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][81]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][82]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][83]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][84]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][85]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][86]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][87]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][88]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][89]=(PDin(8));OVHrefDelay;
                        
             ov5116_pic[OV_HrefCount / 4][90]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][91]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][92]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][93]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][94]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][95]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][96]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][97]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][98]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][99]=(PDin(8));OVHrefDelay;
                                
             ov5116_pic[OV_HrefCount / 4][100]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][101]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][102]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][103]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][104]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][105]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][106]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][107]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][108]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][109]=(PDin(8));OVHrefDelay;
               
             ov5116_pic[OV_HrefCount / 4][110]=(PDin(8));OVHrefDelay; 
             ov5116_pic[OV_HrefCount / 4][111]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][112]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][113]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][114]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][115]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][116]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][117]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][118]=(PDin(8));OVHrefDelay;
             ov5116_pic[OV_HrefCount / 4][119]=(PDin(8));OVHrefDelay;
				}
				OV_HrefCount++;
				if(OV_HrefCount > 236)
				{
					NVIC_DisableIRQ(OV_HrefIRQ);
					Image_Flag = OK;
				}
		}
}
/**
*	@brief ov5116引脚及中断初始化
* 场中断,行中断，信号引脚初始化.
*	@return  状态量
*/
void ov5116_Init(void)
{
    GPIO_QuickInit(Vref_PORT,  Vref_PIN,  kGPIO_Mode_IPU);
    GPIO_QuickInit(Href_PORT,  Href_PIN,  kGPIO_Mode_IPU);
    GPIO_QuickInit(Vedio_PORT, Vedio_PIN, kGPIO_Mode_IPU);

    /*场中断注册,使能*/
    GPIO_CallbackInstall(Vref_PORT, Vref_NVIC);
		GPIO_CallbackInstall(Href_PORT, Href_NVIC);
		
    GPIO_ITDMAConfig(Vref_PORT, Vref_PIN, kGPIO_IT_FallingEdge);
    GPIO_ITDMAConfig(Href_PORT, Href_PIN, kGPIO_IT_RisingEdge);
		NVIC_DisableIRQ(OV_HrefIRQ);
}
