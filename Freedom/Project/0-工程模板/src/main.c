#include "app_include.h"
uint8_t  OV7725_IMAGES[HIGH_D][(WIDTH_D / 8)];

int main(void)
{   
    DelayInit();
    System_Init();	
#ifdef USE_OV7725
//必须写在这里，不加延时会在屏幕左侧产生白色循环流动的花纹
//同时也是为了发车的时候能够留一段时间
		DelayMs(2000);
		StartGetImage();
#endif

    while(1)
    {
        if(Image_Flag != FALSE)	//采集一次差不多20ms
        { 	
						GPIO_WriteBit(HW_GPIOE, 10, 1);	//测试处理时间
						Get_Speed(&MotorQD);//获取速度
						
						//VGA---10tline---选择QVGA比较稳
						//QVGA---14tline---时间不够---导致跳动
						for(int i=0; i<HIGH_D; i++)
							for(int j=0; j<(WIDTH_D / 8); j++)
								OV7725_IMAGES[i][j] = OV7725_IMAGEB[i][j];
								
						//至此，保证了Current_Rol=0；也就是说明这一块复制在这一事件内
						//解压图像数据，在前一场行终端末到当场上升沿---非常重要
						//本来是在这里进行解压缩数据的，但是时间不够，因此改为了赋值语句
						//必须在赋值语句执行完后，才能进行图像解压缩，不然二维数组解压太耗时间
						Array_ImageDeal(OV7725_IMAGES,OV7725_IMAGE);
						
						//在处理的过程中，Current_Rol是实时跳动的，这表示在处理的同时，行中断也在不断执行
						//处理的是上一个采集的图像，当前也在进行采集图像，为了下一次使用
						//控制电机的必须保证固定周期，应在图像处理前处理，因为图像处理的时间有快有慢
						//舵机响应时间不是很高，对于舵机而言，可以将控制近似为一个固定的周期
						TimeToControlMotor(); //电机控制
						ImageFinshed();	//图像处理
						TimeToControlSteer(); //舵机控制
						BeeperLoopTime(&BeepLoopTime);//蜂鸣器响，方便调试
						Image_Flag = FALSE;	//处理完成，下一图像继续
						
						GPIO_WriteBit(HW_GPIOE, 10, 0);	//测试处理时间
        }
				
    }
}
