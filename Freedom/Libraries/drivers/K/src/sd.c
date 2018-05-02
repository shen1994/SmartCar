/**
  ******************************************************************************
  * @file    sd.c
  * @author  YANDLD
  * @version V2.5
  * @date    2014.3.24
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  * @note    此文件为芯片SD模块的底层功能函数
              只支持SD及SDHC卡 不支持MMC！
  ******************************************************************************
  */

#include "sd.h"
#include "gpio.h"

static uint32_t SD_SendCommand(SD_CommandTypeDef *Command);

//SD卡信息
struct sd_card_handler
{
    uint32_t card_type;
    uint32_t OCR;           //Operation Condition Register 保存电压信息及 初始化完成位 操作条件寄存器 对应真实卡OCR 39:8 位
    uint32_t CID[4];        //Card IDentification (CID) register  CID[0-4] 对应 真实卡CID的 127:8
    uint32_t CSD[4];        //Card-Specific Data register CSD 记录卡容量等重要信息 CSD[0-4] 对应 真实卡CSD 127:8
    uint16_t RCA;           //卡相对地址寄存器 是HOST和卡通讯的基础
    uint32_t CSR[2];        //卡配置寄存器
    uint32_t total_block_cnt;
    uint32_t block_size;
};

static struct sd_card_handler sdh;

/**
 * @brief 设置SD卡模块的通信速度
 * @param  baudrate  单位Hz
 * @retval None
 */
static void SD_SetBaudRate(uint32_t baudrate)
{
    uint32_t pres, div, min, minpres = 0x80, mindiv = 0x0F;
    uint32_t  val, clock;
    CLOCK_GetClockFrequency(kBusClock, &clock);
    //找到相近的分频因子
    min = (uint32_t) - 1;
    for (pres = 2; pres <= 256; pres <<= 1)
    {
        for (div = 1; div <= 16; div++)
        {
            val = pres * div * baudrate - clock;
            if ((val > 0) || (val == 0))
            {
                if (min > val)
                {
                    min = val;
                    minpres = pres;
                    mindiv = div;
                }
            }
        }
    }
    /* disable SDHC */
    SDHC->SYSCTL &= (~ SDHC_SYSCTL_SDCLKEN_MASK);
    /* set prescaler */
    div = SDHC->SYSCTL & (~ (SDHC_SYSCTL_DTOCV_MASK | SDHC_SYSCTL_SDCLKFS_MASK | SDHC_SYSCTL_DVS_MASK));
    SDHC->SYSCTL = div | (SDHC_SYSCTL_DTOCV(0x0E) | SDHC_SYSCTL_SDCLKFS(minpres >> 1) | SDHC_SYSCTL_DVS(mindiv - 1));
    /* waitting for stabile */
    while (0 == (SDHC->PRSSTAT & SDHC_PRSSTAT_SDSTB_MASK));
    /* enable module */
    SDHC->SYSCTL |= SDHC_SYSCTL_SDCLKEN_MASK;
    SDHC->IRQSTAT |= SDHC_IRQSTAT_DTOE_MASK;
}

/**
 * @brief 检测内存卡的运行状态
 * @note  内部函数，用户无需调用
 * @retval 0:未运行，1:运行
 */
uint8_t SDHC_is_running(void)
{
    return (0 != (SDHC->PRSSTAT & (SDHC_PRSSTAT_RTA_MASK | SDHC_PRSSTAT_WTA_MASK | SDHC_PRSSTAT_DLA_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_CIHB_MASK)));
}

/**
 * @brief SD模块快速初始化配置
 * @param  baudrate  :通信波特率
 * @retval 0:正常  其它:未完成初始化
 */
uint32_t SD_QuickInit(uint32_t baudrate)
{
    uint32_t retry = 0;
    SD_InitTypeDef SD_InitStruct1;
    SD_InitStruct1.baudrate = baudrate;
    /* init pinmux */
    PORT_PinMuxConfig(HW_GPIOE, 0, kPinAlt4); /* ESDHC.D1  */
    PORT_PinMuxConfig(HW_GPIOE, 1, kPinAlt4); /* ESDHC.D0  */
    PORT_PinMuxConfig(HW_GPIOE, 2, kPinAlt4); /* ESDHC.CLK */
    PORT_PinMuxConfig(HW_GPIOE, 3, kPinAlt4); /* ESDHC.CMD */
    PORT_PinMuxConfig(HW_GPIOE, 4, kPinAlt4); /* ESDHC.D3  */
    PORT_PinMuxConfig(HW_GPIOE, 5, kPinAlt4); /* ESDHC.D2  */
    PORT_PinPullConfig(HW_GPIOE, 0, kPullUp);
    PORT_PinPullConfig(HW_GPIOE, 1, kPullUp);
    PORT_PinPullConfig(HW_GPIOE, 3, kPullUp);
    PORT_PinPullConfig(HW_GPIOE, 4, kPullUp);
    PORT_PinPullConfig(HW_GPIOE, 5, kPullUp);
#if 0
    PORTE->PCR[0] =  (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D1  */
    PORTE->PCR[1] =  (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D0  */
    PORTE->PCR[2] =  (PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK);                                          /* ESDHC.CLK */
    PORTE->PCR[3] =  (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.CMD */
    PORTE->PCR[4] =  (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D3  */
    PORTE->PCR[5] =  (PORT_PCR_MUX(4) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_DSE_MASK);    /* ESDHC.D2  */
#endif
    while(SD_Init(&SD_InitStruct1) && (retry < 10))
    {
        retry++;
    }
    if(retry == 10)
    {
        return retry;
    }
    return 0;
}

/**
 * @brief SD模块初始化配置，4线传输
 * @note  仅仅完成SD模块的初始化配置，需其它函数配合
 * @param  SD_InitStruct  :SD模块设置结构体
 * @retval ESDHC_OK:正常  ESDHC_ERROR_INIT_FAILED:未完成初始化
 */
uint8_t SD_Init(SD_InitTypeDef *SD_InitStruct)
{
    uint32_t delay_cnt = 0;
    uint8_t result;  //存储函数返回结果
    uint8_t i = 0;
    uint8_t hc;     //是否为SDHC标志
    SD_CommandTypeDef SD_CommandStruct1;
    SIM->SCGC3 |= SIM_SCGC3_SDHC_MASK;
    //复位SDHC 设置超时时间
    SDHC->SYSCTL = SDHC_SYSCTL_RSTA_MASK | SDHC_SYSCTL_SDCLKFS(0x80);
    //等待复位完成
    while(SDHC->SYSCTL & SDHC_SYSCTL_RSTA_MASK) {};
    //初始化SDHC相关寄存器
    SDHC->VENDOR = 0;
    SDHC->BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(512); //默认传输1个Block 每个Block 512字节
    SDHC->PROCTL = SDHC_PROCTL_EMODE(2) | SDHC_PROCTL_D3CD_MASK;       //LSB格式   使用DATA3 检测卡插入移除
    SDHC->WML = SDHC_WML_RDWML(1) | SDHC_WML_WRWML(1);                 //这个不懂。。
    //设置SDHC模块的通信速率
    SD_SetBaudRate(SD_InitStruct->baudrate);
    while (SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK)) ;
    //清除SDHC模块的中断标志
    SDHC->IRQSTAT = 0xFFFF;
    /* enable irq status */
    SDHC->IRQSTATEN = SDHC_IRQSTATEN_DEBESEN_MASK
                      | SDHC_IRQSTATEN_DCESEN_MASK
                      | SDHC_IRQSTATEN_DTOESEN_MASK
                      | SDHC_IRQSTATEN_CIESEN_MASK
                      | SDHC_IRQSTATEN_CEBESEN_MASK
                      | SDHC_IRQSTATEN_CCESEN_MASK
                      | SDHC_IRQSTATEN_CTOESEN_MASK
                      | SDHC_IRQSTATEN_BRRSEN_MASK
                      | SDHC_IRQSTATEN_BWRSEN_MASK
                      | SDHC_IRQSTATEN_CRMSEN_MASK
                      | SDHC_IRQSTATEN_TCSEN_MASK
                      | SDHC_IRQSTATEN_CCSEN_MASK;
    for(delay_cnt = 0; delay_cnt < 10000; delay_cnt++) {};
    //80个时钟周期的初始化
    SDHC->SYSCTL |= SDHC_SYSCTL_INITA_MASK;
    while (SDHC->SYSCTL & SDHC_SYSCTL_INITA_MASK) {}; //等待初始化完成
    //--------------以下开始SD卡初始化 物理层协议---------------------------
    //开始SD卡初始化进程 --------------------------------
    //说明 CMD0 -> CMD8 -> while(CMD55+ACMD41) ->CMD2 -> CMD3 ->CMD9
    //            -> CMD7(选中卡)-> CMD16(设置块大小)->(CMD55+ACMD6)设置位4线位宽
    //---------------------------正式开始------------------------------  now Let's begin !
    //CMD0  使所有卡进入IDLE
    SD_CommandStruct1.COMMAND = ESDHC_CMD0;
    SD_CommandStruct1.ARGUMENT = 0;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;      //错误则返回

    //CMD8  判断是V1.0还是V2.0的卡
    SD_CommandStruct1.COMMAND = ESDHC_CMD8;
    SD_CommandStruct1.ARGUMENT = 0x000001AA;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if (result > 0)  //CMD8无响应  错误或者普通卡
    {
        result = ESDHC_ERROR_INIT_FAILED;
    }
    if (result == 0) //SDHC 卡
    {
        hc = true;
    }
    //反复发送55+ACDM41 直到卡准备好
    do
    {
        //延时
        for(delay_cnt = 0; delay_cnt < 10000; delay_cnt++);
        i++;
        SD_CommandStruct1.COMMAND = ESDHC_CMD55;
        SD_CommandStruct1.ARGUMENT = 0;
        SD_CommandStruct1.BLOCKS = 0;
        result = SD_SendCommand(&SD_CommandStruct1);

        SD_CommandStruct1.COMMAND = ESDHC_ACMD41;
        if(hc)
        {
            SD_CommandStruct1.ARGUMENT = 0x40300000;
        }
        else
        {
            SD_CommandStruct1.ARGUMENT = 0x00300000;
        }
        result = SD_SendCommand(&SD_CommandStruct1);
    }
    while ((0 == (SD_CommandStruct1.RESPONSE[0] & 0x80000000)) && (i < 30));
    //CMD2 取CID
    SD_CommandStruct1.COMMAND = ESDHC_CMD2;
    SD_CommandStruct1.ARGUMENT = 0;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;
    sdh.CID[0] = SD_CommandStruct1.RESPONSE[0];
    sdh.CID[1] = SD_CommandStruct1.RESPONSE[1];
    sdh.CID[2] = SD_CommandStruct1.RESPONSE[2];
    sdh.CID[3] = SD_CommandStruct1.RESPONSE[3];

    //CMD3 取RCA
    SD_CommandStruct1.COMMAND = ESDHC_CMD3;
    SD_CommandStruct1.ARGUMENT = 0;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;
    sdh.RCA = SD_CommandStruct1.RESPONSE[0] >> 16;
    //CMD9 取CSD
    SD_CommandStruct1.COMMAND = ESDHC_CMD9;
    SD_CommandStruct1.ARGUMENT = sdh.RCA << 16;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;
    sdh.CSD[0] = SD_CommandStruct1.RESPONSE[0];
    sdh.CSD[1] = SD_CommandStruct1.RESPONSE[1];
    sdh.CSD[2] = SD_CommandStruct1.RESPONSE[2];
    sdh.CSD[3] = SD_CommandStruct1.RESPONSE[3];

    //CMD7 选中卡
    SD_CommandStruct1.COMMAND = ESDHC_CMD7;
    SD_CommandStruct1.ARGUMENT = sdh.RCA << 16;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;
    //CMD16 设置块大小
    SD_CommandStruct1.COMMAND = ESDHC_CMD16;
    SD_CommandStruct1.ARGUMENT = 512;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;

    //CMD55 使用ACMD命令
    SD_CommandStruct1.COMMAND = ESDHC_CMD55;
    SD_CommandStruct1.ARGUMENT = sdh.RCA << 16;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;
    //ACMD6 修改SD卡通讯位宽
    SD_CommandStruct1.COMMAND = ESDHC_ACMD6;
    SD_CommandStruct1.ARGUMENT = 2;
    SD_CommandStruct1.BLOCKS = 0;
    result = SD_SendCommand(&SD_CommandStruct1);//修改SD卡位4位通讯位宽
    if(result != ESDHC_OK) return ESDHC_ERROR_INIT_FAILED;
    //设置 Kinetis的 SDIO模块位4线模式
    SDHC->PROCTL &= (~SDHC_PROCTL_DTW_MASK);
    SDHC->PROCTL |=  SDHC_PROCTL_DTW(ESDHC_PROCTL_DTW_4BIT);
    //判断卡类型
    if((sdh.CSD[3] >> 22) & 0x03)
    {
        sdh.card_type = SD_CARD_TYPE_SDHC;
    }
    else
    {
        sdh.card_type = SD_CARD_TYPE_SD;
    }
    //开启卡插入检测，卡移除中断----未成功
    /*
    NVIC_EnableIRQ(SDHC_IRQn);  //开启CM4核上的中断线
    SDHC->IRQSTATEN |= SDHC_IRQSTATEN_CINTSEN_MASK | SDHC_IRQSTATEN_CRMSEN_MASK | SDHC_IRQSTATEN_CINSEN_MASK; //开启SDHC中断卡插入 卡移除
    SDHC->IRQSIGEN |=SDHC_IRQSIGEN_CINTIEN_MASK | SDHC_IRQSIGEN_CINSIEN_MASK | SDHC_IRQSIGEN_CRMIEN_MASK;
    */
    //顺利初始化操作
    return ESDHC_OK;
}

/**
 * @brief 读SD卡的一个扇区
 * @note  一个扇区至少为512字节
 * @param  sector  :要读取的SD卡扇区号
 * @param  buffer  :数据存储地址
 * @retval ESDHC_OK:正常  其它:读取错误
 */
uint8_t SD_ReadSingleBlock(uint32_t sector, uint8_t *buffer)
{
    uint16_t results;
    uint32_t	j;
    uint32_t	*ptr = (uint32_t *)buffer;
    SD_CommandTypeDef SD_CommandStruct1;
    if(sdh.card_type == SD_CARD_TYPE_SD) //如果是普通SD卡 把块地址转换成字节地址
    {
        sector = sector << 9;
    }
    while (SDHC->PRSSTAT & SDHC_PRSSTAT_DLA_MASK) {}; //等待DATA线空闲
    //SDHC->BLKATTR &= (~ SDHC_BLKATTR_BLKCNT_MASK); //清除快数
    //SDHC->BLKATTR |= SDHC_BLKATTR_BLKCNT(1);
    SD_CommandStruct1.COMMAND = ESDHC_CMD17;
    SD_CommandStruct1.ARGUMENT = sector;
    SD_CommandStruct1.BLOCKS = 1;
    SD_CommandStruct1.BLOCKSIZE = 512;
    results = SD_SendCommand(&SD_CommandStruct1);
    if(results != ESDHC_OK) return ESDHC_ERROR_DATA_TRANSFER;
    //开始读取一个扇区------------------------------
    //读取数据时，每次读取4个字节
    for (j = (512 + 3) >> 2; j != 0; j--)
    {
        //检测错误 有错误则退出
        if (SDHC->IRQSTAT & (   SDHC_IRQSTAT_DEBE_MASK //Data End Bit Error
                                | SDHC_IRQSTAT_DCE_MASK  //Data CRC Error
                                | SDHC_IRQSTAT_DTOE_MASK)) //DataTimeout Error
        {
            SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK
                             | SDHC_IRQSTAT_DCE_MASK
                             | SDHC_IRQSTAT_DTOE_MASK
                             | SDHC_IRQSTAT_BRR_MASK; //Buffer Read Ready
            return ESDHC_ERROR_DATA_TRANSFER;
        }
        //等待数据准备好
        while (0 == (SDHC->PRSSTAT & SDHC_PRSSTAT_BREN_MASK)) {};
        *ptr = SDHC->DATPORT;
        ptr++;	//这里取代 *ptr++=SDHC->DATPORT;	 因为这句有BUG

    }
    return ESDHC_OK;
}

/**
 * @brief 写SD卡的一个扇区
 * @note  一个扇区至少为512字节
 * @param  sector  :要写入的SD卡扇区号
 * @param  buffer  :数据存储地址
 * @retval ESDHC_OK:正常  其它:读取错误
 */
uint8_t SD_WriteSingleBlock(uint32_t sector, const uint8_t *buffer)
{
    uint16_t results;
    uint32_t	j;
    uint32_t	*ptr = (uint32_t *)buffer;
    SD_CommandTypeDef SD_CommandStruct1;
    if(sdh.card_type == SD_CARD_TYPE_SD) //如果是普通SD卡 把块地址转换成字节地址
    {
        sector = sector << 9;
    }
    while (SDHC->PRSSTAT & SDHC_PRSSTAT_DLA_MASK) {}; //等待DATA线空闲
    //SDHC->BLKATTR &= (~ SDHC_BLKATTR_BLKCNT_MASK); //清除快数
    //SDHC->BLKATTR |= SDHC_BLKATTR_BLKCNT(1);
    SD_CommandStruct1.COMMAND = ESDHC_CMD24;
    SD_CommandStruct1.ARGUMENT = sector;
    SD_CommandStruct1.BLOCKS = 1;
    SD_CommandStruct1.BLOCKSIZE = 512;
    results = SD_SendCommand(&SD_CommandStruct1);

    if(results != ESDHC_OK) return ESDHC_ERROR_DATA_TRANSFER;

    //开始写入一个扇区------------------------------

    //读取数据时，每次读取4个字节
    for (j = (512) >> 2; j != 0; j--)
    {
        if (SDHC->IRQSTAT & (  SDHC_IRQSTAT_DEBE_MASK //Data End Bit Error
                               | SDHC_IRQSTAT_DCE_MASK  //Data CRC Error
                               | SDHC_IRQSTAT_DTOE_MASK)) //DataTimeout Error

        {
            SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK
                             | SDHC_IRQSTAT_DCE_MASK
                             | SDHC_IRQSTAT_DTOE_MASK
                             | SDHC_IRQSTAT_BWR_MASK; //Buffer Write Ready
            return ESDHC_ERROR_DATA_TRANSFER;
        }
        while (0 == (SDHC->PRSSTAT & SDHC_PRSSTAT_BWEN_MASK)) {}; //等待数据准备好
        SDHC->DATPORT = *ptr;
        ptr++;	//这里取代 *ptr++=SDHC->DATPORT;	 因为这句有BUG
    }

    if (SDHC->IRQSTAT & (SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK))
    {
        SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK;
        results = ESDHC_ERROR_DATA_TRANSFER;
    }
    SDHC->IRQSTAT |= SDHC_IRQSTAT_TC_MASK | SDHC_IRQSTAT_BRR_MASK | SDHC_IRQSTAT_BWR_MASK;
    //等待卡准备好，传输状态
    do
    {
        SD_CommandStruct1.COMMAND = ESDHC_CMD13;
        SD_CommandStruct1.ARGUMENT = sdh.RCA;
        SD_CommandStruct1.BLOCKS = 0;
        results = SD_SendCommand(&SD_CommandStruct1);
        if(results != ESDHC_OK) return ESDHC_ERROR_DATA_TRANSFER;

        if (SD_CommandStruct1.RESPONSE[0] & 0xFFD98008)
        {
            //count = 0; // necessary to get real number of written blocks
            break;
        }

    }
    while (0x000000900 != (SD_CommandStruct1.RESPONSE[0] & 0x00001F00));

    return ESDHC_OK;
}

/**
 * @brief 获得SD卡容量
 * @retval SD卡容量，单位MB
 */
uint32_t SD_GetSizeInMB(void)
{
    uint32_t BlockBumber;  //快个数
    uint32_t Muti;         //乘数
    uint32_t BlockLen;     //每块长度
    uint32_t Capacity;     //容量
    //计算乘数
    if((sdh.CSD[3] >> 22) & 0x03)
    {
        //------------------------------------------------------------
        //CSD V2.00版本(SDHC卡)
        //卡容量计算公式
        //memory capacity = (C_SIZE+1) * 512K byte
        //------------------------------------------------------------
        BlockLen = (sdh.CSD[2] >> 24) & 0xFF;
        Capacity = ((sdh.CSD[1] >> 8) & 0xFFFFFF) + 1;
        Capacity = (Capacity + 1) / 2;
        return Capacity;
    }
    else
    {
        /*
        CSD V1.00版本(普通SD卡)
        卡容量计算公式  BLOCKNR = (C_SIZE+1) * MULT
        MULT = 2^(C_SIZE_MULT+2)
        BLOCK_LEN = 2^((C_SIZE_MULT < 8) )
         容量=BLOCKNR*BLOCK_LEN
        */
        Muti = (sdh.CSD[1] >> 7) & 0x7;
        Muti = 2 << (Muti + 1);
        //计算块数
        BlockBumber = ((sdh.CSD[2] >> 0) & 0x03);
        BlockBumber = (BlockBumber << 10) + ((sdh.CSD[1] >> 22) & 0x0FFF);
        BlockBumber++;
        BlockBumber = BlockBumber * Muti; //得到快数
        BlockLen = (sdh.CSD[2] >> 8) & 0x0F; //得到每块大小
        BlockLen = 2 << (BlockLen - 1);
        Capacity = BlockBumber * BlockLen; //计算容量 单位Byte
        Capacity = Capacity / 1024 / 1024; //单位MB
        return Capacity;
    }
}

/**
 * @brief 检测IRQSTAT寄存器状态
 * @note  内部函数
 * @retval SD模块的中断状态
 */
uint32_t SD_StatusWait (uint32_t  mask)
{
    uint32_t result;
    do
    {
        result = SDHC->IRQSTAT & mask;
    }
    while (0 == result);
    return result;
}

/**
 * @brief 向SD卡发送命令
 * @note  内部函数
 * @param  Command  :SD卡命令结构参数
 * @retval ESDHC_OK:正常  其它:错误
 */
static uint32_t SD_SendCommand(SD_CommandTypeDef *Command)
{
    uint32_t xfertyp;
    uint32_t blkattr;

    xfertyp = Command->COMMAND;
    if (ESDHC_XFERTYP_CMDTYP_RESUME == ((xfertyp & SDHC_XFERTYP_CMDTYP_MASK) >> SDHC_XFERTYP_CMDTYP_SHIFT))
    {
        /* 恢复命令必须设置DPSEL位 */
        xfertyp |= SDHC_XFERTYP_DPSEL_MASK;
    }
    if (Command->BLOCKS > 1)
    {
        xfertyp |= SDHC_XFERTYP_DPSEL_MASK;
        blkattr = SDHC_BLKATTR_BLKSIZE(Command->BLOCKSIZE) | SDHC_BLKATTR_BLKCNT(Command->BLOCKS);
        /* 准许读取BlockCnt */
        xfertyp |= SDHC_XFERTYP_BCEN_MASK;
        if (Command->BLOCKS > 1)
        {
            //多块传输
            xfertyp |= SDHC_XFERTYP_MSBSEL_MASK;
        }
    }
    else if(Command->BLOCKS == 1)
    {
        blkattr = SDHC_BLKATTR_BLKSIZE(512) | SDHC_BLKATTR_BLKCNT(1);
    }
    else
    {
        blkattr = SDHC_BLKATTR_BLKSIZE(512) | SDHC_BLKATTR_BLKCNT(0);
    }
    //清除卡移除状态
    SDHC->IRQSTAT |= SDHC_IRQSTAT_CRM_MASK;
    //等待CMD空闲
    while (SDHC->PRSSTAT & SDHC_PRSSTAT_CIHB_MASK) {};
    //初始化命令
    SDHC->CMDARG = Command->ARGUMENT;
    SDHC->BLKATTR = blkattr;
    SDHC->DSADDR = 0;
    //发送命令
    SDHC->XFERTYP = xfertyp;
    //等待相应
    if (SD_StatusWait (SDHC_IRQSTAT_CIE_MASK | SDHC_IRQSTAT_CEBE_MASK | SDHC_IRQSTAT_CCE_MASK | SDHC_IRQSTAT_CC_MASK) != SDHC_IRQSTAT_CC_MASK)
    {
        SDHC->IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CIE_MASK | SDHC_IRQSTAT_CEBE_MASK | SDHC_IRQSTAT_CCE_MASK | SDHC_IRQSTAT_CC_MASK;
        return ESDHC_ERROR_COMMAND_FAILED;
    }

    /* if card removed */
    if (SDHC->IRQSTAT & SDHC_IRQSTAT_CRM_MASK)
    {
        SDHC->IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CC_MASK;
        return ESDHC_ERROR_COMMAND_FAILED;
    }
    /* command time out */
    if (SDHC->IRQSTAT & SDHC_IRQSTAT_CTOE_MASK)
    {
        SDHC->IRQSTAT |= SDHC_IRQSTAT_CTOE_MASK | SDHC_IRQSTAT_CC_MASK;
        return ESDHC_ERROR_COMMAND_TIMEOUT;
    }
    //如果发送的是有回应的命令
    if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) != SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_NO))
    {
        Command->RESPONSE[0] = SDHC->CMDRSP[0];
        //如果接受到的是136字节的长响应
        if ((xfertyp & SDHC_XFERTYP_RSPTYP_MASK) == SDHC_XFERTYP_RSPTYP(ESDHC_XFERTYP_RSPTYP_136))
        {
            Command->RESPONSE[1] = SDHC->CMDRSP[1];
            Command->RESPONSE[2] = SDHC->CMDRSP[2];
            Command->RESPONSE[3] = SDHC->CMDRSP[3];
        }
    }
    SDHC->IRQSTAT |= SDHC_IRQSTAT_CC_MASK;
    return ESDHC_OK;
}

/**
 * @brief 读SD卡的多个扇区
 * @note  一个扇区至少为512字节
 * @param  sector  :要读取的SD卡扇区号
 * @param  buffer  :数据存储地址
 * @param  count   :连续读取的扇区数量
 * @retval ESDHC_OK:正常  其它:读取错误
 */
uint8_t SD_ReadMultiBlock(uint32_t sector, uint8_t *pbuffer, uint16_t count)
{
    uint32_t i, j;
    uint16_t results;
    uint32_t *ptr = (uint32_t *)pbuffer;
    SD_CommandTypeDef SD_CommandStruct1;
    if(sdh.card_type  == SD_CARD_TYPE_SD) //如果是普通SD卡 把块地址转换成字节地址
    {
        sector = sector << 9;
    }
    SD_CommandStruct1.COMMAND = ESDHC_CMD18;
    SD_CommandStruct1.BLOCKS = count;
    SD_CommandStruct1.BLOCKSIZE = 512;
    SD_CommandStruct1.ARGUMENT = sector;
    results = SD_SendCommand(&SD_CommandStruct1);
    if(results != ESDHC_OK) return ESDHC_ERROR_DATA_TRANSFER;

    for(i = 0; i < count; i++)
    {
        if (((uint32_t)pbuffer & 0x03) == 0)
        {
            for (j = (512 + 3) >> 2; j != 0; j--)
            {
                //检测错误 有错误则退出
                if (SDHC->IRQSTAT & (   SDHC_IRQSTAT_DEBE_MASK /* data end bit error */
                                        | SDHC_IRQSTAT_DCE_MASK  /* data CRC error */
                                        | SDHC_IRQSTAT_DTOE_MASK)) /* data timeout error */
                {
                    SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK
                                     | SDHC_IRQSTAT_DCE_MASK
                                     | SDHC_IRQSTAT_DTOE_MASK
                                     | SDHC_IRQSTAT_BRR_MASK; /* buffer read ready */
                    return ESDHC_ERROR_DATA_TRANSFER;
                }
                while ((0 == (SDHC->PRSSTAT & SDHC_PRSSTAT_BREN_MASK)) && (SDHC->PRSSTAT & SDHC_PRSSTAT_DLA_MASK)) {};
                *ptr = SDHC->DATPORT;
                ptr++;
            }
        }
    }

    //等待传输结束
    SD_StatusWait (SDHC_IRQSTAT_TC_MASK);
    if (SDHC->IRQSTAT & (SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK))
    {
        SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK;
        results = ESDHC_ERROR_DATA_TRANSFER;
    }
    SDHC->IRQSTAT |= SDHC_IRQSTAT_TC_MASK | SDHC_IRQSTAT_BRR_MASK | SDHC_IRQSTAT_BWR_MASK;
    //等待卡准备好，传输状态
    do
    {
        SD_CommandStruct1.COMMAND = ESDHC_CMD13;
        SD_CommandStruct1.ARGUMENT = sdh.RCA;
        SD_CommandStruct1.BLOCKS = 0;
        results = SD_SendCommand(&SD_CommandStruct1);
        if(results != ESDHC_OK)
        {
            return ESDHC_ERROR_DATA_TRANSFER;
        }
        if (SD_CommandStruct1.RESPONSE[0] & 0xFFD98008)
        {
            count = 0; /* necessary to get real number of written blocks */
            break;
        }

    }
    while (0x000000900 != (SD_CommandStruct1.RESPONSE[0] & 0x00001F00));
    return ESDHC_OK;
}

/**
 * @brief 写SD卡的多个扇区
 * @note  一个扇区至少为512字节
 * @param  sector  :要写入的SD卡扇区号
 * @param  buffer  :数据存储地址
 * @param  count   :连续写入的扇区数
 * @retval ESDHC_OK:正常  其它:读取错误
 */
uint8_t SD_WriteMultiBlock(uint32_t sector, const uint8_t *pbuffer, uint16_t count)
{
    uint32_t i, j;
    uint16_t results;
    uint32_t	*ptr = (uint32_t *)pbuffer;
    SD_CommandTypeDef SD_CommandStruct1;
    if(sdh.card_type  == SD_CARD_TYPE_SD) //如果是普通SD卡 把块地址转换成字节地址
    {
        sector = sector << 9;
    }
    SD_CommandStruct1.COMMAND = ESDHC_CMD25;
    SD_CommandStruct1.BLOCKS = count;
    SD_CommandStruct1.BLOCKSIZE = 512;
    SD_CommandStruct1.ARGUMENT = sector;
    results = SD_SendCommand(&SD_CommandStruct1);
    if(results != ESDHC_OK)
    {
        return ESDHC_ERROR_DATA_TRANSFER;
    }

    //开始传送数据
    for(i = 0; i < count; i++)
    {
        for (j = (512) >> 2; j != 0; j--)
        {
            if (SDHC->IRQSTAT & (  SDHC_IRQSTAT_DEBE_MASK //Data End Bit Error
                                   | SDHC_IRQSTAT_DCE_MASK  //Data CRC Error
                                   | SDHC_IRQSTAT_DTOE_MASK)) //DataTimeout Error

            {
                SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK
                                 | SDHC_IRQSTAT_DCE_MASK
                                 | SDHC_IRQSTAT_DTOE_MASK
                                 | SDHC_IRQSTAT_BWR_MASK; //Buffer Write Ready
                return ESDHC_ERROR_DATA_TRANSFER;
            }
            while (0 == (SDHC->PRSSTAT & SDHC_PRSSTAT_BWEN_MASK)) {}; //等待数据准备好
            SDHC->DATPORT = *ptr;
            ptr++;	//这里取代 *ptr++=SDHC->DATPORT;	 因为这句有BUG
        }
    }

    SD_StatusWait (SDHC_IRQSTAT_TC_MASK);

    if (SDHC->IRQSTAT & (SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK))
    {
        SDHC->IRQSTAT |= SDHC_IRQSTAT_DEBE_MASK | SDHC_IRQSTAT_DCE_MASK | SDHC_IRQSTAT_DTOE_MASK;
        results = ESDHC_ERROR_DATA_TRANSFER;
    }
    SDHC->IRQSTAT |= SDHC_IRQSTAT_TC_MASK | SDHC_IRQSTAT_BRR_MASK | SDHC_IRQSTAT_BWR_MASK;
    //等待卡准备好，传输状态
    do
    {
        SD_CommandStruct1.COMMAND = ESDHC_CMD13;
        SD_CommandStruct1.ARGUMENT = sdh.RCA;
        SD_CommandStruct1.BLOCKS = 0;
        results = SD_SendCommand(&SD_CommandStruct1);
        if(results != ESDHC_OK) return ESDHC_ERROR_DATA_TRANSFER;

        if (SD_CommandStruct1.RESPONSE[0] & 0xFFD98008)
        {
            count = 0; // necessary to get real number of written blocks
            break;
        }

    }
    while (0x000000900 != (SD_CommandStruct1.RESPONSE[0] & 0x00001F00));

    return ESDHC_OK;
}



