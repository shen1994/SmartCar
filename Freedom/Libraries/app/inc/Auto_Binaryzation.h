#ifndef _Auto_binaryzation_H
#define _Auto_binaryzation_H
#include "DAC.h"
#include "Status.h"

Status DAC_Ref_Init(void);
Status DAC_Set_Threshold(uint32_t Threshold);
extern short Threshold_5116;
#endif

