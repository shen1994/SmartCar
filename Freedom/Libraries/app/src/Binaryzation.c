/**
*     COPYRIGHT NOTICE
*     All rights reserved.
*
* @file         Binaryzation.c
* @brief        大律法二值化
*
* OTSU算法对图像二值化
*
* @version 1.0
* @author  hejun
* @date    2015年3月7日
*
*
*     修订说明：最初版本
*/
#include "Binaryzation.h"
#include "stdio.h"
#include "string.h"

/**
*	@brief 大律法求图像二值化阈值
*   @param[in] 图像指针,计算起始点,计算范围
*	@return 二值化阈值
*/
int Otsu(uint8_t image[][180], int x0, int y0, int dx, int dy) //高 宽
{
    uint16_t x = x0, y = y0, i, j, threshold = 0;
    uint16_t pixelCount[256] = {0};
    int pixelPro[256] = {0};
    uint32_t pixelSum = dx * dy / 9; //每3点取一点

    //统计灰度级中每个像素在整幅图像中的个数
    for(i = y; i < dx; i += 3)
    {
        for(j = x; j < dy; j += 3)
        {
            pixelCount[image[i][j]]++;
        }
    }


    //计算每个像素在整幅图像中的比例
    for(i = 0; i < 256; i++)
    {
        pixelPro[i] = (pixelCount[i]) * 10000 / (pixelSum);
    }

    //经典ostu算法,得到前景和背景的分割
    //遍历灰度级[0,255],计算出方差最大的灰度值,为最佳阈值
    //前景点占图像比例为w0,均值为u0,背景点占图像比例为w1,均值为u1
    int w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    for(i = 100; i < 220; i++)
    {
        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;

        for(j = 0; j < 256; j++)
        {
            if(j <= i) //背景部分
            {
                //以i为阈值分类，第一类总的概率
                w0 += pixelPro[j];
                u0tmp += j * pixelPro[j];
            }
            else       //前景部分
            {
                //以i为阈值分类，第二类总的概率
                w1 += pixelPro[j];
                u1tmp += j * pixelPro[j];
            }
        }
        u0 = u0tmp / w0;		//第一类的平均灰度
        u1 = u1tmp / w1;		//第二类的平均灰度
        u = u0tmp + u1tmp;		//整幅图像的平均灰度
        //计算类间方差
        deltaTmp = w0 * (u0 - u) * (u0 - u) + w1 * (u1 - u) * (u1 - u);
        //deltaTmp=w0*w1*(u0-u1)/2;
        //找出最大类间方差以及对应的阈值
        if(deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = i;
        }
    }
    //返回最佳阈值;
    return threshold;
}
