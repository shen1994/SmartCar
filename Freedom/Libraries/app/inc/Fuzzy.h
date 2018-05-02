#ifndef __FUZZY__H__
#define __FUZZY__H__

#ifdef __cplusplus
extern "C" {
#endif

#define Fuzzy_PosSize 7

enum Fuzzy_PosDef
{
	NB = 0, NM, NS, ZO, PS, PM, PB
};

#ifdef _MSC_VER
#pragma pack(push,4)
#endif
typedef struct
{
	float Max;
	float Min;
}Fuzzy_AreaDef;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push,4)
#endif
typedef struct
{
	const unsigned char (*Rules)[Fuzzy_PosSize];
	Fuzzy_AreaDef Err_Area;
	float Err_K;
	float Err;
	float Err_Perc[Fuzzy_PosSize];
	Fuzzy_AreaDef DErr_Area;
	float DErr_K;
	float DErr;
	float DErr_Perc[Fuzzy_PosSize];
	Fuzzy_AreaDef Out_Area;
	float Out;
	float Out_Value[Fuzzy_PosSize];
}Fuzzy_TypeDef;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

extern const float Fuzzy_AD_EMin;
extern const float Fuzzy_AD_EMax;
extern const float Fuzzy_AD_DEMin;
extern const float Fuzzy_AD_DEMax;
extern const float Fuzzy_AD_OUTMin;
extern const float Fuzzy_AD_OUTMax;
extern Fuzzy_TypeDef Fuzzy_AD;
extern const unsigned char Fuzzy_AD_Rules[7][7];
float GetMin(float a, float b);
float GetMax(float a, float b);
float GetSql(float number);
float GetSquareRoot(float a, float b);
extern void Fuzzy_Value_init(Fuzzy_TypeDef*p);
extern void Fuzzy_init(void);
extern void Fuzzy_ClearArray(float *data, int size);
extern void Fuzzy_Fuzzify(Fuzzy_TypeDef*p);
extern void Fuzzy_DisFuzzify(Fuzzy_TypeDef* p);
extern float Fuzzy_Do(float err, float derr, Fuzzy_TypeDef*p);


#ifdef __cplusplus
}
#endif

#endif
