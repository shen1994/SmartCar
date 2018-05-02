#ifndef __TARTGETSPEED_H_
#define __TARTGETSPEED_H_

#define TartgetSpeed_FuzzyNum 5

enum TartgetSpeed_Pos{
	SpeedNB = 0,
	SpeedNS,
	SpeedZO,
	SpeedPS,
	SpeedPB
};

typedef struct{
	float min;
	float max;
}Range_Typedef;

typedef struct{
	const unsigned char(*GoRules)[TartgetSpeed_FuzzyNum];
	Range_Typedef GoErr;
	Range_Typedef GoDerr;
	Range_Typedef GoOut;
	int Err_pos[TartgetSpeed_FuzzyNum - 1];
	int Derr_pos[TartgetSpeed_FuzzyNum - 1];
	float Err_Value[2];
	float Derr_Value[2];
	float Out_Value[4];
	float Out_RealValue[TartgetSpeed_FuzzyNum];
	int Err_CurrentPos;
	int Derr_CurrentPos;
}TargetSpeed_Typedef;

extern TargetSpeed_Typedef TartgetSpeed_Reference;

void TargetSpeed_FuzzyInit(TargetSpeed_Typedef *Ptr);
void TargetSpeed_FuzzyExpand(TargetSpeed_Typedef *Ptr);
void TargetSpeed_SetFuzzyFunc(int XErr_pos[TartgetSpeed_FuzzyNum - 1], \
	float XErr_Value[2], int* Pos, float XErr);
float TargetSpeed_DoFuzzy(TargetSpeed_Typedef *Ptr, float Err, float Derr);

#endif
