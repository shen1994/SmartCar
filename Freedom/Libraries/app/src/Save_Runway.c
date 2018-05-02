/**
*     COPYRIGHT NOTICE
*     All rights reserved.
*
*	@file         Save_Runway.c
*	@brief        保存赛道为BMP格式图片
*
*	保存至SD卡
*
*	@version 1.0
*	@author  hejun
*	@date    2014年11月25日
*
*
*	修订说明：最初版本
*/
#include "Save_Runway.h"
FIL File_bmp;
#define Header_size   0x36    //文件头长度
#define Palette_size  1024    //调色板长度
/**
*	@brief BMP文件二进制格式
*   包含文件头,位图信息头,调色板
*/
const char BmpHeader[Header_size] =
{
    ///BMP文件头
    0x42, 0x4D,             //0x00~0x01,类型"BM"
    0x60, 0x54, 0x00, 0x00, //0x02~0x05,位图文件大小(对齐)每行字节数为4的倍数
    //例80*60 8位灰度图：188*120+54+1024=5878	@需修改   5460
    0x00, 0x00, 0x00, 0x00, //0x06~0x09,保留段 0x00
    0x36, 0x04, 0x00, 0x00, //0x0A~0x0D,文件头开始到图像数据的偏移量  0x36+调色板大小	@需修改

    ///位图信息头
    0x28, 0x00, 0x00, 0x00, //0x0E~0x11,BMP文件头的大小 ：40
    0xB4, 0x00, 0x00, 0x00, //0x12~0x15,宽度:180	@需修改  180 0xB4 188 0xBC
    0x78, 0x00, 0x00, 0x00, //0x16~0x19,高度:120	@需修
    0x01, 0x00,             //0x1A~0x1B,目标设备的级别，必须为1
    0x08, 0x00,             //0x1C~0x1D,每个像素所需的位数:1(双色),4(16色),8(256色)或24(真彩色)
    0x00, 0x00, 0x00, 0x00, //0x1E~0x21,位图压缩类型通常为0：不压缩
    0x00, 0x00, 0x00, 0x00, //0x22~0x25,位图的大小使用BI_RGB，所以设置为0
    0x00, 0x00, 0x00, 0x00, //0x26~0x29,位图水平分辨率，每米像素数打印时需要
    0x00, 0x00, 0x00, 0x00, //0x2A~0x2D,位图垂直分辨率，每米像素数打印室需要
    0x00, 0x01, 0x00, 0x00, //0x2E~0x31,位图实际使用的颜色表中的颜色数：0表示选择所有调色板选项
    0x00, 0x01, 0x00, 0x00, //0x32~0x35,位图显示过程中重要的颜色数：0表示都重要

};

//调色板
char Palette[Palette_size];
char  file_name[22];
/**
*	@brief 生成文件名
*	例: 0:/BMP01.bmp
*	@parma[in] 文件前缀
*	@parma[in] 当前数
*	@return  状态
*/
Status Get_BMP_Name(int count)
{
    //sprintf(file_name, "0:/BMP_%d.bmp", count);
    sprintf(file_name, "0:/TRY_%d.bmp", count);
    return OK;
}


FATFS fs_sd;
FATFS *fs;
DWORD fre_clust, fre_sect, tot_sect;
/**
*	@brief 初始化SD,并挂载文件系统
*	初始化包括硬件SDHC初始化,计算空余空间大小
*	以及文件系统获取空闲空间位置初始化
*	@parma[out] 文件
*	@return  状态
*/
Status Save_Runway_Init(FIL *file_bmp)
{
    if(file_bmp == NULL)
        return ERROR;
    FRESULT rc;
    fs = &fs_sd;
    SD_QuickInit(2 * 1000 * 1000);

    /** 挂载文件系统 */
    rc = f_mount(fs, "0:", 0);
    ASSERM_SD(rc);
    rc = f_getfree("0:", &fre_clust, &fs);
    ASSERM_SD(rc);

    int j = 0;
    for(int k = 0; k < 1024; k += 4)
    {
        Palette[k] = j;
        Palette[k + 1] = j;
        Palette[k + 2] = j;
        Palette[k + 3] = 0;
        j++;
    }
    /** 计算磁盘空间及剩余空间 */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    return OK;
}

/**
*	@brief 保存赛道图像为BMP
*
*	@parma[in] 写入的路径
*	@parma[in] 写入的数组
*	@return  状态
*/
Status Save_Runway_BMP(FIL *file_bmp, uint8_t bmp[][BMP_width])
{
    FRESULT rc;
    UINT Byte_write;

    if(file_bmp == NULL)
        return ERROR;

    /* 写入文件 */
    rc = f_open(file_bmp, file_name, FA_WRITE | FA_CREATE_ALWAYS);
    ASSERM_SD(rc);

    /**写入BMP文件头*/
    rc = f_write(file_bmp, BmpHeader, Header_size, &Byte_write);
    ASSERM_SD(rc);

    /**写入BMP调色板*/
    rc = f_write(file_bmp, Palette, Palette_size, &Byte_write);
    ASSERM_SD(rc);

    /**写入图像数据*/
    int height = 0;
    for(height = BMP_height-1; height >= 0; height--)
    {
        rc = f_write(file_bmp, bmp[height], BMP_width, &Byte_write);
        ASSERM_SD(rc);
    }

    rc = f_close(file_bmp);
    return OK;
}

/**
*	@brief 设置保存时间或其他设置
*	@return  状态
*/
Status Save_To_SD(void)
{
    static int count = 0;
    FRESULT rc;
    do
    {
        count++;
        Get_BMP_Name(count);
        rc = f_open(&File_bmp , file_name, FA_CREATE_NEW | FA_WRITE);
        if(fs_sd.fs_type == 0)
        {
            File_bmp.fs = 0;
            return FALSE;
        }
    }
    while(rc == FR_EXIST);
    Save_Runway_BMP(&File_bmp, MT9V022_IMAGE);
    return OK;
}
