#include "Beeper.h"

/**
*	@brief 蜂鸣器音响
*   @param[in] 响声次数,响声持续时间/33ms,停歇时间/33ms
*	@return 结果状态
*/
Status Beeper(char *Count,char Continous,char Rest)
{
	static char AllDone=1,OnceDone=0,BeepCount=0,BeepContinous=0,BeepRest=0;
	if(0==*Count)
	{
		Buzzer_Off;
		return 0;
	}
	if(AllDone == 1)
	{
		BeepCount = *Count;  
		OnceDone=1;
		AllDone=0;
  }//只在第一次接收幅值
	if(OnceDone == 1)
	{
		BeepContinous=Continous;
		BeepRest=Rest;
		OnceDone=0;
	}//每一个间歇结束后接收赋值
	
	if(0!=BeepContinous)
	{
		Buzzer_On;
		BeepContinous--;
	}//持续是响
	
	if(0==BeepContinous&&0!=BeepRest)
	{
			Buzzer_Off;
			BeepRest--;
	}//持续结束,间歇时不响
	
	if(0==BeepRest)
	{
		OnceDone=1;
		BeepCount--;
	}//持续间歇均结束,刷新赋值,并判断是否到头
	if(0==BeepCount)
	{
		AllDone=1;
		*Count=0;
	}//到头就不响
	return 1;
}

/**
* @breif 持续的图像数
**/
Status BeeperLoopTime(unsigned char* LoopTime){
	static unsigned char KeepLoopTime = 0,OnceTime = 1; 
	if(0 == *LoopTime){
		Buzzer_Off;
		OnceTime = 1;
		return 0;
	}
	if(1 == OnceTime){
		OnceTime = 0;
		KeepLoopTime = *LoopTime;
		Buzzer_On;
	}//第一次的时候赋值
	KeepLoopTime--;
	if(0 == KeepLoopTime){
		*LoopTime = 0;
		Buzzer_Off;
	}
	return 1;
}
