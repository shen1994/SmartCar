/**
  ******************************************************************************
  * @file    enet.h
  * @author  YANDLD
  * @version V2.5
  * @date    2014.3.26
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com
  * @note    此文件为芯片ENET模块的底层功能函数，具体应用请查看实例程序
  ******************************************************************************
  */

#ifndef __CH_LIB_ENET_H__
#define __CH_LIB_ENET_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    /* MII寄存器地址 */
#define PHY_BMCR                    (0x00) /* Basic Control */
#define PHY_BMSR                    (0x01) /* Basic Status */
#define PHY_PHYIDR1                 (0x02) /* PHY Identifer 1 */
#define PHY_PHYIDR2                 (0x03) /* PHY Identifer 2 */
#define PHY_ANAR                    (0x04) /* Auto-Negotiation Advertisement */
#define PHY_ANLPAR                  (0x05) /* Auto-Negotiation Link Partner Ability */
#define PHY_ANER                    (0x06) /* Auto-Negotiation Expansion */
#define PHY_LPNPA                   (0x07) /* Link Partner Next Page Ability */
#define PHY_RXERC                   (0x15) /* RXER Counter */
#define PHY_ICS                     (0x1B) /* Interrupt Control/Status */
#define PHY_PHYC1                   (0x1E) /* PHY Control 1 */
#define PHY_PHYC2                   (0x1F) /* PHY Control 2 */





    /* PHY_BMCR寄存器位定义 */
#define PHY_BMCR_RESET              (0x8000)
#define PHY_BMCR_LOOP               (0x4000)
#define PHY_BMCR_SPEED              (0x2000)
#define PHY_BMCR_AN_ENABLE          (0x1000)
#define PHY_BMCR_POWERDOWN          (0x0800)
#define PHY_BMCR_ISOLATE            (0x0400)
#define PHY_BMCR_AN_RESTART         (0x0200)
#define PHY_BMCR_FDX                (0x0100)
#define PHY_BMCR_COL_TEST           (0x0080)

    /* PHY_BMSR寄存器位定义 */
#define PHY_BMSR_100BT4             (0x8000)
#define PHY_BMSR_100BTX_FDX         (0x4000)
#define PHY_BMSR_100BTX             (0x2000)
#define PHY_BMSR_10BT_FDX           (0x1000)
#define PHY_BMSR_10BT               (0x0800)
#define PHY_BMSR_NO_PREAMBLE        (0x0040)
#define PHY_BMSR_AN_COMPLETE        (0x0020)
#define PHY_BMSR_REMOTE_FAULT       (0x0010)
#define PHY_BMSR_AN_ABILITY         (0x0008)
#define PHY_BMSR_LINK               (0x0004)
#define PHY_BMSR_JABBER             (0x0002)
#define PHY_BMSR_EXTENDED           (0x0001)

    /* PHY_ANAR寄存器位定义 */
#define PHY_ANAR_NEXT_PAGE          (0x8001)
#define PHY_ANAR_REM_FAULT          (0x2001)
#define PHY_ANAR_PAUSE              (0x0401)
#define PHY_ANAR_100BT4             (0x0201)
#define PHY_ANAR_100BTX_FDX         (0x0101)
#define PHY_ANAR_100BTX             (0x0081)
#define PHY_ANAR_10BT_FDX           (0x0041)
#define PHY_ANAR_10BT               (0x0021)
#define PHY_ANAR_802_3              (0x0001)

    /* PHY_ANLPAR寄存器位定义 */
#define PHY_ANLPAR_NEXT_PAGE        (0x8000)
#define PHY_ANLPAR_ACK              (0x4000)
#define PHY_ANLPAR_REM_FAULT        (0x2000)
#define PHY_ANLPAR_PAUSE            (0x0400)
#define PHY_ANLPAR_100BT4           (0x0200)
#define PHY_ANLPAR_100BTX_FDX       (0x0100)
#define PHY_ANLPAR_100BTX           (0x0080)
#define PHY_ANLPAR_10BTX_FDX        (0x0040)
#define PHY_ANLPAR_10BT             (0x0020)


    /* PHY_PHYSTS寄存器位定义 */
#define PHY_PHYSTS_MDIXMODE         (0x4000)
#define PHY_PHYSTS_RX_ERR_LATCH     (0x2000)
#define PHY_PHYSTS_POL_STATUS       (0x1000)
#define PHY_PHYSTS_FALSECARRSENSLAT (0x0800)
#define PHY_PHYSTS_SIGNALDETECT     (0x0400)
#define PHY_PHYSTS_PAGERECEIVED     (0x0100)
#define PHY_PHYSTS_MIIINTERRUPT     (0x0080)
#define PHY_PHYSTS_REMOTEFAULT      (0x0040)
#define PHY_PHYSTS_JABBERDETECT     (0x0020)
#define PHY_PHYSTS_AUTONEGCOMPLETE  (0x0010)
#define PHY_PHYSTS_LOOPBACKSTATUS   (0x0008)
#define PHY_PHYSTS_DUPLEXSTATUS     (0x0004)
#define PHY_PHYSTS_SPEEDSTATUS      (0x0002)
#define PHY_PHYSTS_LINKSTATUS       (0x0001)

    /* PHY硬件特性 */
#define PHY_STATUS                  ( 0x1F )
#define PHY_DUPLEX_STATUS           ( 4<<2 )
#define PHY_SPEED_STATUS            ( 1<<2 )
    /* PHY收发器硬件地址 */
#define CFG_PHY_ADDRESS             (0x01)

    //Freescale处理器相关定义

    /* TX缓冲区描述符位定义 */
#define TX_BD_R			0x0080
#define TX_BD_TO1		0x0040
#define TX_BD_W			0x0020
#define TX_BD_TO2		0x0010
#define TX_BD_L			0x0008
#define TX_BD_TC		0x0004
#define TX_BD_ABC		0x0002

    /* TX增强型缓冲区描述符位定义 */
#define TX_BD_INT       0x00000040
#define TX_BD_TS        0x00000020
#define TX_BD_PINS      0x00000010
#define TX_BD_IINS      0x00000008
#define TX_BD_TXE       0x00800000
#define TX_BD_UE        0x00200000
#define TX_BD_EE        0x00100000
#define TX_BD_FE        0x00080000
#define TX_BD_LCE       0x00040000
#define TX_BD_OE        0x00020000
#define TX_BD_TSE       0x00010000

#define TX_BD_BDU       0x00000080

    /* RX缓冲区描述符位定义 */
    // 0偏移标志 - 状态:大端格式
#define RX_BD_E			0x0080
#define RX_BD_R01		0x0040
#define RX_BD_W			0x0020
#define RX_BD_R02		0x0010
#define RX_BD_L			0x0008
#define RX_BD_M			0x0001
#define RX_BD_BC		0x8000
#define RX_BD_MC		0x4000
#define RX_BD_LG		0x2000
#define RX_BD_NO		0x1000
#define RX_BD_CR		0x0400
#define RX_BD_OV		0x0200
#define RX_BD_TR		0x0100

    /* RX增强型缓冲区描述符位定义 */
#define RX_BD_ME               0x00000080
#define RX_BD_PE               0x00000004
#define RX_BD_CE               0x00000002
#define RX_BD_UC               0x00000001

#define RX_BD_INT              0x00008000

#define RX_BD_ICE              0x20000000
#define RX_BD_PCR              0x10000000
#define RX_BD_VLAN             0x04000000
#define RX_BD_IPV6             0x02000000
#define RX_BD_FRAG             0x01000000

#define RX_BD_BDU              0x00000080

    /* MII接口超时 */
#define MII_TIMEOUT		0x1FFFF

    /* 以太帧相关定义 */
#define CFG_NUM_ENET_TX_BUFFERS     1     //发送缓冲区个数
#define CFG_NUM_ENET_RX_BUFFERS     1     //接收缓冲区个数  驱动程序设定必须为1了 改了就会出错
#define CFG_ENET_BUFFER_SIZE        1520    //以太发送帧缓冲区长度
#define CFG_ENET_MAX_PACKET_SIZE    1520    //以太发最大数据包长度

    /* 缓冲区描述符结构体 */
    typedef struct
    {
        uint16_t status;	            /* control and status */
        uint16_t length;	            /* transfer length */
        uint8_t  *data;	                /* buffer address */
        uint32_t ebd_status;
        uint16_t length_proto_type;
        uint16_t payload_checksum;
        uint32_t bdu;
        uint32_t timestamp;
        uint32_t reserverd_word1;
        uint32_t reserverd_word2;
    } NBUF;

    //!< 以太网初始化结构
    typedef struct
    {
        uint8_t *pMacAddress;
    } ENET_InitTypeDef;

    typedef enum
    {
        kENET_IT_TXF_Disable,   //!< 禁止发送一帧后产生中断
        kENET_IT_RXF_Disable,   //!< 禁止接收一帧后产生中断
        kENET_IT_TXF,           //!< 开启ENET发送一帧中断
        kENET_IT_RXF,           //!< 开启ENET接收一帧中断
    } ENET_ITDMAConfig_Type;

    //!< ENET CallBack Type
    typedef void (*ENET_CallBackTxType)(void);
    typedef void (*ENET_CallBackRxType)(void);

    //!< API functions
    void ENET_Init(ENET_InitTypeDef *ENET_InitStrut);
    void ENET_MacSendData(uint8_t *data, uint16_t len);
    uint16_t ENET_MacReceiveData(uint8_t *data);
    void ENET_ITDMAConfig(ENET_ITDMAConfig_Type config);
    void ENET_CallbackTxInstall(ENET_CallBackTxType AppCBFun);
    void ENET_CallbackRxInstall(ENET_CallBackRxType AppCBFun);
    uint32_t ENET_IsTransmitComplete(void);

#ifdef __cplusplus
}
#endif



#endif


