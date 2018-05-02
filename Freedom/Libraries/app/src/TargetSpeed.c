#include "TargetSpeed.h"

const float TargetSpeed_err_min = 0.0f;
const float TargetSpeed_err_max = 120.0f;	//不小于10.0f
const float TargetSpeed_derr_min = 0.0f;
const float TargetSpeed_derr_max = 50.0f;	//不小于10.0f
const float TargetSpeed_out_min = -5.0f;
const float TargetSpeed_out_max =  5.0f;

TargetSpeed_Typedef TartgetSpeed_Reference = { 0 };

/*模糊规则表*/
const unsigned char TargetSpeed_rules[TartgetSpeed_FuzzyNum][TartgetSpeed_FuzzyNum] = {
//Err 0 38 68 82 112 150
//DErr 0 11 19 21 29 40
	{ SpeedPB, SpeedPB, SpeedPB, SpeedPS, SpeedPS },
	{ SpeedPS, SpeedPS, SpeedPS, SpeedPS, SpeedPS },
	{ SpeedPS, SpeedPS, SpeedZO, SpeedZO, SpeedZO },
	{ SpeedNS, SpeedNS, SpeedNS, SpeedNS, SpeedNS },
	{ SpeedNB, SpeedNB, SpeedNB, SpeedNB, SpeedNB },
};

/**
* @breif 模糊初始化
**/
void TargetSpeed_FuzzyInit(TargetSpeed_Typedef *Ptr){
	Ptr->GoRules = TargetSpeed_rules;
	Ptr->GoErr.min = TargetSpeed_err_min;
	Ptr->GoErr.max = TargetSpeed_err_max;
	Ptr->GoDerr.min = TargetSpeed_derr_min;
	Ptr->GoDerr.max = TargetSpeed_derr_max;
	Ptr->GoOut.min = TargetSpeed_out_min;
	Ptr->GoOut.max = TargetSpeed_out_max;
	TargetSpeed_FuzzyExpand(Ptr);
}

/**
* @breif 输入量模糊化
**/
void TargetSpeed_FuzzyExpand(TargetSpeed_Typedef *Ptr){
	//重叠系数 = 重叠范围/隶属度函数范围 （0.2-0.6） ---0.4
	//重叠力度 = 重叠面积 / 重叠横坐标跨度范围 * 2 （0.3-0.7） ---0.5
	int AllRange = 0, PercRange[TartgetSpeed_FuzzyNum - 1] = { 0 };
	float OutRange = 0.0f, OutPerc = 0.0f;
	//err_range
	AllRange = (int)(Ptr->GoErr.max - Ptr->GoErr.min);
	PercRange[0] = AllRange / (TartgetSpeed_FuzzyNum - 1) + 1;
	PercRange[1] = (int)(AllRange * 0.2f);
	PercRange[2] = AllRange - 2 * PercRange[0] - 2 * PercRange[1];
	PercRange[3] = PercRange[1];
	Ptr->Err_pos[0] = PercRange[0];
	Ptr->Err_pos[1] = Ptr->Err_pos[0] + PercRange[1];
	Ptr->Err_pos[2] = Ptr->Err_pos[1] + PercRange[2];
	Ptr->Err_pos[3] = Ptr->Err_pos[2] + PercRange[3];
	if (Ptr->Err_pos[1] == Ptr->Err_pos[2]){
		Ptr->Err_pos[1] -= 1;
		Ptr->Err_pos[2] += 1;
	}
	//derr_range
	AllRange = (int)(Ptr->GoDerr.max - Ptr->GoDerr.min);
	PercRange[0] = AllRange / (TartgetSpeed_FuzzyNum - 1) + 1;
	PercRange[1] = (int)(AllRange * 0.2f);
	PercRange[2] = AllRange - 2 * PercRange[0] - 2 * PercRange[1];
	PercRange[3] = PercRange[1];
	Ptr->Derr_pos[0] = PercRange[0];
	Ptr->Derr_pos[1] = Ptr->Derr_pos[0] + PercRange[1];
	Ptr->Derr_pos[2] = Ptr->Derr_pos[1] + PercRange[2];
	Ptr->Derr_pos[3] = Ptr->Derr_pos[2] + PercRange[3];
	if (Ptr->Derr_pos[1] == Ptr->Err_pos[2]){
		Ptr->Derr_pos[1] -= 1;
		Ptr->Derr_pos[2] += 1;
	}
	//out_range
	OutRange = Ptr->GoOut.max - Ptr->GoOut.min;
	OutPerc = OutRange / (TartgetSpeed_FuzzyNum - 1);
	for (int i = 0; i < TartgetSpeed_FuzzyNum; i++){
		Ptr->Out_RealValue[i] = Ptr->GoOut.min + i*OutPerc;
	}
}

/**
@breif 模糊隶属度函数建立
**/
void TargetSpeed_SetFuzzyFunc(int XErr_pos[TartgetSpeed_FuzzyNum - 1], \
	float XErr_Value[2], int* Pos, float XErr){
	float fuzzy_k = 0.0f;
	if ((XErr-XErr_pos[0]) <= 0){
		XErr_Value[0] = 0.0f;
		XErr_Value[1] = 1.0f;
		*Pos = SpeedNB;
	}
	else if ((XErr - XErr_pos[1]) <= 0){
		fuzzy_k = 1.0f / (float)(XErr_pos[1] - XErr_pos[0]);
		XErr_Value[0] = fuzzy_k*XErr - fuzzy_k*XErr_pos[0];
		XErr_Value[1] = -fuzzy_k*XErr + fuzzy_k*XErr_pos[1];
		*Pos = SpeedNS;
	}
	else if ((XErr - XErr_pos[2]) <= 0){
		XErr_Value[0] = 1.0f;
		XErr_Value[1] = 0.0f;
		*Pos = SpeedZO;
	}
	else if ((XErr - XErr_pos[3]) <= 0){
		fuzzy_k = 1.0f / (float)(XErr_pos[3] - XErr_pos[2]);
		XErr_Value[0] = -fuzzy_k*XErr + fuzzy_k*XErr_pos[3];
		XErr_Value[1] = fuzzy_k*XErr - fuzzy_k*XErr_pos[2];
		*Pos = SpeedPS;
	}
	else{
		XErr_Value[0] = 0.0f;
		XErr_Value[1] = 1.0f;
		*Pos = SpeedPB;
	}
}

/**
* @breif 查询规则表与解模糊
**/
float TargetSpeed_DoFuzzy(TargetSpeed_Typedef *Ptr, float Err, float Derr){
	int rules_out[4] = { 0 };
	float Nume = 0.0f, Demo = 0.0f;
	/*获取隶属度值与位置*/
	TargetSpeed_SetFuzzyFunc(Ptr->Err_pos, Ptr->Err_Value, &(Ptr->Err_CurrentPos), Err);
	TargetSpeed_SetFuzzyFunc(Ptr->Derr_pos, Ptr->Derr_Value, &(Ptr->Derr_CurrentPos), Derr);
	if (Ptr->Err_CurrentPos == TartgetSpeed_FuzzyNum) Ptr->Err_CurrentPos = TartgetSpeed_FuzzyNum - 1;
	if (Ptr->Derr_CurrentPos == TartgetSpeed_FuzzyNum) Ptr->Derr_CurrentPos = TartgetSpeed_FuzzyNum - 1;
	/*查规则表,一般都是四个规则有效*/
	rules_out[0] = *(Ptr->GoRules + Ptr->Err_CurrentPos)[Ptr->Derr_CurrentPos];
	rules_out[1] = *(Ptr->GoRules + Ptr->Err_CurrentPos)[Ptr->Derr_CurrentPos + 1];
	rules_out[2] = *(Ptr->GoRules + Ptr->Err_CurrentPos + 1)[Ptr->Derr_CurrentPos];
	rules_out[3] = *(Ptr->GoRules + Ptr->Err_CurrentPos + 1)[Ptr->Derr_CurrentPos + 1];
	/*输出的语言值，求最小*/
	if ((Ptr->Err_Value[0] - Ptr->Derr_Value[0]) <= 0)  Ptr->Out_Value[0] = Ptr->Err_Value[0];
	else Ptr->Out_Value[0] = Ptr->Derr_Value[0];
	if ((Ptr->Err_Value[0] - Ptr->Derr_Value[1]) <= 0)  Ptr->Out_Value[1] = Ptr->Err_Value[0];
	else Ptr->Out_Value[1] = Ptr->Derr_Value[1];
	if ((Ptr->Err_Value[1] - Ptr->Derr_Value[0]) <= 0)  Ptr->Out_Value[2] = Ptr->Err_Value[1];
	else Ptr->Out_Value[2] = Ptr->Derr_Value[0];
	if ((Ptr->Err_Value[1] - Ptr->Derr_Value[1]) <= 0)  Ptr->Out_Value[3] = Ptr->Err_Value[1];
	else Ptr->Out_Value[3] = Ptr->Derr_Value[1];
	/*同隶属度输出语言求最大,可有可无，一定程度上抗干扰*/	
	int Count = 3;
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < Count; j++){
			if (rules_out[i] == rules_out[i + j + 1]){
				if ((Ptr->Out_Value[i] - Ptr->Out_Value[i + j + 1]) < 0) Ptr->Out_Value[i] = 0.0f;
				else Ptr->Out_Value[i + j + 1] = 0.0f;
			}
		}
		Count--;
	}
	/*重心法反模糊*/
	Nume = Ptr->Out_Value[0] * Ptr->Out_RealValue[rules_out[0]]	\
		+ Ptr->Out_Value[1] * Ptr->Out_RealValue[rules_out[1]]	\
		+ Ptr->Out_Value[2] * Ptr->Out_RealValue[rules_out[2]]	\
		+ Ptr->Out_Value[3] * Ptr->Out_RealValue[rules_out[3]];
	Demo = Ptr->Out_Value[0] + Ptr->Out_Value[1]	\
		+ Ptr->Out_Value[2] + Ptr->Out_Value[3];
	return (Nume / Demo);
}
