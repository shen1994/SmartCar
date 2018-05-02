#include "Auto_Binaryzation.h"
short Threshold_5116=300;

/**
*	@brief DAC初始化
*  DAC0引脚
*	@return  状态
*/
Status DAC_Ref_Init(void)
{

    DAC_InitTypeDef DAC_InitStruct = {0};
    DAC_InitStruct.bufferMode = kDAC_Buffer_Swing; /*Buffer摇摆模式 产生三角波 */
    DAC_InitStruct.instance = HW_DAC0;
    DAC_InitStruct.referenceMode = kDAC_Reference_2; /* 使用VDDA作为参考源 */
    DAC_InitStruct.triggerMode = kDAC_TriggerSoftware; /*软件触发 */
    DAC_Init(&DAC_InitStruct);
    return  OK;

}

/**
*	@brief   阈值设立
*	 范围从0-1024
*	@return  状态
*/
Status DAC_Set_Threshold(uint32_t Threshold)
{
    uint32_t i;
    uint16_t value[16];
    if(Threshold > 1024)
        return ERROR;
    for(i = 0; i < 16; i++)
    {
        value[i] = Threshold * 64 / 16; //数模转换的数据
    }
    DAC_SetBufferValue(HW_DAC0, value, 16); /*写入DAC buffer 最多写入16个 uint16_t 的转换值 */
    return OK;
}
