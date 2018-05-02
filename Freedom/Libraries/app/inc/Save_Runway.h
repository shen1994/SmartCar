#ifndef _SAVE_RUNWAY_H
#define _SAVE_RUNWAY_H

#include "sd.h"
#include "ff.h"
#include "uart.h"
#include <stdio.h>
#include "Status.h"
#include "ov5116.h"

#define BMP_height 120
#define BMP_width  180

#define ASSERM_SD(rc)     do {if(rc != 0){printf("fatfs error:%d\r\n", rc);}} while(0)

typedef char *BMPdata;  ///传递二维数组
Status Save_Runway_Init(FIL *file_bmp);
Status Save_Runway_BMP(FIL *file_bmp, uint8_t bmp[][BMP_width]);
Status Get_BMP_Name(int count);
Status Save_To_SD(void);
extern FIL File_bmp;

#endif
