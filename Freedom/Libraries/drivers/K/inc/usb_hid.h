#ifndef __USB_HID_H__
#define __USB_HID_H__
/***********************************************************************************************
//CH_Kinetis驱动库  V2.3
//作者    :YANDLD
//E-MAIL  :yandld@126.com
//修改日期:2013/2/14
//版本：V2.3
//淘宝：http://upcmcu.taobao.com
//QQ    1453363089
//Copyright(C) YANDLD 2012-2022
//All rights reserved
************************************************************************************************/
//外部数据接口
#include <stdint.h>
//本构件实现的接口函数
void USB_HID_SetMouse(uint8_t HOffset, uint8_t VOffset, uint8_t SOffset, uint8_t Key);
void HID_Proc(void);
void USB_HID_SendData(uint8_t *buf, uint8_t len);
uint8_t USB_HID_SetKeyBoard(uint8_t FnKey, uint8_t *Keybuf);
uint8_t USB_HID_RecData(uint8_t *buf);
#endif
