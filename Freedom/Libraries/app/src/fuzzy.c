#include "fuzzy.h"

const float Fuzzy_AD_EMin = -30.0f;
const float Fuzzy_AD_EMax = 30.0f;
const float Fuzzy_AD_DEMin = -12.0f;
const float Fuzzy_AD_DEMax = 12.0f;//12
const float Fuzzy_AD_OUTMin = -2.4f;///-2.4
const float Fuzzy_AD_OUTMax = 3.4f;//3.8
///////////////////////////////////////////////////////
/**
* @brief 存储隶属度
**/
Fuzzy_TypeDef Fuzzy_AD = { 0 };
///////////////////////////////////////////////////////
/**
* @brief 模糊控制规则表
**/
//const unsigned char Fuzzy_AD_Rules[7][7] =
//{
//	//Ec	   
//	//E	        NB  NM  NS  ZO  PS  PM  PB	
//	/*NB -12*/{ NS, PS, ZO, ZO, PB, PB, PB },
//	/*NM  -8*/{ NS, ZO, ZO, ZO, PM, PM, PM },
//	/*NS  -4*/{ NS, ZO, PS, PS, PM, PS, PM },
//	/*ZO   0*/{ PB, PS, ZO, ZO, ZO, PS, PM },
//	/*PS   4*/{ PB, PS, PM, PS, PS, PS, PS },
//	/*PM   8*/{ PM, PM, PM, ZO, ZO, PB, PS },
//	/*PB  12*/{ PB, PB, PB, ZO, PM, PB, PS },
//	//////////-30//-20//-10//0//10//20//30//
//};
//const unsigned char Fuzzy_AD_Rules[7][7] =
//{
//	//Ec	   
//	//E	        NB  NM  NS  ZO  PS  PM  PB	
//	/*NB -12*/{ NS, PS, ZO, ZO, PB, PB, PB },
//	/*NM  -8*/{ NS, ZO, ZO, ZO, PM, PM, PM },
//	/*NS  -4*/{ NS, ZO, PS, PS, PM, PS, PM },
//	/*ZO   0*/{ PB, PS, ZO, ZO, ZO, PS, PM },
//	/*PS   4*/{ PB, PS, PM, PS, PS, PS, PS },
//	/*PM   8*/{ PM, PM, PM, ZO, ZO, PB, PS },
//	/*PB  12*/{ PB, PB, PB, ZO, PM, PB, PS },
//	//////////-30//-20//-10//0//10//20//30//
//};
//const unsigned char Fuzzy_AD_Rules[7][7] =
//{
//	//Ec	   
//	//E	        NB  NM  NS  ZO  PS  PM  PB	
//	/*NB -12*/{ NS, PS, ZO, ZO, PB, PB, PB },
//	/*NM  -8*/{ NS, ZO, ZO, ZO, PM, PM, PM },
//	/*NS  -4*/{ NS, ZO, PS, PS, PM, PM, PM },
//	/*ZO   0*/{ PB, PS, ZO, ZO, PS, PM, PM },
//	/*PS   4*/{ PB, PS, PM, PS, PS, PS, PS },
//	/*PM   8*/{ PM, PM, PM, ZO, ZO, PB, PS },
//	/*PB  12*/{ PB, PB, PB, ZO, PM, PB, PS },
//	//////////-30//-20//-10//0//10//20//30//
//};
const unsigned char Fuzzy_AD_Rules[7][7] =
{
	//Ec	   
	//E	        NB  NM  NS  ZO  PS  PM  PB	
	/*NB -12*/{ NS, ZO, ZO, ZO, PM, PB, PB },
	/*NM  -8*/{ NS, ZO, ZO, ZO, PS, PM, PM },
	/*NS  -4*/{ NS, ZO, PS, PS, PM, PM, PM },
	/*ZO   0*/{ PM, PS, ZO, ZO, PS, PS, PM },
	/*PS   4*/{ PB, PM, PM, PS, PS, PS, PS },	//PM?
	/*PM   8*/{ PM, PM, PM, ZO, ZO, PM, PS },
	/*PB  12*/{ PB, PB, PB, ZO, PM, PB, PS },
	//////////-30//-20//-10//0//10//20//30//
};
///////////////////////////////////////////////////////
/**
* @brief 求最小值
**/
float GetMin(float a, float b){
	if ((a - b) >= 0) return b;
	else return a;
}
///////////////////////////////////////////////////////
/**
* @brief 求最大值
**/
float GetMax(float a, float b){
	if ((a - b) >= 0) return a;
	else return b;
}
///////////////////////////////////////////////////////
/**
* @brief 求平方根---高斯牛顿迭代
**/
float GetSql(float number){
	long i;
	float x, y;
	const float f = 1.5F;
	x = number * 0.5F;
	y = number;
	i = *(long *)&y;
	i = 0x5f3759df - (i >> 1);

	y = *(float *)&i;
	y = y * (f - (x * y * y));
	y = y * (f - (x * y * y));
	return number * y;
}
///////////////////////////////////////////////////////
/**
* @brief:求两个数的平方根
**/
float GetSquareRoot(float a, float b){
	return GetSql(a * a + b * b);
}
///////////////////////////////////////////////////////
/**
* @brief 控制论域中一一对应
**/
void Fuzzy_Value_init(Fuzzy_TypeDef*p)
{
	float k = (p->Out_Area.Max - p->Out_Area.Min) / (Fuzzy_PosSize - 1);
	for (int i = 0; i<Fuzzy_PosSize; i++)
	{
		p->Out_Value[i] = k*i + p->Out_Area.Min;
	}
	p->Err_K = (p->Err_Area.Max - p->Err_Area.Min) / (Fuzzy_PosSize - 1);
	p->DErr_K = (p->DErr_Area.Max - p->DErr_Area.Min) / (Fuzzy_PosSize - 1);
}
///////////////////////////////////////////////////////
/**
* @brief 初始化模糊控制中相关参数
**/
void Fuzzy_init()
{
	Fuzzy_AD.DErr_Area.Max = Fuzzy_AD_DEMax;
	Fuzzy_AD.DErr_Area.Min = Fuzzy_AD_DEMin;
	Fuzzy_AD.Err_Area.Max = Fuzzy_AD_EMax;
	Fuzzy_AD.Err_Area.Min = Fuzzy_AD_EMin;
	Fuzzy_AD.Out_Area.Max = Fuzzy_AD_OUTMax;
	Fuzzy_AD.Out_Area.Min = Fuzzy_AD_OUTMin;
	Fuzzy_AD.Rules = Fuzzy_AD_Rules;
	Fuzzy_Value_init(&Fuzzy_AD);
}
///////////////////////////////////////////////////////
/**
* @brief:初始化隶属度存储数组
**/
void Fuzzy_ClearArray(float *data, int size)
{
	for (int i = 0; i<size; i++)
	{
		data[i] = 0;
	}
}
///////////////////////////////////////////////////////
/**
* @brief 确定隶属度函数，计算e和ec的隶属度
**/
void Fuzzy_Fuzzify(Fuzzy_TypeDef*p)
{
	float err = p->Err;
	float derr = p->DErr;
	float temp, prec;
	int pos;
	Fuzzy_ClearArray(p->Err_Perc, Fuzzy_PosSize);
	Fuzzy_ClearArray(p->DErr_Perc, Fuzzy_PosSize);	//重要，为了查找隶属度
	if (err>p->Err_Area.Max)
		p->Err_Perc[PB] = 1;
	else if (err<p->Err_Area.Min)
		p->Err_Perc[NB] = 1;
	else
	{
		temp = (err - p->Err_Area.Min) / p->Err_K;
		pos = (int)temp;
		prec = temp - (float)(pos);
		p->Err_Perc[pos] = 1 - prec;
		if (pos<Fuzzy_PosSize - 1)
			p->Err_Perc[pos + 1] = prec;
	}
	if (derr>p->DErr_Area.Max)
		p->DErr_Perc[PB] = 1;
	else if (derr<p->DErr_Area.Min)
		p->DErr_Perc[NB] = 1;
	else
	{
		temp = (derr - p->DErr_Area.Min) / p->DErr_K;
		pos = (int)temp;
		prec = temp - (float)(pos);
		p->DErr_Perc[pos] = 1 - prec;
		if (pos<Fuzzy_PosSize - 1)
			p->DErr_Perc[pos + 1] = prec;
	}
}
///////////////////////////////////////////////////////
/**
* @brief 查找模糊控制规则表，确定隶属度，根据控制论域计算控制增量
* @param[in] DealWay---从控制论域到控制增量的几种方式
**/
void Fuzzy_DisFuzzify(Fuzzy_TypeDef* p)
{
	int err_pos1, err_pos2, derr_pos1, derr_pos2;
	int outpos[4];
	float err_perc1, err_perc2, derr_perc1, derr_perc2;

	//float perc[4] = { 0.0f };
	//float OutValue = 0.0f, PercValue = 0.0f;

	for (int i = 0; i<Fuzzy_PosSize - 1; i++)
	{
		if (p->Err_Perc[i] != 0 || i == Fuzzy_PosSize - 2)
		{
			err_pos1 = i;
			err_perc1 = p->Err_Perc[i];
			err_pos2 = i + 1;
			err_perc2 = p->Err_Perc[i + 1];
			break;
		}
	}
	for (int i = 0; i<Fuzzy_PosSize - 1; i++)
	{
		if (p->DErr_Perc[i] != 0 || i == Fuzzy_PosSize - 2)
		{
			derr_pos1 = i;
			derr_perc1 = p->DErr_Perc[i];
			derr_pos2 = i + 1;
			derr_perc2 = p->DErr_Perc[i + 1];
			break;
		}
	}
	outpos[0] = p->Rules[derr_pos1][err_pos1];
	outpos[1] = p->Rules[derr_pos1][err_pos2];
	outpos[2] = p->Rules[derr_pos2][err_pos1];
	outpos[3] = p->Rules[derr_pos2][err_pos2];
	//无道理，未知
	// dy/dt---s  dy---1 卷积---s 对微分作用强
	//0 == DealWay
		p->Out = p->Out_Value[outpos[0]] * derr_perc1*err_perc1 +
			p->Out_Value[outpos[1]] * derr_perc1*err_perc2 +
			p->Out_Value[outpos[2]] * derr_perc2*err_perc1 +
			p->Out_Value[outpos[3]] * derr_perc2*err_perc2;

	//取交集
	//1 == DealWay
	/*
		perc[0] = GetMin(derr_perc1, err_perc1);
		perc[1] = GetMin(derr_perc1, err_perc2);
		perc[2] = GetMin(derr_perc2, err_perc1);
		perc[3] = GetMin(derr_perc2, err_perc2);
		for (int i = 0; i<4; i++){
			OutValue += p->Out_Value[outpos[i]] * perc[i];
			PercValue += perc[i];
		}
		p->Out = OutValue / PercValue;
	*/
		
	//三维空间求点坐标
	//2 == DealWay
	/*
		perc[0] = GetSquareRoot(derr_perc1, err_perc1);
		perc[1] = GetSquareRoot(derr_perc1, err_perc2);
		perc[2] = GetSquareRoot(derr_perc2, err_perc1);
		perc[3] = GetSquareRoot(derr_perc2, err_perc2);
		for (int i = 0; i<4; i++){
			OutValue += p->Out_Value[outpos[i]] * perc[i];
			PercValue += perc[i];
		}
		p->Out = OutValue / PercValue;
	*/
}
///////////////////////////////////////////////////////
/**
* @brief 执行模糊控制
**/
float Fuzzy_Do(float err, float derr, Fuzzy_TypeDef*p)
{
	p->DErr = derr;
	p->Err = err;
	Fuzzy_Fuzzify(p);
	Fuzzy_DisFuzzify(p);
	return p->Out;
}
